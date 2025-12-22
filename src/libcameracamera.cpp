#include "libcameracamera.h"
#include <QThread>
#include <QDebug>
#include <chrono>
#include <gphoto2/gphoto2-list.h>
#include <thread>
#include <libcamera/formats.h>
#include <libcamera/framebuffer_allocator.h>
#include <qvideoframe.h>
#include <sys/mman.h>
#include <iostream>
#include <iomanip>

using namespace libcamera;

LibcameraDevice::LibcameraDevice(QObject *parent)
    : QVideoFrameInput(parent),
      mWorkerThread(std::make_unique<QThread>(this))
{
    // Create worker
    mWorker = new LibCameraWorker();
    mWorker->moveToThread(mWorkerThread.get());

    connect(this, &QVideoFrameInput::readyToSendVideoFrame, mWorker,
          &LibCameraWorker::queueViewfinderRequest);

    // Connect worker signals to device signals (relay/forward)
    connect(mWorker, &LibCameraWorker::frameReady,
            this, &LibcameraDevice::onFrameReady);
    connect(mWorker, &LibCameraWorker::imageCaptured,
            this, &LibcameraDevice::onImageCaptured);
    connect(mWorker, &LibCameraWorker::errorOccurred,
            this, &LibcameraDevice::onErrorOccurred);

    // Connect device slots to worker slots
    connect(this, &LibcameraDevice::startCamera,
            mWorker, &LibCameraWorker::startCamera);
    connect(this, &LibcameraDevice::stopCamera,
            mWorker, &LibCameraWorker::stopCamera);
    connect(this, &LibcameraDevice::captureImage,
            mWorker, &LibCameraWorker::captureImage);

    // Clean up when thread finishes
    connect(mWorkerThread.get(), &QThread::finished,
            mWorker, &QObject::deleteLater);

    // Start thread
    mWorkerThread->start();

    qDebug() << "[INFO] LibcameraDevice initialized";
}

LibcameraDevice::~LibcameraDevice()
{
    if (mWorkerThread) {
        mWorkerThread->quit();
        mWorkerThread->wait(3000);
    }
    qDebug() << "[INFO] LibcameraDevice destroyed";
}

QStringList LibcameraDevice::availableCameras() const
{
    QStringList cameras;
    if (mWorker) {
        // Block until worker returns result
        QMetaObject::invokeMethod(mWorker, [this, &cameras]() {
            cameras = mWorker->availableCameras();
        }, Qt::BlockingQueuedConnection);
    }
    return cameras;
}

QString LibcameraDevice::getDefaultCamera() const
{
    QStringList cameras = availableCameras();
    return cameras.isEmpty() ? QString() : cameras.first();
}

void LibcameraDevice::startCamera(const QString &cameraId)
{
    QMetaObject::invokeMethod(mWorker, "startCamera",
                            Qt::QueuedConnection,
                            Q_ARG(QString, cameraId));
}

void LibcameraDevice::stopCamera()
{
    QMetaObject::invokeMethod(mWorker, "stopCamera",
                            Qt::QueuedConnection);
}

void LibcameraDevice::captureImage()
{
    QMetaObject::invokeMethod(mWorker, "captureImage",
                            Qt::QueuedConnection);
}

void LibcameraDevice::onFrameReady(const QImage &image)
{
    sendVideoFrame(QVideoFrame(image));
}

void LibcameraDevice::onImageCaptured(const QImage &image)
{
    emit imageCaptured(image);
}

void LibcameraDevice::onErrorOccurred(const QString &error)
{
    emit errorOccurred(error);
}

LibCameraWorker::LibCameraWorker(QObject *parent)
    : QObject(parent)
{
    initCameraManager();
}

LibCameraWorker::~LibCameraWorker()
{
    stopCamera();
    if (mCameraManager)
        mCameraManager->stop();
}

void LibCameraWorker::initCameraManager()
{
    mCameraManager = std::make_unique<CameraManager>();
    int ret = mCameraManager->start();
    if (ret) {
        emit errorOccurred(QString::asprintf("libcamera: CameraManager start failed: %d", ret));
        mCameraManager.reset();
    }
}

QStringList LibCameraWorker::availableCameras() const
{
    QStringList list;
    if (!mCameraManager) return list;
    for (auto const &id : mCameraManager->cameras())
        list << QString::fromStdString(id->id());
    return list;
}

void LibCameraWorker::startCamera(const QString &cameraId)
{
    if (!mCameraManager) {
        emit errorOccurred("libcamera: CameraManager not initialized");
        return;
    }

    if (mRunning) stopCamera();

    // find camera by id
    std::shared_ptr<Camera> cam = mCameraManager->get(cameraId.toStdString());
    if (!cam) {
        emit errorOccurred("libcamera: camera not found: " + cameraId);
        return;
    }
    mCamera = cam;

    mCamera->acquire();

    // configure camera for viewfinder role (basic example)
    std::unique_ptr<CameraConfiguration> config =
        mCamera->generateConfiguration({ StreamRole::Viewfinder });
    if (!config) {
        emit errorOccurred("libcamera: failed to generate configuration");
        mCamera.reset();
        return;
    }

    // Get default size and aspect ratio
    unsigned int defaultWidth = config->at(0).size.width;
    unsigned int defaultHeight = config->at(0).size.height;
    float aspectRatio = static_cast<float>(defaultWidth) / defaultHeight;

    qDebug() << "libcamera: generated configuration with default width " << defaultWidth << ", height " << defaultHeight << ", pixelFormat " << config->at(0).pixelFormat;

    // Set preview size: width = 640, height = 640 / aspectRatio (maintain aspect ratio)
    config->at(0).pixelFormat = formats::RGB888;
    config->at(0).size.width = 640;
    config->at(0).size.height = static_cast<unsigned int>(640.0f / aspectRatio);
    config->at(0).bufferCount = 4;

    if (config->validate() == CameraConfiguration::Invalid) {
        emit errorOccurred("libcamera: configuration invalid");
        mCamera.reset();
        return;
    }

    if (mCamera->configure(config.get()) < 0) {
        emit errorOccurred("libcamera: camera configure failed");
        mCamera.reset();
        return;
    }

    // allocate buffers
    mAllocator = std::make_unique<FrameBufferAllocator>(mCamera);
    for (StreamConfiguration &cfg : *config) {
        Stream *stream = cfg.stream();
        if (mAllocator->allocate(stream) < 0) {
            emit errorOccurred("libcamera: buffer allocation failed");
            mCamera.reset();
            return;
        }
        const std::vector<std::unique_ptr<FrameBuffer>> &bufs = mAllocator->buffers(stream);
        for (auto &b : bufs)
            mBuffers.push_back(std::unique_ptr<FrameBuffer>(b.get()));
    }

    // start camera
    if (mCamera->start() < 0) {
        emit errorOccurred("libcamera: start failed");
        mCamera.reset();
        return;
    }

    mCamera->requestCompleted.connect(this, &LibCameraWorker::processCompletedRequest);

    mRunning = true;

    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    queueViewfinderRequest();
}

void LibCameraWorker::stopCamera()
{
    if (!mRunning) return;

    if (mCamera) {
        mCamera->stop();
        if (mAllocator) {
            // free buffers
            mAllocator.reset();
        }
        mCamera.reset();
    }
    mBuffers.clear();
    mRunning = false;
}

void LibCameraWorker::captureImage()
{
    if (!mRunning || !mCamera) {
        emit errorOccurred("libcamera: camera not running");
        return;
    }

    // Create a request and keep it alive until completion
    std::unique_ptr<Request> request = mCamera->createRequest();
    if (!request) {
        emit errorOccurred("libcamera: createRequest failed");
        return;
    }

    // attach first available buffer for viewfinder stream (simplified)
    Stream *stream = *mCamera->streams().begin();
    if (mBuffers.empty()) {
        emit errorOccurred("libcamera: no buffers available");
        return;
    }
    FrameBuffer *fb = mBuffers.front().get();
    if (request->addBuffer(stream, fb) < 0) {
        emit errorOccurred("libcamera: addBuffer failed");
        return;
    }

    // queue request
    if (mCamera->queueRequest(request.get()) < 0) {
        emit errorOccurred("libcamera: queueRequest failed");
        return;
    }

    // wait for completion (polling). This runs in the worker thread so blocking is acceptable.
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (std::chrono::steady_clock::now() < deadline) {
        if (request->status() != Request::RequestPending)
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    if (request->status() != Request::RequestComplete) {
        emit errorOccurred("libcamera: capture failed or timed out");
        // let request be destroyed / cleaned up
        return;
    }

    // find buffer and convert
    if (!request->buffers().empty()) {
        auto it = request->buffers().begin();
        FrameBuffer *completedFb = it->second;
        QImage img;// = convertBufferToImage(completedFb);
        emit imageCaptured(img);
    }
    // request destroyed when leaving scope
}

void LibCameraWorker::queueViewfinderRequest()
{
    if (!mCamera || mBuffers.empty()) return;

    // Create a request
    std::shared_ptr<Request> request = mCamera->createRequest();
    if (!request) {
        Q_EMIT errorOccurred("libcamera: createRequest failed for preview");
        return;
    }

    // Get next buffer (rotate through available buffers)
    FrameBuffer *fb = mBuffers[mBufferIndex % mBuffers.size()].get();
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

void LibCameraWorker::processCompletedRequest(Request *request)
{
    if (!request || request->buffers().empty()) return;

    const std::map<const Stream *, FrameBuffer *> &buffers = request->buffers();

    // Convert buffer to QImage and emit preview
    QImage preview = convertBufferToImage(buffers);
    if (!preview.isNull())
        Q_EMIT frameReady(preview);

    auto it = std::find_if(mPendingRequests.begin(), mPendingRequests.end(),
                           [request](const std::shared_ptr<Request> &ptr) {
                             return ptr.get() == request;
                           });
    if (it != mPendingRequests.end())
        mPendingRequests.erase(it);
}

QImage LibCameraWorker::convertBufferToImage(const std::map<const Stream *, FrameBuffer *> &buffers)
{
    // Create QImage from RGB888 data (copy data)
    QImage image;
    
    for (auto bufferPair : buffers)
    {
        // Use framebuffer which has the image data
        FrameBuffer *buffer = bufferPair.second;

        // Find the size of buffer
        size_t size = buffer->metadata().planes()[0].bytesused;
        const FrameBuffer::Plane &plane = buffer->planes().front();
        void *memory = mmap(NULL, plane.length, PROT_READ, MAP_SHARED, plane.fd.get(), 0);

        // Load image from a raw buffer into the QImage widget
        image.loadFromData(static_cast<unsigned char *>(memory), (int)size);
    }

    return image;
}

