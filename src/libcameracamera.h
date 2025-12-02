#pragma once
#include <QObject>
#include <QImage>
#include <QTimer>
#include <QStringList>
#include <memory>

#undef emit
#undef slots
#include <libcamera/camera_manager.h>
#include <libcamera/camera.h>
#include <libcamera/controls.h>
#include <libcamera/framebuffer_allocator.h>
#define slots Q_SLOTS
#define emit Q_EMIT

class LibCameraWorker : public QObject
{
    Q_OBJECT
public:
    explicit LibCameraWorker(QObject *parent = nullptr);
    ~LibCameraWorker() override;

public Q_SLOTS:
    void startCamera(const QString &cameraId); // cameraId as returned by CameraManager
    void stopCamera();
    void captureImage();                        // single capture -> emits imageCaptured
    QStringList availableCameras() const;

Q_SIGNALS:
    void frameReady(const QImage &image);      // optionally emit preview frames
    void imageCaptured(const QImage &image);   // emitted after captureImage
    void errorOccurred(const QString &err);

private:
    void initCameraManager();
    void configureCamera(int width, int height);
    void queueViewfinderRequest();
    QImage convertBufferToImage(const libcamera::FrameBuffer &fb);

    std::unique_ptr<libcamera::CameraManager> mCameraManager;
    std::shared_ptr<libcamera::Camera> mCamera;
    std::unique_ptr<libcamera::FrameBufferAllocator> mAllocator;

    std::vector<std::unique_ptr<libcamera::FrameBuffer>> mBuffers;
    std::atomic<bool> mRunning{false};
    QTimer mPreviewTimer; // optional periodic preview or parameter checks
    int mRequestedWidth{640}, mRequestedHeight{480};
};
