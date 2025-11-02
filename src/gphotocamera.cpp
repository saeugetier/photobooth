#include "gphotocamera.h"
#include <QDebug>
#include <QString>
#include <QVideoFrame>
#include <gphoto2/gphoto2-camera.h>
#include <gphoto2/gphoto2-context.h>
#include <gphoto2/gphoto2-list.h>
#include <gphoto2/gphoto2-port.h>

namespace {
constexpr auto capturingFailLimit = 10;
}

GPhotoCameraDevice::GPhotoCameraDevice() : mWorker(new GPhotoCameraWorker()) {
  mWorker->moveToThread(&mWorkerThread);

  connect(this, &QVideoFrameInput::readyToSendVideoFrame, mWorker.get(),
          &GPhotoCameraWorker::getPreviewFrame);
  connect(mWorker.get(), &GPhotoCameraWorker::frameReady, this,
          &GPhotoCameraDevice::onFrameReady);
  connect(mWorker.get(), &GPhotoCameraWorker::errorOccurred,
            this, &GPhotoCameraDevice::errorOccurred);

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

void GPhotoCameraDevice::setCameraName(const QString &name) {
  mCameraName = name;

  if( mCameraName.isEmpty()) {
    QMetaObject::invokeMethod(mWorker.get(), "stopCamera",
                              Qt::QueuedConnection);
  }
  else {
    QMetaObject::invokeMethod(mWorker.get(), "startCamera",
                            Qt::QueuedConnection, Q_ARG(QString, name));
  }
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
  if (mCameraStarted) {
    stopCamera();
  }

  // Parse camera name and port from format "CameraName (port)"
  QString name = cameraName.section(" (", 0, 0);
  QString port = cameraName.section(" (", 1).chopped(1);

  // Initialize port info list
  gp_port_info_list_load(mPortInfoList.get());

  // Load camera abilities
  gp_abilities_list_load(mAbilitiesList.get(), mContext.get());

  // Create new camera object
  Camera *camera;
  gp_camera_new(&camera);
  mCamera.reset(camera);

  // Find camera abilities
  CameraAbilities abilities;
  int abilitiesIndex = gp_abilities_list_lookup_model(
      mAbilitiesList.get(), name.toLatin1().constData());
  if (abilitiesIndex < 0) {
    emit errorOccurred(tr("Camera %1 not found").arg(name));
    return;
  }
  gp_abilities_list_get_abilities(mAbilitiesList.get(), abilitiesIndex,
                                  &abilities);
  gp_camera_set_abilities(mCamera.get(), abilities);

  // Find and set port
  int portIndex = gp_port_info_list_lookup_path(mPortInfoList.get(),
                                                port.toLatin1().constData());
  if (portIndex < 0) {
    emit errorOccurred(tr("Port %1 not found").arg(port));
    return;
  }
  GPPortInfo portInfo;
  gp_port_info_list_get_info(mPortInfoList.get(), portIndex, &portInfo);
  gp_camera_set_port_info(mCamera.get(), portInfo);

  // Initialize camera connection
  int ret = gp_camera_init(mCamera.get(), mContext.get());
  if (ret < GP_OK) {
    emit errorOccurred(tr("Failed to initialize camera: %1").arg(ret));
    mCamera.reset();
    return;
  }

  // Create preview file handle
  CameraFile *file;
  gp_file_new(&file);
  mPreviewFile.reset(file);

  mCameraStarted = true;

  getPreviewFrame();
}

void GPhotoCameraWorker::stopCamera() {
  if (!mCameraStarted) {
    return;
  }

  // Reset capturing fail counter
  mCapturingFailCount = 0;

  // Close camera connection first
  if (mCamera) {
    gp_camera_exit(mCamera.get(), mContext.get());
  }

  // Reset smart pointers in specific order
  mPreviewFile.reset(); // Release preview file handle first
  mCamera.reset();      // Release camera handle next

  mCameraStarted = false;
}

void GPhotoCameraWorker::captureImage() {
  // Implement image capture logic using gPhoto2
  if (!mCameraStarted || !mCamera) {
    emit captureError("Camera not started");
    return;
  }

  // Create a new file for the capture
  CameraFile *file;
  gp_file_new(&file);
  CameraFilePtr captureFile(file, gp_file_free);

  // Capture the image
  CameraFilePath camera_file_path;
  int ret = gp_camera_capture(mCamera.get(), GP_CAPTURE_IMAGE,
                              &camera_file_path, mContext.get());
  if (ret < GP_OK) {
    emit captureError(tr("Failed to capture image: %1").arg(ret));
    return;
  }

  // Download the image from camera
  ret = gp_camera_file_get(mCamera.get(), camera_file_path.folder,
                           camera_file_path.name, GP_FILE_TYPE_NORMAL,
                           captureFile.get(), mContext.get());
  if (ret < GP_OK) {
    emit captureError(tr("Failed to download image: %1").arg(ret));
    return;
  }

  // Get the image data
  const char *data;
  unsigned long int size = 0;
  ret = gp_file_get_data_and_size(captureFile.get(), &data, &size);
  if (ret < GP_OK) {
    emit captureError(tr("Failed to get image data: %1").arg(ret));
    return;
  }

  // Convert to QImage and emit
  QImage image = QImage::fromData(QByteArray(data, int(size)));
  if (image.isNull()) {
    emit captureError("Failed to convert image data");
    return;
  }

  // Delete the file from camera
  gp_camera_file_delete(mCamera.get(), camera_file_path.folder,
                        camera_file_path.name, mContext.get());

  emit imageCaptured(image);
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

    auto ret = gp_camera_capture_preview(mCamera.get(), mPreviewFile.get(),
                                         mContext.get());
    if (GP_OK == ret) {
      const char *data = nullptr;
      unsigned long int size = 0;
      ret = gp_file_get_data_and_size(mPreviewFile.get(), &data, &size);
      if (GP_OK == ret) {
        mCapturingFailCount = 0;
        if (!QThread::currentThread()->isInterruptionRequested()) {
          auto image = QImage::fromData(QByteArray(data, int(size)))
                           .convertToFormat(QImage::Format_RGB32);
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
