#include "libcameracamera.h"
#include <QThread>
#include <QDebug>
#include <chrono>
#include <thread>
#include <libcamera/formats.h>
#include <libcamera/framebuffer_allocator.h>

using namespace libcamera;

LibCameraWorker::LibCameraWorker(QObject *parent)
    : QObject(parent)
{
    initCameraManager();
    connect(&mPreviewTimer, &QTimer::timeout, this, [this](){
        if (mRunning && mCamera)
            queueViewfinderRequest(); // optional: grab periodic preview frames
    });
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
        Q_EMIT errorOccurred(QString::asprintf("libcamera: CameraManager start failed: %d", ret));
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
        Q_EMIT errorOccurred("libcamera: CameraManager not initialized");
        return;
    }

    if (mRunning) stopCamera();

    // find camera by id
    std::shared_ptr<Camera> cam = mCameraManager->get(cameraId.toStdString());
    if (!cam) {
        Q_EMIT errorOccurred("libcamera: camera not found: " + cameraId);
        return;
    }
    mCamera = cam;

    // configure camera for viewfinder role (basic example)
    std::unique_ptr<CameraConfiguration> config =
        mCamera->generateConfiguration({ StreamRole::Viewfinder });
    if (!config) {
        Q_EMIT errorOccurred("libcamera: failed to generate configuration");
        mCamera.reset();
        return;
    }

    // choose size; you can support both 320 and 640 by selecting based on requested size
    config->at(0).pixelFormat = formats::RGB888; // prefer RGB if supported; else use YUV and convert
    config->at(0).size.width = mRequestedWidth;
    config->at(0).size.height = mRequestedHeight;
    config->at(0).bufferCount = 4;

    if (config->validate() == CameraConfiguration::Invalid) {
        Q_EMIT errorOccurred("libcamera: configuration invalid");
        mCamera.reset();
        return;
    }

    if (mCamera->configure(config.get()) < 0) {
        Q_EMIT errorOccurred("libcamera: camera configure failed");
        mCamera.reset();
        return;
    }

    // allocate buffers
    mAllocator = std::make_unique<FrameBufferAllocator>(mCamera);
    for (StreamConfiguration &cfg : *config) {
        Stream *stream = cfg.stream();
        if (mAllocator->allocate(stream) < 0) {
            Q_EMIT errorOccurred("libcamera: buffer allocation failed");
            mCamera.reset();
            return;
        }
        const std::vector<std::unique_ptr<FrameBuffer>> &bufs = mAllocator->buffers(stream);
        for (auto &b : bufs)
            mBuffers.push_back(std::unique_ptr<FrameBuffer>(b.get()));
    }

    // start camera
    if (mCamera->start() < 0) {
        Q_EMIT errorOccurred("libcamera: start failed");
        mCamera.reset();
        return;
    }

    mRunning = true;
    mPreviewTimer.start(100); // optional: request preview every 100ms
}

void LibCameraWorker::stopCamera()
{
    if (!mRunning) return;

    mPreviewTimer.stop();

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
        Q_EMIT errorOccurred("libcamera: camera not running");
        return;
    }

    // Create a request and keep it alive until completion
    std::unique_ptr<Request> request = mCamera->createRequest();
    if (!request) {
        Q_EMIT errorOccurred("libcamera: createRequest failed");
        return;
    }

    // attach first available buffer for viewfinder stream (simplified)
    Stream *stream = *mCamera->streams().begin();
    if (mBuffers.empty()) {
        Q_EMIT errorOccurred("libcamera: no buffers available");
        return;
    }
    FrameBuffer *fb = mBuffers.front().get();
    if (request->addBuffer(stream, fb) < 0) {
        Q_EMIT errorOccurred("libcamera: addBuffer failed");
        return;
    }

    // queue request
    if (mCamera->queueRequest(request.get()) < 0) {
        Q_EMIT errorOccurred("libcamera: queueRequest failed");
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
        Q_EMIT errorOccurred("libcamera: capture failed or timed out");
        // let request be destroyed / cleaned up
        return;
    }

    // find buffer and convert
    if (!request->buffers().empty()) {
        auto it = request->buffers().begin();
        FrameBuffer *completedFb = it->second;
        QImage img = convertBufferToImage(*completedFb);
        Q_EMIT imageCaptured(img);
    }
    // request destroyed when leaving scope
}

void LibCameraWorker::queueViewfinderRequest()
{
    if (!mCamera) return;
    // Similar to captureImage but re-use buffers and emit frameReady for previews.
    // TODO: implement re-queueing multiple requests, handle Request::completed, reuse buffers
}

QImage LibCameraWorker::convertBufferToImage(const FrameBuffer &fb)
{
    // TODO: implement conversion from FrameBuffer payload to QImage.
    // If fb.pixelFormat() == formats::RGB888 you can construct QImage directly from buffer.
    // If YUV, use conversion (libyuv/opencv) to RGB.
    // Placeholder empty image:
    return QImage();
}
