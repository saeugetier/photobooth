#include "libcameracamera.h"
#include <QDebug>
#include <QThread>
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
  // Create worker
  mWorker = new LibCameraWorker();
  mWorker->moveToThread(mWorkerThread.get());

  connect(this, &QVideoFrameInput::readyToSendVideoFrame, mWorker,
          &LibCameraWorker::queueViewfinderRequest);

  // Connect worker signals to device signals (relay/forward)
  connect(mWorker, &LibCameraWorker::frameReady, this,
          &LibcameraDevice::onFrameReady);
  connect(mWorker, &LibCameraWorker::imageCaptured, this,
          &LibcameraDevice::onImageCaptured);
  connect(mWorker, &LibCameraWorker::errorOccurred, this,
          &LibcameraDevice::onErrorOccurred);

  // Clean up when thread finishes
  connect(mWorkerThread.get(), &QThread::finished, mWorker,
          &QObject::deleteLater);

  // Start thread
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
    // Block until worker returns result
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
  initCameraManager();
}

LibCameraWorker::~LibCameraWorker() {
  stopCamera();
  if (mCameraManager)
    mCameraManager->stop();
}

void LibCameraWorker::initCameraManager() {
    // Check environment variables for debugging
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

    // Debug: list detected cameras
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

  // find camera by id
  std::shared_ptr<Camera> cam = mCameraManager->get(cameraId.toStdString());
  if (!cam) {
    emit errorOccurred("libcamera: camera not found: " + cameraId);
    return;
  }
  mCamera = cam;

  mCamera->acquire();

  configureCamera(StreamRole::Viewfinder); // Configure for viewfinder

  // start camera
  if (mCamera->start() < 0) {
    emit errorOccurred("libcamera: start failed");
    mCamera.reset();
    return;
  }

  mCamera->requestCompleted.connect(this,
                                    &LibCameraWorker::processCompletedRequest);

  mRunning = true;

  std::this_thread::sleep_for(std::chrono::milliseconds(5));

  // Queue initial requests for all buffers
  for (size_t i = 0; i < mBuffers.size(); i++) {
    queueViewfinderRequest();
  }
}

void LibCameraWorker::stopCamera() {
  if (!mRunning)
    return;

  if (mCamera) {
    mCamera->requestCompleted.disconnect();
    mCamera->stop();
    mPendingRequests.clear();
    // Clear raw pointers (allocator will delete the unique_ptrs)
    mBuffers.clear();
    mAllocator.reset();
    mCamera->release();
    mCamera.reset();
  }
  mRunning = false;
  mBufferIndex = 0;
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

  // Wait for pending requests to complete with timeout
  auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(1);
  while (!mPendingRequests.empty() &&
         std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  if (!mPendingRequests.empty()) {
    emit errorOccurred(
        "libcamera: timeout waiting for pending preview image requests");
    return;
  }

  mCaptureInProgress = true;

  // Stop camera and release to return to Available state
  mCamera->stop();
  mCamera->release(); // Release to Available state
  mAllocator.reset();
  mBuffers.clear();

  // Acquire again before reconfiguration
  if (mCamera->acquire() < 0) {
    emit errorOccurred("libcamera: acquire failed after release");
    mCaptureInProgress = false;
    return;
  }

  // Reconfigure for still capture
  configureCamera(StreamRole::StillCapture);

  // Start camera for capture
  if (mCamera->start() < 0) {
    emit errorOccurred("libcamera: start failed");
    mCaptureInProgress = false;
    return;
  }

  // Create request and capture
  std::unique_ptr<Request> request = mCamera->createRequest();
  if (!request) {
    emit errorOccurred("libcamera: createRequest failed");
    mCaptureInProgress = false;
    return;
  }

  Stream *stream = *mCamera->streams().begin();
  if (mBuffers.empty()) {
    emit errorOccurred("libcamera: no buffers available");
    mCaptureInProgress = false;
    return;
  }
  FrameBuffer *fb = mBuffers.front();
  if (request->addBuffer(stream, fb) < 0) {
    emit errorOccurred("libcamera: addBuffer failed");
    mCaptureInProgress = false;
    return;
  }

  if (mCamera->queueRequest(request.get()) < 0) {
    emit errorOccurred("libcamera: queueRequest failed");
    mCaptureInProgress = false;
    return;
  }

  // Wait for completion
  deadline = std::chrono::steady_clock::now() + std::chrono::seconds(1);
  while (std::chrono::steady_clock::now() < deadline &&
         request->status() == Request::RequestPending) {
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  if (request->status() != Request::RequestComplete) {
    emit errorOccurred("libcamera: capture failed or timed out");
    mCaptureInProgress = false;
    return;
  }

  // Convert full-resolution buffer to QImage
  if (!request->buffers().empty()) {
    auto bufferIt = request->buffers().begin();
    FrameBuffer *completedFb = bufferIt->second;
    std::map<const Stream *, FrameBuffer *> buffers = {{stream, completedFb}};
    QImage img = convertBufferToImage(buffers);
    emit imageCaptured(img);
  } else {
    emit errorOccurred("libcamera: capture request failed");
  }

  mCaptureInProgress = false;

  // Reconfigure back to viewfinder
  mCamera->stop();
  mCamera->release(); // Release again
  mAllocator.reset();
  mBuffers.clear();

  if (mCamera->acquire() < 0) {
    emit errorOccurred("libcamera: acquire failed for viewfinder");
    return;
  }

  configureCamera(StreamRole::Viewfinder);

  if (mCamera->start() < 0) {
    emit errorOccurred("libcamera: start failed for viewfinder");
    return;
  }

  // Resume preview - queue all buffers
  mBufferIndex = 0;
  for (size_t i = 0; i < mBuffers.size(); i++) {
    queueViewfinderRequest();
  }
}

void LibCameraWorker::queueViewfinderRequest() {
  if (!mCamera || mBuffers.empty() || !mRunning || mCaptureInProgress)
    return;

  // Check if we have a free buffer
  if (mPendingRequests.size() >= mBuffers.size()) {
    // All buffers are in use, wait for one to complete
    return;
  }

  // Create a request
  std::unique_ptr<Request> request = mCamera->createRequest();
  if (!request) {
    Q_EMIT errorOccurred("libcamera: createRequest failed for preview");
    return;
  }

  // Get next available buffer (find one not currently in pending requests)
  FrameBuffer *fb = nullptr;
  for (FrameBuffer *buf : mBuffers) {
    bool inUse = false;
    for (const auto &pendingReq : mPendingRequests) {
      for (const auto &[stream, pendingBuf] : pendingReq->buffers()) {
        if (pendingBuf == buf) {
          inUse = true;
          break;
        }
      }
      if (inUse) break;
    }
    if (!inUse) {
      fb = buf;
      break;
    }
  }

  if (!fb) {
    // No free buffer available
    return;
  }

  // Attach buffer to stream
  Stream *stream = *mCamera->streams().begin();
  if (request->addBuffer(stream, fb) < 0) {
    Q_EMIT errorOccurred("libcamera: addBuffer failed for preview");
    return;
  }

  // Store shared_ptr to keep request alive
  std::shared_ptr<Request> sharedRequest(request.release());
  mPendingRequests.push_back(sharedRequest);

  // Queue the request (async completion via callback)
  if (mCamera->queueRequest(sharedRequest.get()) < 0) {
    Q_EMIT errorOccurred("libcamera: queueRequest failed for preview");
    mPendingRequests.pop_back();
    return;
  }
}

void LibCameraWorker::processCompletedRequest(Request *request) {
  if (mCaptureInProgress) {
    // During capture, just remove from pending list
    auto it = std::find_if(mPendingRequests.begin(), mPendingRequests.end(),
                           [request](const std::shared_ptr<Request> &ptr) {
                             return ptr.get() == request;
                           });
    if (it != mPendingRequests.end()) {
      mPendingRequests.erase(it);
    }
    return;
  }

  if (!request || request->buffers().empty())
    return;

  if (request->status() == Request::RequestCancelled) {
    // Remove cancelled request from pending
    auto it = std::find_if(mPendingRequests.begin(), mPendingRequests.end(),
                           [request](const std::shared_ptr<Request> &ptr) {
                             return ptr.get() == request;
                           });
    if (it != mPendingRequests.end()) {
      mPendingRequests.erase(it);
    }
    return;
  }

  const std::map<const Stream *, FrameBuffer *> &buffers = request->buffers();

  // Convert buffer to QImage and emit preview
  QImage preview = convertBufferToImage(buffers);

  if (!preview.isNull()) {
    Q_EMIT frameReady(preview);
  }

  // Remove completed request from pending list
  auto it = std::find_if(mPendingRequests.begin(), mPendingRequests.end(),
                         [request](const std::shared_ptr<Request> &ptr) {
                           return ptr.get() == request;
                         });
  if (it != mPendingRequests.end()) {
    mPendingRequests.erase(it);
  }

  // Queue another request to keep the pipeline running
  queueViewfinderRequest();
}

QImage LibCameraWorker::convertBufferToImage(
    const std::map<const Stream *, FrameBuffer *> &buffers) {
  QImage image;

  for (const auto &bufferPair : buffers) {
    const Stream *stream = bufferPair.first;
    FrameBuffer *buffer = bufferPair.second;

    const FrameBuffer::Plane &plane = buffer->planes().front();
    void *memory =
        mmap(NULL, plane.length, PROT_READ, MAP_SHARED, plane.fd.get(), 0);

    if (memory == MAP_FAILED) {
      qDebug() << "[ERROR] Failed to mmap framebuffer memory:"
               << strerror(errno);
      return QImage();
    }

    // Get the stream configuration to know the format
    const StreamConfiguration &cfg = stream->configuration();
    
    if (cfg.pixelFormat == libcamera::formats::RGB888) {
      // Raw RGB888 data - create QImage directly from raw pixels
      QImage temp(static_cast<const uchar *>(memory),
                  cfg.size.width,
                  cfg.size.height,
                  cfg.stride,
                  QImage::Format_RGB888);
      // Make a deep copy since we'll unmap the memory
      image = temp.copy();
    } else if (cfg.pixelFormat == libcamera::formats::BGR888) {
      // BGR888 format
      QImage temp(static_cast<const uchar *>(memory),
                  cfg.size.width,
                  cfg.size.height,
                  cfg.stride,
                  QImage::Format_BGR888);
      image = temp.copy();
    } else if (cfg.pixelFormat == libcamera::formats::MJPEG) {
      // MJPEG - use loadFromData for encoded formats
      size_t size = buffer->metadata().planes()[0].bytesused;
      image.loadFromData(static_cast<const uchar *>(memory), static_cast<int>(size), "JPEG");
    } else {
      qDebug() << "[ERROR] Unsupported pixel format:" 
               << QString::fromStdString(cfg.pixelFormat.toString());
    }

    munmap(memory, plane.length);
    
    // Only process first buffer
    break;
  }

  return image;
}

void LibCameraWorker::configureCamera(libcamera::StreamRole role) {
  std::unique_ptr<CameraConfiguration> config =
      mCamera->generateConfiguration({role});
  if (!config) {
    emit errorOccurred("libcamera: failed to generate configuration");
    return;
  }

  if (role == StreamRole::Viewfinder) {
    unsigned int defaultWidth = config->at(0).size.width;
    unsigned int defaultHeight = config->at(0).size.height;
    float aspectRatio = static_cast<float>(defaultWidth) / defaultHeight;

    config->at(0).pixelFormat = formats::RGB888;
    config->at(0).size.width = 640;
    config->at(0).size.height = static_cast<unsigned int>(640.0f / aspectRatio);
    config->at(0).bufferCount = 4; // Multiple buffers for preview
    mCurrentWidth = config->at(0).size.width;
    mCurrentHeight = config->at(0).size.height;
  } else if (role == StreamRole::StillCapture) {
    config->at(0).pixelFormat = formats::MJPEG; // Use MJPEG for still capture
    config->at(0).bufferCount = 1;              // Single buffer for still
    mCurrentWidth = config->at(0).size.width;
    mCurrentHeight = config->at(0).size.height;
  }

  if (config->validate() == CameraConfiguration::Invalid) {
    emit errorOccurred("libcamera: configuration invalid");
    return;
  }

  if (mCamera->configure(config.get()) < 0) {
    emit errorOccurred("libcamera: camera configure failed");
    return;
  }

  // Allocate buffers
  mAllocator = std::make_unique<FrameBufferAllocator>(mCamera);
  mBuffers.clear();
  
  for (StreamConfiguration &cfg : *config) {
    Stream *stream = cfg.stream();
    if (mAllocator->allocate(stream) < 0) {
      emit errorOccurred("libcamera: buffer allocation failed");
      return;
    }
    const std::vector<std::unique_ptr<FrameBuffer>> &bufs =
        mAllocator->buffers(stream);
    for (const auto &b : bufs) {
      mBuffers.push_back(b.get());
    }
  }

  qDebug() << "[INFO] Configured camera with" << mBuffers.size() << "buffers";
}
