#pragma once
#include <QImage>
#include <QObject>
#include <QStringList>
#include <QTimer>
#include <QVideoFrameInput>
#include <memory>

#undef emit
#undef slots
#include <libcamera/camera.h>
#include <libcamera/camera_manager.h>
#include <libcamera/controls.h>
#include <libcamera/framebuffer_allocator.h>
#define slots Q_SLOTS
#define emit Q_EMIT

class LibCameraWorker;

class LibcameraDevice : public QVideoFrameInput {
  Q_OBJECT
public:
  explicit LibcameraDevice(QObject *parent = nullptr);
  ~LibcameraDevice() override;

  Q_INVOKABLE QStringList availableCameras() const;
  Q_INVOKABLE QString getDefaultCamera() const;

public slots:
  Q_INVOKABLE void startCamera(const QString &cameraId);
  Q_INVOKABLE void stopCamera();
  Q_INVOKABLE void captureImage();

signals:
  void imageCaptured(const QImage &image);
  void errorOccurred(const QString &error);

private slots:
  void onFrameReady(const QImage &image);
  void onImageCaptured(const QImage &image);
  void onErrorOccurred(const QString &error);

private:
  std::unique_ptr<QThread> mWorkerThread;
  LibCameraWorker *mWorker = nullptr;
};

class LibCameraWorker : public QObject {
  Q_OBJECT
public:
  explicit LibCameraWorker(QObject *parent = nullptr);
  ~LibCameraWorker() override;

public slots:
  void startCamera(const QString &cameraId); // cameraId as returned by CameraManager
  void stopCamera();
  void captureImage();
  void queueViewfinderRequest(); // single capture -> emits imageCaptured
  QStringList availableCameras() const;

signals:
  void frameReady(const QImage &image);    // optionally emit preview frames
  void imageCaptured(const QImage &image); // emitted after captureImage
  void errorOccurred(const QString &err);

private:
  void initCameraManager();

  QImage convertBufferToImage(
      const std::map<const libcamera::Stream *, libcamera::FrameBuffer *>
          &buffers);

  std::unique_ptr<libcamera::CameraManager> mCameraManager;
  std::shared_ptr<libcamera::Camera> mCamera;
  std::unique_ptr<libcamera::FrameBufferAllocator> mAllocator;

  std::vector<libcamera::FrameBuffer *>
      mBuffers; // Changed to raw pointers to avoid ownership issues
  std::atomic<bool> mRunning{false};

  std::vector<std::shared_ptr<libcamera::Request>> mPendingRequests;
  size_t mBufferIndex = 0;
  void processCompletedRequest(libcamera::Request *request);

  unsigned int mCurrentWidth = 0;
  unsigned int mCurrentHeight = 0;

  void configureCamera(libcamera::StreamRole role);

  bool mCaptureInProgress = false;

  private:
    std::queue<FrameBuffer *> freeBuffers_;
    std::mutex bufferMutex_;
};
