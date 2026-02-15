#pragma once
#include <QImage>
#include <QObject>
#include <QStringList>
#include <QTimer>
#include <QVariantList>
#include <QVideoFrameInput>
#include <memory>
#include <deque>
#include <set>
#include <mutex>

#undef emit
#undef slots
#include <libcamera/camera.h>
#include <libcamera/camera_manager.h>
#include <libcamera/framebuffer_allocator.h>
#include <libcamera/stream.h>

#define slots Q_SLOTS
#define emit Q_EMIT

class QThread;

class LibCameraWorker : public QObject {
  Q_OBJECT
public:
  explicit LibCameraWorker(QObject *parent = nullptr);
  ~LibCameraWorker();

  QVariantList availableCameras() const;

public slots:
  void startCamera(const QString &cameraId);
  void stopCamera();
  void captureImage();
  void resumeViewfinder();
  void queueViewfinderRequest();

signals:
  void frameReady(const QImage &image);
  void imageCaptured(const QImage &image);
  void errorOccurred(const QString &error);
  void captureCompleted();  // Add this

private:
  void initCameraManager();
  bool configureCamera(libcamera::StreamRole role);
  void processCompletedRequest(libcamera::Request *request);
  void processCaptureComplete(libcamera::Request *request);
  void queueCaptureRequest();
  
  QImage convertBufferToImage(
      const std::map<const libcamera::Stream *, libcamera::FrameBuffer *> &buffers);

  void queueViewfinderRequestLocked();

  std::unique_ptr<libcamera::CameraManager> mCameraManager;
  std::shared_ptr<libcamera::Camera> mCamera;
  std::unique_ptr<libcamera::CameraConfiguration> mConfig;
  std::unique_ptr<libcamera::FrameBufferAllocator> mAllocator;
  libcamera::Stream *mStream = nullptr;
  
  // Buffer management - use deque for free buffers, set for in-flight tracking
  std::deque<libcamera::FrameBuffer *> mFreeBuffers;
  std::set<libcamera::FrameBuffer *> mBuffersInFlight;

  std::mutex mBufferMutex;

  bool mRunning = false;
  bool mCaptureInProgress = false;
  int mCaptureRetryCount = 0;
  unsigned int mCurrentWidth = 0;
  unsigned int mCurrentHeight = 0;
};

class LibcameraDevice : public QVideoFrameInput {
  Q_OBJECT
public:
  explicit LibcameraDevice(QObject *parent = nullptr);
  ~LibcameraDevice();

  Q_INVOKABLE QVariantList availableCameras() const;
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
  LibCameraWorker *mWorker = nullptr;
  std::unique_ptr<QThread> mWorkerThread;
};
