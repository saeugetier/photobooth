#pragma once
#include <QVideoFrameInput>
#include <QThread>
#include <QStringList>


class GPhotoCameraWorker;

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
private:
    // Add private members for camera handling
};


