#pragma once
#include <QVideoFrameInput>
#include <QThread>
#include <QStringList>

#include <gphoto2/gphoto2-abilities-list.h>
#include <gphoto2/gphoto2-context.h>
#include <gphoto2/gphoto2-port.h>


class GPhotoCameraWorker;

using CameraAbilitiesListPtr = std::unique_ptr<CameraAbilitiesList, int (*)(CameraAbilitiesList*)>;
using GPContextPtr = std::unique_ptr<GPContext, void (*)(GPContext*)>;
using GPPortInfoListPtr = std::unique_ptr<GPPortInfoList, int (*)(GPPortInfoList*)>;

class GPhotoCameraDevice : public QVideoFrameInput
{
    Q_OBJECT
public:
    GPhotoCameraDevice();
    ~GPhotoCameraDevice() override;

    Q_INVOKABLE QStringList availableCameras() const;

    Q_INVOKABLE QString getDefautCamera() const;
protected:
    QThread mWorkerThread;

protected slots:
    void onFrameReady(const QVideoFrame &frame);

protected:
    // worker object for camera operations in thread
    std::unique_ptr<GPhotoCameraWorker> mWorker;
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
protected:
    GPContextPtr mContext;
    GPPortInfoListPtr mPortInfoList;
    CameraAbilitiesListPtr mAbilitiesList;
    // Add private members for camera handling
};


