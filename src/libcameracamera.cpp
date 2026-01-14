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

  queueViewfinderRequest();
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

  // Resume preview
  queueViewfinderRequest();
}

void LibCameraWorker::queueViewfinderRequest() {
  if (!mCamera || mBuffers.empty() || !mRunning || mCaptureInProgress)
    return;

  // Create a request
  std::shared_ptr<Request> request = mCamera->createRequest();
  if (!request) {
    Q_EMIT errorOccurred("libcamera: createRequest failed for preview");
    return;
  }

  // Get next buffer (rotate through available buffers)
  FrameBuffer *fb =
      mBuffers[mBufferIndex % mBuffers.size()]; // Use raw pointer directly
  mBufferIndex++;

  // Attach buffer to stream
  Stream *stream = *mCamera->streams().begin();
  if (request->addBuffer(stream, fb) < 0) {
    Q_EMIT errorOccurred("libcamera: addBuffer failed for preview");
    return;
  }

  // Store request to keep it alive
  mPendingRequests.push_back(request);

  // Queue the request (async completion via callback)
  if (mCamera->queueRequest(request.get()) < 0) {
    Q_EMIT errorOccurred("libcamera: queueRequest failed for preview");
    mPendingRequests.pop_back();
    return;
  }
}

void LibCameraWorker::processCompletedRequest(Request *request) {
  if (mCaptureInProgress) {
    mCaptureInProgress = false;
    return; // Ignore preview processing during capture
    // Capture processing handled separately by blocking captureImage threaded
    // function.
  }

  if (!request || request->buffers().empty())
    return;

  const std::map<const Stream *, FrameBuffer *> &buffers = request->buffers();

  // Convert buffer to QImage and emit preview
  QImage preview = convertBufferToImage(buffers);

  if (!preview.isNull()) {
    Q_EMIT frameReady(preview);
  }

  auto it = std::find_if(mPendingRequests.begin(), mPendingRequests.end(),
                         [request](const std::shared_ptr<Request> &ptr) {
                           return ptr.get() == request;
                         });
  if (it != mPendingRequests.end()) {
    mPendingRequests.erase(it);
  }
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
      // Note: RGB888 in libcamera is actually BGR in memory order for QImage
      QImage temp(static_cast<const uchar *>(memory),
                  cfg.size.width,
                  cfg.size.height,
                  cfg.stride,
                  QImage::Format_RGB888);
      // Make a deep copy since we'll unmap the memory
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

  mAllocator = std::make_unique<FrameBufferAllocator>(mCamera);
  for (StreamConfiguration &cfg : *config) {
    Stream *stream = cfg.stream();
    if (mAllocator->allocate(stream) < 0) {
      emit errorOccurred("libcamera: buffer allocation failed");
      return;
    }
    const std::vector<std::unique_ptr<FrameBuffer>> &bufs =
        mAllocator->buffers(stream);
    for (auto &b : bufs) {
      mBuffers.push_back(b.get());
    }
  }

  // After allocating buffers, populate the free buffer queue
  Stream *stream = config_->at(0).stream();
  const std::vector<std::unique_ptr<FrameBuffer>> &buffers = allocator_->buffers(stream);
  
  {
    std::lock_guard<std::mutex> lock(bufferMutex_);
    for (const auto &buffer : buffers) {
      freeBuffers_.push(buffer.get());
    }
  }

  // Start the camera
  int ret = camera_->start();
  if (ret) {
    qDebug() << "[ERROR] Failed to start camera:" << ret;
    return false;
  }

  // Queue initial requests
  for (size_t i = 0; i < buffers.size(); i++) {
    if (!queueRequest(CaptureMode::Preview)) {
      qDebug() << "[ERROR] Failed to queue initial request";
    }
  }

  return true;
}

bool LibCameraWorker::queueRequest(CaptureMode mode) {
  FrameBuffer *buffer = nullptr;
  
  {
    std::lock_guard<std::mutex> lock(bufferMutex_);
    if (freeBuffers_.empty()) {
      qDebug() << "[WARNING] No free buffers available";
      return false;
    }
    buffer = freeBuffers_.front();
    freeBuffers_.pop();
  }

  std::unique_ptr<Request> request = camera_->createRequest();
  if (!request) {
    // Return buffer to pool
    std::lock_guard<std::mutex> lock(bufferMutex_);
    freeBuffers_.push(buffer);
    qDebug() << "[ERROR] Failed to create request";
    return false;
  }

  Stream *stream = config_->at(0).stream();
  int ret = request->addBuffer(stream, buffer);
  if (ret < 0) {
    std::lock_guard<std::mutex> lock(bufferMutex_);
    freeBuffers_.push(buffer);
    qDebug() << "[ERROR] Failed to add buffer to request:" << ret;
    return false;
  }

  ret = camera_->queueRequest(request.release());
  if (ret < 0) {
    std::lock_guard<std::mutex> lock(bufferMutex_);
    freeBuffers_.push(buffer);
    qDebug() << "[ERROR] Failed to queue request:" << ret;
    return false;
  }

  return true;
}

void LibCameraWorker::requestComplete(Request *request) {
  if (request->status() == Request::RequestCancelled) {
    return;
  }

  // Process the completed buffers
  const Request::BufferMap &buffers = request->buffers();
  
  // Convert buffer to image and emit signal
  QImage image = convertBufferToImage(buffers);
  if (!image.isNull()) {
    emit frameReady(image);
  }

  // Return buffers to the free pool
  {
    std::lock_guard<std::mutex> lock(bufferMutex_);
    for (auto &[stream, buffer] : buffers) {
      freeBuffers_.push(buffer);
    }
  }

  // Queue another request to keep the pipeline running
  queueRequest(CaptureMode::Preview);
}
