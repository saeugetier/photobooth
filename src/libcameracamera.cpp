#include "libcameracamera.h"
#include <QDebug>
#include <QThread>
#include <QTimer>
#include <QVariantMap>
#include <cmath>
#include <limits>
#include <cerrno>
#include <cstring>
#include <libcamera/formats.h>
#include <libcamera/framebuffer_allocator.h>
#include <libcamera/property_ids.h>
#include <qvideoframe.h>
#include <sys/mman.h>
#include <opencv2/opencv.hpp>

using namespace libcamera;

LibcameraDevice::LibcameraDevice(QObject *parent)
    : QVideoFrameInput(parent), mWorkerThread(std::make_unique<QThread>()) {
  mWorker = new LibCameraWorker();
  mWorker->moveToThread(mWorkerThread.get());

  connect(this, &QVideoFrameInput::readyToSendVideoFrame, mWorker,
          &LibCameraWorker::queueViewfinderRequest);

  connect(mWorker, &LibCameraWorker::frameReady, this,
          &LibcameraDevice::onFrameReady);
  connect(mWorker, &LibCameraWorker::imageCaptured, this,
          &LibcameraDevice::onImageCaptured);
  connect(mWorker, &LibCameraWorker::errorOccurred, this,
          &LibcameraDevice::onErrorOccurred);

  connect(mWorkerThread.get(), &QThread::finished, mWorker,
          &QObject::deleteLater);

  mWorkerThread->start();

  qDebug() << "[INFO] LibcameraDevice initialized";
}

LibcameraDevice::~LibcameraDevice() {
  if (mWorkerThread) {
    mWorkerThread->quit();
    mWorkerThread->wait(3000);
  }
  qDebug() << "[INFO] LibcameraDevice destroyed";
}

QVariantList LibcameraDevice::availableCameras() const {
  QVariantList cameras;
  if (mWorker) {
    QMetaObject::invokeMethod(
        mWorker, [this, &cameras]() { cameras = mWorker->availableCameras(); },
        Qt::BlockingQueuedConnection);
  }
  return cameras;
}

QString LibcameraDevice::getDefaultCamera() const {
  QVariantList cameras = availableCameras();
  if (cameras.isEmpty())
    return QString();
  return cameras.first().toMap().value("value").toString();
}

void LibcameraDevice::startCamera(const QString &cameraId) {
  QMetaObject::invokeMethod(mWorker, "startCamera", Qt::QueuedConnection,
                            Q_ARG(QString, cameraId));
}

void LibcameraDevice::stopCamera() {
  QMetaObject::invokeMethod(mWorker, "stopCamera", Qt::QueuedConnection);
}

void LibcameraDevice::captureImage() {
  QMetaObject::invokeMethod(mWorker, "captureImage", Qt::QueuedConnection);
}

void LibcameraDevice::onFrameReady(const QImage &image) {
  sendVideoFrame(QVideoFrame(image));
}

void LibcameraDevice::onImageCaptured(const QImage &image) {
  emit imageCaptured(image);
}

void LibcameraDevice::onErrorOccurred(const QString &error) {
  emit errorOccurred(error);
}

LibCameraWorker::LibCameraWorker(QObject *parent) : QObject(parent) {
  // Connect captureCompleted to resumeViewfinder with QueuedConnection
  // This ensures resumeViewfinder runs on the Qt event loop, not libcamera's thread
  connect(this, &LibCameraWorker::captureCompleted, 
          this, &LibCameraWorker::resumeViewfinder, 
          Qt::QueuedConnection);
  
  initCameraManager();
}

LibCameraWorker::~LibCameraWorker() {
  stopCamera();
  if (mCameraManager)
    mCameraManager->stop();
}

void LibCameraWorker::initCameraManager() {
  qDebug() << "[INFO] LIBCAMERA_IPA_CONFIG_PATH:" << qgetenv("LIBCAMERA_IPA_CONFIG_PATH");
  qDebug() << "[INFO] LIBCAMERA_IPA_MODULE_PATH:" << qgetenv("LIBCAMERA_IPA_MODULE_PATH");

  mCameraManager = std::make_unique<CameraManager>();
  int ret = mCameraManager->start();
  if (ret) {
    emit errorOccurred(
        QString::asprintf("libcamera: CameraManager start failed: %d", ret));
    mCameraManager.reset();
    return;
  }

  qDebug() << "[INFO] libcamera: CameraManager started, cameras found:" << mCameraManager->cameras().size();
  for (const auto &cam : mCameraManager->cameras()) {
    qDebug() << "[INFO] libcamera: Camera ID:" << QString::fromStdString(cam->id());
  }
}

QVariantList LibCameraWorker::availableCameras() const {
  QVariantList list;
  if (!mCameraManager)
    return list;
  for (auto const &cam : mCameraManager->cameras()) {
    QString rawId = QString::fromStdString(cam->id());
    QString displayName = rawId;

    const auto &props = cam->properties();
    const auto model = props.get(libcamera::properties::Model);
    if (model && !model->empty()) {
      displayName = QString::fromUtf8(model->data(), model->size());
    }

    QVariantMap entry;
    entry["text"] = "Libcamera - " + displayName;
    entry["value"] = rawId;
    list.append(entry);
  }
  return list;
}

void LibCameraWorker::startCamera(const QString &cameraId) {
  if (!mCameraManager) {
    emit errorOccurred("libcamera: CameraManager not initialized");
    return;
  }

  if (mRunning)
    stopCamera();

  std::shared_ptr<Camera> cam = mCameraManager->get(cameraId.toStdString());
  if (!cam) {
    emit errorOccurred("libcamera: camera not found: " + cameraId);
    return;
  }
  mCamera = cam;

  if (mCamera->acquire() < 0) {
    emit errorOccurred("libcamera: acquire failed");
    mCamera.reset();
    return;
  }

  if (!configureCamera(StreamRole::Viewfinder)) {
    mCamera->release();
    mCamera.reset();
    return;
  }

  mCamera->requestCompleted.connect(this, &LibCameraWorker::processCompletedRequest);

  if (mCamera->start() < 0) {
    emit errorOccurred("libcamera: start failed");
    mCamera->requestCompleted.disconnect();
    mCamera->release();
    mCamera.reset();
    return;
  }

  mRunning = true;

  // Queue initial requests
  {
    std::lock_guard<std::mutex> lock(mBufferMutex);
    size_t numBuffers = mFreeBuffers.size();
    for (size_t i = 0; i < numBuffers; i++) {
      queueViewfinderRequestLocked();
    }
  }
}

void LibCameraWorker::stopCamera() {
  if (!mRunning)
    return;

  mRunning = false;

  if (mCamera) {
    mCamera->stop();
    mCamera->requestCompleted.disconnect();
    
    // Clear buffer tracking
    {
      std::lock_guard<std::mutex> lock(mBufferMutex);
      mFreeBuffers.clear();
      mBuffersInFlight.clear();
    }
    
    mAllocator.reset();
    mConfig.reset();
    mStream = nullptr;
    mCamera->release();
    mCamera.reset();
  }
}

void LibCameraWorker::captureImage() {
  qDebug() << "[INFO] LibCameraWorker::captureImage called";

  if (!mRunning || !mCamera) {
    emit errorOccurred("libcamera: camera not running");
    return;
  }

  if (mCaptureInProgress) {
    emit errorOccurred("libcamera: capture already in progress");
    return;
  }

  // Set flag FIRST to stop new requests from being queued
  mCaptureInProgress = true;
  qDebug() << "[DEBUG] Set mCaptureInProgress = true";

  // Stop the camera to cancel pending requests
  qDebug() << "[DEBUG] Calling mCamera->stop()";
  mCamera->stop();
  qDebug() << "[DEBUG] mCamera->stop() returned";
  
  mCamera->requestCompleted.disconnect();
  qDebug() << "[DEBUG] Disconnected requestCompleted";

  // Now clear the buffer tracking
  {
    std::lock_guard<std::mutex> lock(mBufferMutex);
    qDebug() << "[DEBUG] Clearing buffers. InFlight:" << mBuffersInFlight.size() << "Free:" << mFreeBuffers.size();
    mBuffersInFlight.clear();
    mFreeBuffers.clear();
  }

  // Release camera to reconfigure
  mCamera->release();
  mAllocator.reset();
  mConfig.reset();
  mStream = nullptr;
  qDebug() << "[DEBUG] Released camera, now acquiring for still capture";

  // Acquire and configure for still capture
  if (mCamera->acquire() < 0) {
    emit errorOccurred("libcamera: acquire failed for capture");
    mCaptureInProgress = false;
    return;
  }

  if (!configureCamera(StreamRole::StillCapture)) {
    mCamera->release();
    mCaptureInProgress = false;
    return;
  }

  // Connect capture completion handler
  mCamera->requestCompleted.connect(this, &LibCameraWorker::processCaptureComplete);

  if (mCamera->start() < 0) {
    emit errorOccurred("libcamera: start failed for capture");
    mCamera->requestCompleted.disconnect();
    mCamera->release();
    mCaptureInProgress = false;
    return;
  }

  // Reset retry count and queue first capture request
  mCaptureRetryCount = 0;
  queueCaptureRequest();
}

void LibCameraWorker::queueCaptureRequest() {
  // Create and queue capture request
  std::unique_ptr<Request> request = mCamera->createRequest();
  if (!request) {
    emit errorOccurred("libcamera: createRequest failed for capture");
    mCamera->stop();
    mCamera->requestCompleted.disconnect();
    mCamera->release();
    mCaptureInProgress = false;
    return;
  }

  FrameBuffer *fb = nullptr;
  {
    std::lock_guard<std::mutex> lock(mBufferMutex);
    if (mFreeBuffers.empty()) {
      emit errorOccurred("libcamera: no buffers available for capture");
      mCamera->stop();
      mCamera->requestCompleted.disconnect();
      mCamera->release();
      mCaptureInProgress = false;
      return;
    }
    fb = mFreeBuffers.front();
    mFreeBuffers.pop_front();
    mBuffersInFlight.insert(fb);
  }

  int ret = request->addBuffer(mStream, fb);
  if (ret < 0) {
    emit errorOccurred(QString("libcamera: addBuffer failed for capture: %1").arg(ret));
    std::lock_guard<std::mutex> lock(mBufferMutex);
    mBuffersInFlight.erase(fb);
    mFreeBuffers.push_back(fb);
    mCamera->stop();
    mCamera->requestCompleted.disconnect();
    mCamera->release();
    mCaptureInProgress = false;
    return;
  }

  ret = mCamera->queueRequest(request.release());
  if (ret < 0) {
    emit errorOccurred(QString("libcamera: queueRequest failed for capture: %1").arg(ret));
    std::lock_guard<std::mutex> lock(mBufferMutex);
    mBuffersInFlight.erase(fb);
    mFreeBuffers.push_back(fb);
    mCamera->stop();
    mCamera->requestCompleted.disconnect();
    mCamera->release();
    mCaptureInProgress = false;
    return;
  }

  qDebug() << "[INFO] Capture request queued successfully (retry:" << mCaptureRetryCount << ")";
  // Completion will be handled by processCaptureComplete
}

void LibCameraWorker::processCaptureComplete(Request *request) {
  qDebug() << "[DEBUG] processCaptureComplete called, request_status:" << request->status();

  // Return buffer to free list first
  const std::map<const Stream *, FrameBuffer *> &buffers = request->buffers();
  {
    std::lock_guard<std::mutex> lock(mBufferMutex);
    for (auto it = buffers.begin(); it != buffers.end(); ++it) {
      mBuffersInFlight.erase(it->second);
      mFreeBuffers.push_back(it->second);
    }
  }

  if (request->status() == Request::RequestCancelled) {
    qDebug() << "[DEBUG] Request was cancelled";
    Q_EMIT captureCompleted();
    return;
  }

  // Check frame metadata status
  if (!buffers.empty()) {
    FrameBuffer *buffer = buffers.begin()->second;
    const FrameMetadata &metadata = buffer->metadata();
    
    // If frame has error, retry up to 5 times
    if (metadata.status != 0) {
      qDebug() << "[WARNING] Frame has error status:" << metadata.status << ", retrying...";
      mCaptureRetryCount++;
      if (mCaptureRetryCount < 5) {
        // Queue another capture request
        queueCaptureRequest();
        return;
      } else {
        qDebug() << "[ERROR] Max capture retries exceeded";
        Q_EMIT errorOccurred("libcamera: capture failed after 5 retries");
        Q_EMIT captureCompleted();
        return;
      }
    }
  }

  // Reset retry count on success
  mCaptureRetryCount = 0;

  if (!buffers.empty()) {
    QImage img = convertBufferToImage(buffers);
    if (!img.isNull()) {
      Q_EMIT imageCaptured(img);
    } else {
      Q_EMIT errorOccurred("libcamera: failed to convert capture buffer");
    }
  } else {
    Q_EMIT errorOccurred("libcamera: capture request has no buffers");
  }

  // Emit signal to trigger resume on Qt thread
  Q_EMIT captureCompleted();
}

void LibCameraWorker::resumeViewfinder() {
  // Stop capture mode
  mCamera->stop();
  mCamera->requestCompleted.disconnect();
  mCamera->release();
  mAllocator.reset();
  mFreeBuffers.clear();
  mConfig.reset();
  mStream = nullptr;

  mCaptureInProgress = false;

  // Reconfigure for viewfinder
  if (mCamera->acquire() < 0) {
    emit errorOccurred("libcamera: acquire failed for viewfinder");
    return;
  }

  if (!configureCamera(StreamRole::Viewfinder)) {
    return;
  }

  mCamera->requestCompleted.connect(this, &LibCameraWorker::processCompletedRequest);

  if (mCamera->start() < 0) {
    emit errorOccurred("libcamera: start failed for viewfinder");
    return;
  }

  mRunning = true;

  // Queue requests for all buffers
  size_t numBuffers = mFreeBuffers.size();
  for (size_t i = 0; i < numBuffers; i++) {
    queueViewfinderRequest();
  }
}

void LibCameraWorker::queueViewfinderRequest() {
  std::lock_guard<std::mutex> lock(mBufferMutex);
  queueViewfinderRequestLocked();
}

void LibCameraWorker::queueViewfinderRequestLocked() {
  // Must be called with mBufferMutex held!
  if (!mCamera || !mStream || !mRunning || mCaptureInProgress)
    return;

  if (mFreeBuffers.empty()) {
    return;
  }

  std::unique_ptr<Request> request = mCamera->createRequest();
  if (!request) {
    qDebug() << "[ERROR] createRequest failed";
    return;
  }

  FrameBuffer *fb = mFreeBuffers.front();
  mFreeBuffers.pop_front();

  int ret = request->addBuffer(mStream, fb);
  if (ret < 0) {
    qDebug() << "[ERROR] addBuffer failed:" << ret;
    mFreeBuffers.push_back(fb);
    return;
  }

  mBuffersInFlight.insert(fb);

  ret = mCamera->queueRequest(request.release());
  if (ret < 0) {
    qDebug() << "[ERROR] queueRequest failed:" << ret;
    mBuffersInFlight.erase(fb);
    mFreeBuffers.push_back(fb);
    return;
  }
}

void LibCameraWorker::processCompletedRequest(Request *request) {
  if (mCaptureInProgress) {
    // Still need to return buffer to free list!
    const std::map<const Stream *, FrameBuffer *> &buffers = request->buffers();
    {
      std::lock_guard<std::mutex> lock(mBufferMutex);
      for (auto it = buffers.begin(); it != buffers.end(); ++it) {
        mBuffersInFlight.erase(it->second);
        mFreeBuffers.push_back(it->second);
      }
    }
    return;
  }

  if (!request || request->buffers().empty())
    return;

  const std::map<const Stream *, FrameBuffer *> &buffers = request->buffers();
  
  if (request->status() == Request::RequestCancelled) {
    // Return buffer to free list (with lock)
    std::lock_guard<std::mutex> lock(mBufferMutex);
    for (auto it = buffers.begin(); it != buffers.end(); ++it) {
      mBuffersInFlight.erase(it->second);
      mFreeBuffers.push_back(it->second);
    }
    return;
  }

  // IMPORTANT: Convert image BEFORE returning buffer to free list!
  // Otherwise the buffer may be reused before we read it
  QImage preview = convertBufferToImage(buffers);
  
  // Now return buffer to free list (with lock)
  {
    std::lock_guard<std::mutex> lock(mBufferMutex);
    for (auto it = buffers.begin(); it != buffers.end(); ++it) {
      mBuffersInFlight.erase(it->second);
      mFreeBuffers.push_back(it->second);
    }
  }
  
  if (!preview.isNull()) {
    Q_EMIT frameReady(preview);
  }

  // Queue another request (with lock)
  {
    std::lock_guard<std::mutex> lock(mBufferMutex);
    queueViewfinderRequestLocked();
  }
}

QImage LibCameraWorker::convertBufferToImage(
    const std::map<const Stream *, FrameBuffer *> &buffers) {
  QImage image;

  for (auto it = buffers.begin(); it != buffers.end(); ++it) {
    const Stream *stream = it->first;
    FrameBuffer *buffer = it->second;

    const StreamConfiguration &cfg = stream->configuration();
    Span<const FrameBuffer::Plane> planes = buffer->planes();
    
    const FrameMetadata &metadata = buffer->metadata();
    
    // Check frame status - 0=Success, 1=Error, 2=Cancelled
    if (metadata.status != 0) {
      qDebug() << "[WARNING] Frame has error status:" << metadata.status 
               << "(0=Success, 1=Error, 2=Cancelled)";
      // Continue anyway to see what we get, but the data may be corrupted
    }
    
    // Check if buffer has data
    if (metadata.planes().empty() || metadata.planes()[0].bytesused == 0) {
      qDebug() << "[ERROR] Buffer is empty - bytesused=0";
      return QImage();
    }

    // For single-plane formats (RGB, BGR, MJPEG, YUYV)
    if (cfg.pixelFormat == libcamera::formats::RGB888 ||
        cfg.pixelFormat == libcamera::formats::BGR888 ||
        cfg.pixelFormat == libcamera::formats::MJPEG ||
        cfg.pixelFormat == libcamera::formats::YUYV) {
      
      const FrameBuffer::Plane &plane = planes.front();
      void *memory = mmap(NULL, plane.length, PROT_READ, MAP_SHARED, 
                          plane.fd.get(), plane.offset);
      
      if (memory == MAP_FAILED) {
        qDebug() << "[ERROR] Failed to mmap framebuffer memory:" << strerror(errno);
        return QImage();
      }
      
      if (cfg.pixelFormat == libcamera::formats::RGB888) {
        QImage temp(static_cast<const uchar *>(memory),
                    cfg.size.width,
                    cfg.size.height,
                    cfg.stride,
                    QImage::Format_RGB888);
        image = temp.copy();
      } else if (cfg.pixelFormat == libcamera::formats::BGR888) {
        QImage temp(static_cast<const uchar *>(memory),
                    cfg.size.width,
                    cfg.size.height,
                    cfg.stride,
                    QImage::Format_BGR888);
        image = temp.rgbSwapped();
      } else if (cfg.pixelFormat == libcamera::formats::MJPEG) {
        size_t size = metadata.planes()[0].bytesused;
        image.loadFromData(static_cast<const uchar *>(memory), static_cast<int>(size), "JPEG");
      } else if (cfg.pixelFormat == libcamera::formats::YUYV) {
        cv::Mat yuyv(cfg.size.height, cfg.size.width, CV_8UC2, 
                     const_cast<void*>(memory), cfg.stride);
        cv::Mat rgb;
        cv::cvtColor(yuyv, rgb, cv::COLOR_YUV2RGB_YUYV);
        
        image = QImage(rgb.data, rgb.cols, rgb.rows, rgb.step, 
                       QImage::Format_RGB888).copy();
      }
      
      munmap(memory, plane.length);
      
    } else if (cfg.pixelFormat == libcamera::formats::YUV420) {
      // Use OpenCV for YUV420 (I420) to RGB conversion
      unsigned int width = cfg.size.width;
      unsigned int height = cfg.size.height;
      unsigned int stride = cfg.stride;
      
      // Check if all planes share the same fd (contiguous buffer with offsets)
      bool samefd = true;
      if (planes.size() >= 3) {
        int fd0 = planes[0].fd.get();
        for (size_t i = 1; i < planes.size(); i++) {
          if (planes[i].fd.get() != fd0) {
            samefd = false;
            break;
          }
        }
      }
      
      if (planes.size() >= 3 && samefd) {
        // All planes share same fd - map once with the total size
        // Calculate total size needed
        size_t totalSize = 0;
        for (size_t i = 0; i < planes.size(); i++) {
          size_t endOffset = planes[i].offset + planes[i].length;
          if (endOffset > totalSize) totalSize = endOffset;
        }
        
        void *memory = mmap(NULL, totalSize, PROT_READ, MAP_SHARED, 
                            planes[0].fd.get(), 0);
        
        if (memory == MAP_FAILED) {
          qDebug() << "[ERROR] Failed to mmap YUV420 buffer:" << strerror(errno);
          return QImage();
        }
        
        // Get pointers to each plane using their offsets
        const uint8_t *yData = static_cast<const uint8_t*>(memory) + planes[0].offset;
        const uint8_t *uData = static_cast<const uint8_t*>(memory) + planes[1].offset;
        const uint8_t *vData = static_cast<const uint8_t*>(memory) + planes[2].offset;
        
        // Create contiguous I420 buffer for OpenCV
        size_t ySize = stride * height;
        size_t uvStride = stride / 2;
        size_t uvSize = uvStride * (height / 2);
        std::vector<uint8_t> i420Buffer(ySize + 2 * uvSize);
        
        memcpy(i420Buffer.data(), yData, ySize);
        memcpy(i420Buffer.data() + ySize, uData, uvSize);
        memcpy(i420Buffer.data() + ySize + uvSize, vData, uvSize);
        
        cv::Mat yuv(height * 3 / 2, stride, CV_8UC1, i420Buffer.data());
        cv::Mat rgb;
        cv::cvtColor(yuv, rgb, cv::COLOR_YUV2RGB_I420);
        
        if (stride != width) {
          rgb = rgb(cv::Rect(0, 0, width, height)).clone();
        }
        
        image = QImage(rgb.data, rgb.cols, rgb.rows, rgb.step, 
                       QImage::Format_RGB888).copy();
        
        munmap(memory, totalSize);
        
      } else if (planes.size() >= 3) {
        // Different fds for each plane - map each separately
        void *yMem = mmap(NULL, planes[0].length, PROT_READ, MAP_SHARED, 
                          planes[0].fd.get(), planes[0].offset);
        void *uMem = mmap(NULL, planes[1].length, PROT_READ, MAP_SHARED, 
                          planes[1].fd.get(), planes[1].offset);
        void *vMem = mmap(NULL, planes[2].length, PROT_READ, MAP_SHARED, 
                          planes[2].fd.get(), planes[2].offset);
        
        if (yMem == MAP_FAILED || uMem == MAP_FAILED || vMem == MAP_FAILED) {
          qDebug() << "[ERROR] Failed to mmap YUV420 planes:" << strerror(errno);
          if (yMem != MAP_FAILED) munmap(yMem, planes[0].length);
          if (uMem != MAP_FAILED) munmap(uMem, planes[1].length);
          if (vMem != MAP_FAILED) munmap(vMem, planes[2].length);
          return QImage();
        }
        
        size_t ySize = stride * height;
        size_t uvStride = stride / 2;
        size_t uvSize = uvStride * (height / 2);
        std::vector<uint8_t> i420Buffer(ySize + 2 * uvSize);
        
        memcpy(i420Buffer.data(), yMem, ySize);
        memcpy(i420Buffer.data() + ySize, uMem, uvSize);
        memcpy(i420Buffer.data() + ySize + uvSize, vMem, uvSize);
        
        cv::Mat yuv(height * 3 / 2, stride, CV_8UC1, i420Buffer.data());
        cv::Mat rgb;
        cv::cvtColor(yuv, rgb, cv::COLOR_YUV2RGB_I420);
        
        if (stride != width) {
          rgb = rgb(cv::Rect(0, 0, width, height)).clone();
        }
        
        image = QImage(rgb.data, rgb.cols, rgb.rows, rgb.step, 
                       QImage::Format_RGB888).copy();
        
        munmap(yMem, planes[0].length);
        munmap(uMem, planes[1].length);
        munmap(vMem, planes[2].length);
        
      } else {
        // Single plane - all data contiguous
        const FrameBuffer::Plane &plane = planes.front();
        void *memory = mmap(NULL, plane.length, PROT_READ, MAP_SHARED, 
                            plane.fd.get(), plane.offset);
        
        if (memory == MAP_FAILED) {
          qDebug() << "[ERROR] Failed to mmap YUV420 buffer:" << strerror(errno);
          return QImage();
        }
        
        cv::Mat yuv(height * 3 / 2, stride, CV_8UC1, const_cast<void*>(memory));
        cv::Mat rgb;
        cv::cvtColor(yuv, rgb, cv::COLOR_YUV2RGB_I420);
        
        if (stride != width) {
          rgb = rgb(cv::Rect(0, 0, width, height)).clone();
        }
        
        image = QImage(rgb.data, rgb.cols, rgb.rows, rgb.step, 
                       QImage::Format_RGB888).copy();
        
        munmap(memory, plane.length);
      }
    } else {
      qDebug() << "[ERROR] Unsupported pixel format:"
               << QString::fromStdString(cfg.pixelFormat.toString());
    }

    break;
  }

  return image;
}

bool LibCameraWorker::configureCamera(libcamera::StreamRole role) {
  mConfig = mCamera->generateConfiguration({role});
  if (!mConfig) {
    Q_EMIT errorOccurred("libcamera: failed to generate configuration");
    return false;
  }

  StreamConfiguration &cfg = mConfig->at(0);

  if (role == StreamRole::Viewfinder) {
    float targetAspectRatio = 0.0f;
    std::unique_ptr<CameraConfiguration> stillConfig =
        mCamera->generateConfiguration({StreamRole::StillCapture});
    if (stillConfig && !stillConfig->empty()) {
      const StreamConfiguration &stillCfg = stillConfig->at(0);
      if (stillCfg.size.height > 0) {
        targetAspectRatio = static_cast<float>(stillCfg.size.width) /
                            static_cast<float>(stillCfg.size.height);
      }
    }

    if (targetAspectRatio <= 0.0f && cfg.size.height > 0) {
      targetAspectRatio = static_cast<float>(cfg.size.width) /
                          static_cast<float>(cfg.size.height);
    }

    std::vector<PixelFormat> pixelFormats = cfg.formats().pixelformats();
    // Search for the best available pixel format in order of preference
    PixelFormat selectedFormat = cfg.pixelFormat;
    bool formatFound = false;

    // Prefer YUV formats first; they are typically the most reliable on Pi ISP paths.
    std::vector<PixelFormat> preferredFormats = {
      formats::YUV420,
      formats::MJPEG,
      formats::YUYV,
      formats::BGR888,
      formats::RGB888
    };

    for (const auto &preferred : preferredFormats) {
      for (const auto &available : pixelFormats) {
        if (available == preferred) {
          selectedFormat = preferred;
          formatFound = true;
          qDebug() << "[INFO] Viewfinder - Selected pixel format:" << QString::fromStdString(selectedFormat.toString());
          break;
        }
      }
      if (formatFound) break;
    }

    if (!formatFound) {
      qDebug() << "[WARNING] None of the preferred viewfinder formats available, using default:" 
               << QString::fromStdString(cfg.pixelFormat.toString());
    }

    cfg.pixelFormat = selectedFormat;

    // Pick a supported viewfinder size whose aspect ratio matches still capture.
    const std::vector<Size> supportedSizes = cfg.formats().sizes(selectedFormat);
    if (!supportedSizes.empty() && targetAspectRatio > 0.0f) {
      const Size *bestSize = nullptr;
      float bestAspectDiff = std::numeric_limits<float>::max();
      unsigned int bestWidthDelta = std::numeric_limits<unsigned int>::max();

      for (const Size &size : supportedSizes) {
        if (size.height == 0) {
          continue;
        }

        float aspect = static_cast<float>(size.width) / static_cast<float>(size.height);
        float aspectDiff = std::fabs(aspect - targetAspectRatio);
        unsigned int widthDelta = (size.width > 1280) ?
                                      (size.width - 1280) :
                                      (1280 - size.width);

        if (aspectDiff < bestAspectDiff ||
            (std::fabs(aspectDiff - bestAspectDiff) < 1e-4f && widthDelta < bestWidthDelta)) {
          bestSize = &size;
          bestAspectDiff = aspectDiff;
          bestWidthDelta = widthDelta;
        }
      }

      if (bestSize) {
        cfg.size = *bestSize;
        qDebug() << "[INFO] Viewfinder - Selected size:" << cfg.size.width << "x" << cfg.size.height
                 << "for target aspect" << targetAspectRatio;
      }
    }

    // Avoid hard-forcing an arbitrary viewfinder size (can trigger crop modes on some Pi pipelines).
    // We only override the size when selecting a supported resolution that matches the still-capture aspect ratio.
    cfg.bufferCount = 4;
    mCurrentWidth = cfg.size.width;
    mCurrentHeight = cfg.size.height;
  } else if (role == StreamRole::StillCapture) {
    std::vector<PixelFormat> pixelFormats = cfg.formats().pixelformats();
    // Search for the best available pixel format in order of preference
    PixelFormat selectedFormat = formats::RGB888; // fallback
    bool formatFound = false;

    // Priority order: BGR888, RGB888, YUYV, MJPEG, YUV420
    std::vector<PixelFormat> preferredFormats = {
      formats::YUV420,
      formats::RGB888,
      formats::BGR888,
      formats::YUYV,
      formats::MJPEG
    };

    for (const auto &preferred : preferredFormats) {
      for (const auto &available : pixelFormats) {
        if (available == preferred) {
          selectedFormat = preferred;
          formatFound = true;
          qDebug() << "[INFO] StillCapture - Selected pixel format:" << QString::fromStdString(selectedFormat.toString());
          break;
        }
      }
      if (formatFound) break;
    }

    if (!formatFound) {
      qDebug() << "[WARNING] None of the preferred formats available, using default";
    }

    cfg.pixelFormat = selectedFormat;
    cfg.bufferCount = 4;
    mCurrentWidth = cfg.size.width;
    mCurrentHeight = cfg.size.height;
    qDebug() << "[INFO] Still capture default format:" << QString::fromStdString(cfg.pixelFormat.toString());
    qDebug() << "[INFO] Still capture default size:" << cfg.size.width << "x" << cfg.size.height;
  }

  CameraConfiguration::Status status = mConfig->validate();
  if (status == CameraConfiguration::Invalid) {
    Q_EMIT errorOccurred("libcamera: configuration invalid");
    return false;
  }

  if (status == CameraConfiguration::Adjusted) {
    qDebug() << "[INFO] Configuration adjusted by libcamera";
    qDebug() << "[INFO] Final format:" << QString::fromStdString(cfg.pixelFormat.toString());
    qDebug() << "[INFO] Final size:" << cfg.size.width << "x" << cfg.size.height;
  }

  if (mCamera->configure(mConfig.get()) < 0) {
    Q_EMIT errorOccurred("libcamera: camera configure failed");
    return false;
  }

  mStream = cfg.stream();
  if (!mStream) {
    Q_EMIT errorOccurred("libcamera: no stream in configuration");
    return false;
  }

  qDebug() << "[INFO] Stream configured:" << mStream;

  // Allocate buffers
  mAllocator = std::make_unique<FrameBufferAllocator>(mCamera);
  
  {
    std::lock_guard<std::mutex> lock(mBufferMutex);
    mFreeBuffers.clear();
    mBuffersInFlight.clear();
  }

  int ret = mAllocator->allocate(mStream);
  if (ret < 0) {
    Q_EMIT errorOccurred("libcamera: buffer allocation failed");
    return false;
  }

  const std::vector<std::unique_ptr<FrameBuffer>> &bufs = mAllocator->buffers(mStream);
  
  {
    std::lock_guard<std::mutex> lock(mBufferMutex);
    for (const auto &b : bufs) {
      mFreeBuffers.push_back(b.get());
    }
    qDebug() << "[INFO] Configured camera with" << mFreeBuffers.size() << "buffers for stream" << mStream;
  }

  return true;
}
