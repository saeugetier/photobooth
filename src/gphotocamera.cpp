#include "gphotocamera.h"
#include <gphoto2/gphoto2-camera.h>
#include <gphoto2/gphoto2-context.h>

GPhotoCameraDevice::GPhotoCameraDevice() : mWorker(new GPhotoCameraWorker() )
{
    mWorker->moveToThread(&mWorkerThread);

    connect(this, &QVideoFrameInput::readyToSendVideoFrame, mWorker.get(), &GPhotoCameraWorker::getPreviewFrame);
    connect(mWorker.get(), &GPhotoCameraWorker::frameReady, this, &GPhotoCameraDevice::onFrameReady);

    mWorkerThread.start();
}   

GPhotoCameraDevice::~GPhotoCameraDevice()
{
    mWorkerThread.quit();
    mWorkerThread.wait();
}

QString GPhotoCameraDevice::getDefautCamera() const
{
    QStringList cameras = availableCameras();
    if (!cameras.isEmpty()) {
        return cameras.first();
    }
    return QString();
}

QStringList GPhotoCameraDevice::availableCameras() const
{
    QStringList result;
    QMetaObject::invokeMethod(mWorker.get(), "availableCameras", Qt::BlockingQueuedConnection,
                              Q_RETURN_ARG(QStringList, result));
    return result;
}

void GPhotoCameraDevice::onFrameReady(const QVideoFrame &frame)
{
    sendVideoFrame(frame);
}

GPhotoCameraWorker::GPhotoCameraWorker()
{
}
GPhotoCameraWorker::~GPhotoCameraWorker()
{
}

void GPhotoCameraWorker::startCamera(const QString &cameraName)
{
    // Implement camera start logic using gPhoto2
}

void GPhotoCameraWorker::stopCamera()
{
    // Implement camera stop logic using gPhoto2
}

void GPhotoCameraWorker::captureImage()
{
    // Implement image capture logic using gPhoto2
}

QStringList GPhotoCameraWorker::availableCameras() const
{
    QStringList cameraList;

    GPContext *context = gp_context_new();
    CameraList *list;
    gp_list_new(&list);
    gp_camera_autodetect(list, context);

    int count = gp_list_count(list);
    for (int i = 0; i < count; i++) {
        const char *name;
        const char *value;
        gp_list_get_name(list, i, &name);
        gp_list_get_value(list, i, &value);
        cameraList.append(QString("%1 (%2)").arg(name).arg(value));
    }

    gp_list_free(list);
    gp_context_unref(context);

    return cameraList;
}

void GPhotoCameraWorker::getPreviewFrame()
{
    // Implement logic to get a preview frame from the camera and emit frameReady signal
}


