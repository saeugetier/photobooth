#include "libcameracamera.h"
#include <QDebug>
#include <QThread>
#include <QTimer>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <libcamera/formats.h>
#include <libcamera/framebuffer_allocator.h>
#include <qvideoframe.h>
#include <sys/mman.h>
#include <thread>

using namespace libcamera;

LibcameraDevice::LibcameraDevice(QObject *parent)
    : QVideoFrameInput(parent), mWorkerThread(std::make_unique<QThread>(this)) {
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

QStringList LibcameraDevice::availableCameras() const {
  QStringList cameras;
  if (mWorker) {
    QMetaObject::invokeMethod(
        mWorker, [this, &cameras]() { cameras = mWorker->availableCameras(); },
        Qt::BlockingQueuedConnection);
  }
  return cameras;
}

QString LibcameraDevice::getDefaultCamera() const {
  QStringList cameras = availableCameras();
  return cameras.isEmpty() ? QString() : cameras.first();
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

QStringList LibCameraWorker::availableCameras() const {
  QStringList list;
  if (!mCameraManager)
    return list;
  for (auto const &id : mCameraManager->cameras())
    list << QString::fromStdString(id->id());
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

  qDebug() << "[INFO] Capture request queued successfully";
  // Completion will be handled by processCaptureComplete
}

void LibCameraWorker::processCaptureComplete(Request *request) {
  qDebug() << "[DEBUG] processCaptureComplete called, status:" << request->status();

  if (request->status() != Request::RequestCancelled) {
    if (!request->buffers().empty()) {
      QImage img = convertBufferToImage(request->buffers());
      if (!img.isNull()) {
        Q_EMIT imageCaptured(img);
      } else {
        Q_EMIT errorOccurred("libcamera: failed to convert capture buffer");
      }
    } else {
      Q_EMIT errorOccurred("libcamera: capture request has no buffers");
    }
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
  qDebug() << "[DEBUG] processCompletedRequest called, mCaptureInProgress:" << mCaptureInProgress;
  
  if (mCaptureInProgress) {
    qDebug() << "[DEBUG] Ignoring completed request during capture";
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
  
  // Return buffer to free list (with lock)
  {
    std::lock_guard<std::mutex> lock(mBufferMutex);
    for (auto it = buffers.begin(); it != buffers.end(); ++it) {
      mBuffersInFlight.erase(it->second);
      mFreeBuffers.push_back(it->second);
    }
  }

  if (request->status() == Request::RequestCancelled) {
    return;
  }

  QImage preview = convertBufferToImage(buffers);
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

    const FrameBuffer::Plane &plane = buffer->planes().front();
    void *memory =
        mmap(NULL, plane.length, PROT_READ, MAP_SHARED, plane.fd.get(), 0);

    if (memory == MAP_FAILED) {
      qDebug() << "[ERROR] Failed to mmap framebuffer memory:"
               << strerror(errno);
      return QImage();
    }

    const StreamConfiguration &cfg = stream->configuration();

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
      image = temp.copy();
    } else if (cfg.pixelFormat == libcamera::formats::MJPEG) {
      size_t size = buffer->metadata().planes()[0].bytesused;
      image.loadFromData(static_cast<const uchar *>(memory), static_cast<int>(size), "JPEG");
    } else if (cfg.pixelFormat == libcamera::formats::YUYV) {
      image = QImage(cfg.size.width, cfg.size.height, QImage::Format_RGB888);
      const uint8_t *src = static_cast<const uint8_t *>(memory);
      for (unsigned int y = 0; y < cfg.size.height; y++) {
        const uint8_t *row = src + y * cfg.stride;
        for (unsigned int x = 0; x < cfg.size.width; x += 2) {
          int y0 = row[x * 2 + 0];
          int u  = row[x * 2 + 1];
          int y1 = row[x * 2 + 2];
          int v  = row[x * 2 + 3];

          auto clamp = [](int val) { return std::max(0, std::min(255, val)); };

          int c0 = y0 - 16;
          int c1 = y1 - 16;
          int d = u - 128;
          int e = v - 128;

          int r0 = clamp((298 * c0 + 409 * e + 128) >> 8);
          int g0 = clamp((298 * c0 - 100 * d - 208 * e + 128) >> 8);
          int b0 = clamp((298 * c0 + 516 * d + 128) >> 8);

          int r1 = clamp((298 * c1 + 409 * e + 128) >> 8);
          int g1 = clamp((298 * c1 - 100 * d - 208 * e + 128) >> 8);
          int b1 = clamp((298 * c1 + 516 * d + 128) >> 8);

          image.setPixel(x, y, qRgb(r0, g0, b0));
          image.setPixel(x + 1, y, qRgb(r1, g1, b1));
        }
      }
    } else if (cfg.pixelFormat == libcamera::formats::YUV420) {
      // YUV420 planar format - Pi camera uses I420 (Y, U, V order)
      // But colors appear swapped, so this might be YV12 (Y, V, U order)
      unsigned int width = cfg.size.width;
      unsigned int height = cfg.size.height;
      unsigned int stride = cfg.stride;
      image = QImage(width, height, QImage::Format_RGB888);
      
      const uint8_t *src = static_cast<const uint8_t *>(memory);
      const uint8_t *yPlane = src;
      // Swap U and V planes - Pi outputs Y, V, U (YV12) not Y, U, V (I420)
      const uint8_t *uPlane = src + stride * height;
      const uint8_t *vPlane = uPlane + (stride / 2) * (height / 2);
      
      for (unsigned int y = 0; y < height; y++) {
        uint8_t *destRow = image.scanLine(y);
        for (unsigned int x = 0; x < width; x++) {
          int yVal = yPlane[y * stride + x];
          int uVal = uPlane[(y / 2) * (stride / 2) + (x / 2)];
          int vVal = vPlane[(y / 2) * (stride / 2) + (x / 2)];
          
          // YUV to RGB conversion
          int c = yVal - 16;
          int d = uVal - 128;
          int e = vVal - 128;
          
          int r = (298 * c + 409 * e + 128) >> 8;
          int g = (298 * c - 100 * d - 208 * e + 128) >> 8;
          int b = (298 * c + 516 * d + 128) >> 8;
          
          // Clamp values
          r = r < 0 ? 0 : (r > 255 ? 255 : r);
          g = g < 0 ? 0 : (g > 255 ? 255 : g);
          b = b < 0 ? 0 : (b > 255 ? 255 : b);
          
          destRow[x * 3 + 0] = static_cast<uint8_t>(r);
          destRow[x * 3 + 1] = static_cast<uint8_t>(g);
          destRow[x * 3 + 2] = static_cast<uint8_t>(b);
        }
      }
    } else {
      qDebug() << "[ERROR] Unsupported pixel format:"
               << QString::fromStdString(cfg.pixelFormat.toString());
    }

    munmap(memory, plane.length);
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
    unsigned int defaultWidth = cfg.size.width;
    unsigned int defaultHeight = cfg.size.height;
    float aspectRatio = static_cast<float>(defaultWidth) / defaultHeight;

    cfg.pixelFormat = formats::RGB888;
    cfg.size.width = 640;
    cfg.size.height = static_cast<unsigned int>(640.0f / aspectRatio);
    cfg.bufferCount = 4;
    mCurrentWidth = cfg.size.width;
    mCurrentHeight = cfg.size.height;
  } else if (role == StreamRole::StillCapture) {
    // Don't force MJPEG - use the camera's default format for still capture
    // Pi cameras typically use YUV420 which we'll convert
    // Only set buffer count, keep default format and resolution
    cfg.bufferCount = 1;
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
