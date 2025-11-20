#pragma once
#include <QVideoFrameInput>
#include <QThread>
#include <QStringList>
#include <QImage>
#include <QTimer>
#include <QVariant>
#include <memory>

#include <cstdint>
#include <gphoto2/gphoto2-abilities-list.h>
#include <gphoto2/gphoto2-context.h>
#include <gphoto2/gphoto2-port.h>
#include <gphoto2/gphoto2-camera.h>


class GPhotoCameraWorker;

using CameraAbilitiesListPtr = std::unique_ptr<CameraAbilitiesList, int (*)(CameraAbilitiesList*)>;
using GPContextPtr = std::unique_ptr<GPContext, void (*)(GPContext*)>;
using GPPortInfoListPtr = std::unique_ptr<GPPortInfoList, int (*)(GPPortInfoList*)>;
using CameraFilePtr = std::unique_ptr<CameraFile, int (*)(CameraFile*)>;
using CameraPtr = std::unique_ptr<Camera, int (*)(Camera*)>;

class GPhotoCameraDevice : public QVideoFrameInput
{
    Q_OBJECT
    Q_PROPERTY(QString cameraName READ getCameraName WRITE setCameraName)
public:
    GPhotoCameraDevice();
    ~GPhotoCameraDevice() override;

    Q_INVOKABLE QStringList availableCameras() const;

    Q_INVOKABLE QString getDefautCamera() const;

    Q_INVOKABLE void captureImage() const;

    QString getCameraName() const { return mCameraName; }
    void setCameraName(const QString &name);
protected:
    QThread mWorkerThread;
    QString mCameraName;
    bool mPreviewStarted = false;

protected slots:
    void onFrameReady(const QVideoFrame &frame);

protected:
    // worker object for camera operations in thread
    std::unique_ptr<GPhotoCameraWorker> mWorker;

signals:
    void errorOccurred(const QString &error);
    void imageCaptured(const QImage &image);
    void captureError(const QString &error);
};

class GPhotoCameraWorker : public QObject
{
    Q_OBJECT
public:
    GPhotoCameraWorker();
    ~GPhotoCameraWorker() override; 
public slots:
    void startCamera(const QString &cameraName);
    void stopCamera();
    void captureImage();

    void getPreviewFrame();
    
    QStringList availableCameras() const;
signals:
    void frameReady(const QVideoFrame &frame);
    void errorOccurred(const QString &error);
    void imageCaptured(const QImage &image);
    void captureError(const QString &error);
protected:
    QTimer mKeepAliveTimer;
    GPContextPtr mContext;
    GPPortInfoListPtr mPortInfoList;
    CameraAbilitiesListPtr mAbilitiesList;
    CameraPtr mCamera;
    CameraFilePtr mPreviewFile;
    bool mCameraStarted = false;
    uint32_t mCapturingFailCount = 0;

    void waitForOperationCompleted();
    QVariant parameter(const QString &name);
    bool setParameter(const QString &name, const QVariant &value);
protected slots:
    // check and set capture parameters to keep camera alive
    void checkCaptureParameter();
};


