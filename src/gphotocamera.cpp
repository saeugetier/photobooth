#include "gphotocamera.h"
#include <gphoto2/gphoto2-camera.h>
#include <gphoto2/gphoto2-context.h>
#include <gphoto2/gphoto2-list.h>
#include <gphoto2/gphoto2-port.h>
#include <QVideoFrame>
#include <QDebug>

namespace {
constexpr auto capturingFailLimit = 10;
}

GPhotoCameraDevice::GPhotoCameraDevice() : mWorker(new GPhotoCameraWorker()) {
  mWorker->moveToThread(&mWorkerThread);

  connect(this, &QVideoFrameInput::readyToSendVideoFrame, mWorker.get(),
          &GPhotoCameraWorker::getPreviewFrame);
  connect(mWorker.get(), &GPhotoCameraWorker::frameReady, this,
          &GPhotoCameraDevice::onFrameReady);

  mWorkerThread.start();
}

GPhotoCameraDevice::~GPhotoCameraDevice() {
  mWorkerThread.quit();
  mWorkerThread.wait();
}

QString GPhotoCameraDevice::getDefautCamera() const {
  QStringList cameras = availableCameras();
  if (!cameras.isEmpty()) {
    return cameras.first();
  }
  return QString();
}

QStringList GPhotoCameraDevice::availableCameras() const {
  QStringList result;
  QMetaObject::invokeMethod(mWorker.get(), "availableCameras",
                            Qt::BlockingQueuedConnection,
                            Q_RETURN_ARG(QStringList, result));
  return result;
}

void GPhotoCameraDevice::onFrameReady(const QVideoFrame &frame) {
  sendVideoFrame(frame);
}

GPhotoCameraWorker::GPhotoCameraWorker()
    : mContext(gp_context_new(), gp_context_unref),
      mPortInfoList(nullptr, gp_port_info_list_free),
      mAbilitiesList(nullptr, gp_abilities_list_free),
      mCamera(nullptr, gp_camera_free), mPreviewFile(nullptr, gp_file_free) {
  GPPortInfoList *piList;
  gp_port_info_list_new(&piList);
  mPortInfoList.reset(piList);

  CameraAbilitiesList *caList;
  gp_abilities_list_new(&caList);
  mAbilitiesList.reset(caList);
}
GPhotoCameraWorker::~GPhotoCameraWorker() {}

void GPhotoCameraWorker::startCamera(const QString &cameraName) {
  // Implement camera start logic using gPhoto2
}

void GPhotoCameraWorker::stopCamera() {
  mCameraStarted = false;
  mCamera.reset();
  mPreviewFile.reset();
}

void GPhotoCameraWorker::captureImage() {
  // Implement image capture logic using gPhoto2
}

QStringList GPhotoCameraWorker::availableCameras() const {
  QStringList cameraList;

  CameraList *list;
  gp_list_new(&list);
  gp_camera_autodetect(list, mContext.get());

  int count = gp_list_count(list);
  for (int i = 0; i < count; i++) {
    const char *name;
    const char *value;
    gp_list_get_name(list, i, &name);
    gp_list_get_value(list, i, &value);
    cameraList.append(QString("%1 (%2)").arg(name).arg(value));
  }

  gp_list_free(list);

  return cameraList;
}

void GPhotoCameraWorker::getPreviewFrame() {
  if (!mCameraStarted) {
    emit errorOccurred("Camera not started");
    return;
  } else {

    gp_file_clean(mPreviewFile.get());

    auto ret =
        gp_camera_capture_preview(mCamera.get(), mPreviewFile.get(), mContext.get());
    if (GP_OK == ret) {
      const char *data = nullptr;
      unsigned long int size = 0;
      ret = gp_file_get_data_and_size(mPreviewFile.get(), &data, &size);
      if (GP_OK == ret) {
        mCapturingFailCount = 0;
        if (!QThread::currentThread()->isInterruptionRequested()) {
          auto image = QImage::fromData(QByteArray(data, int(size))).convertToFormat(QImage::Format_RGB32);
            emit frameReady(QVideoFrame(image));
        }
        return;
      }
    }

    qWarning() << "GPhoto: Failed retrieving preview" << ret;
    ++mCapturingFailCount;

    if (capturingFailLimit < mCapturingFailCount) {
      qWarning() << "GPhoto: Closing camera because of capturing fail";
      emit errorOccurred(tr("Unable to capture frame"));
      stopCamera();
    }
  }
}
