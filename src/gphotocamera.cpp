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
  connect(mWorker.get(), &GPhotoCameraWorker::errorOccurred, this,
          &GPhotoCameraDevice::errorOccurred);
  connect(mWorker.get(), &GPhotoCameraWorker::imageCaptured, this,
          &GPhotoCameraDevice::imageCaptured);
  connect(mWorker.get(), &GPhotoCameraWorker::captureError, this,
          &GPhotoCameraDevice::captureError);

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

void GPhotoCameraDevice::captureImage() const {
  QMetaObject::invokeMethod(mWorker.get(), "captureImage",
                            Qt::QueuedConnection);
}

void GPhotoCameraDevice::setCameraName(const QString &name) {
  mCameraName = name;

  if (mCameraName.isEmpty()) {
    QMetaObject::invokeMethod(mWorker.get(), "stopCamera",
                              Qt::QueuedConnection);
  } else {
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

  mKeepAliveTimer.setInterval(1000 * 60); // Check every minute
  mKeepAliveTimer.setSingleShot(true);
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

  if (!mKeepAliveTimer.isActive()) {
    QMetaObject::invokeMethod(&mKeepAliveTimer, "start", Qt::QueuedConnection);
  }

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

    if(!mKeepAliveTimer.isActive()) {
      checkCaptureParameter();
      QMetaObject::invokeMethod(&mKeepAliveTimer, "start", Qt::QueuedConnection);
    }

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

void GPhotoCameraWorker::checkCaptureParameter() {
  // this function checks and sets the capture parameter to keep the camera
  // alive
  if (!mCameraStarted || !mCamera) {
    return;
  }

  QVariant captureParameter = parameter("capture");

  if (captureParameter.isValid())
  {
    if (captureParameter.toBool() == false)
    {
      setParameter("capture", true);
      qWarning() << "Set parameter 'capture' to 'true'";
    }
  }
}

void GPhotoCameraWorker::waitForOperationCompleted() {
  CameraEventType type;
  void *data;
  int ret;

  do {
    ret = gp_camera_wait_for_event(mCamera.get(), 10, &type, &data, mContext.get());
  } while ((ret == GP_OK) && (type != GP_EVENT_TIMEOUT));
}

QVariant GPhotoCameraWorker::parameter(const QString &name) {
  CameraWidget *root;
  int ret = gp_camera_get_config(mCamera.get(), &root, mContext.get());
  if (ret < GP_OK) {
    qWarning() << "Unable to get root option from gphoto";
    return QVariant();
  }

  CameraWidget *option;
  ret = gp_widget_get_child_by_name(root, qPrintable(name), &option);
  if (ret < GP_OK) {
    qWarning() << "Unable to get config widget from gphoto";
    return QVariant();
  }

  CameraWidgetType type;
  ret = gp_widget_get_type(option, &type);
  if (ret < GP_OK) {
    qWarning() << "Unable to get config widget type from gphoto";
    return QVariant();
  }

  if (type == GP_WIDGET_RADIO) {
    char *value;
    ret = gp_widget_get_value(option, &value);
    if (ret < GP_OK) {
      qWarning() << "Unable to get value for option" << qPrintable(name)
                 << "from gphoto";
      return QVariant();
    } else {
      return QString::fromLocal8Bit(value);
    }
  } else if (type == GP_WIDGET_TOGGLE) {
    int value;
    ret = gp_widget_get_value(option, &value);
    if (ret < GP_OK) {
      qWarning() << "Unable to get value for option" << qPrintable(name)
                 << "from gphoto";
      return QVariant();
    } else {
      return value == 0 ? false : true;
    }
  } else {
    qWarning() << "Options of type" << type << "are currently not supported";
  }

  return QVariant();
}

bool GPhotoCameraWorker::setParameter(const QString &name,
                                      const QVariant &value) {
  CameraWidget *root;
  int ret = gp_camera_get_config(mCamera.get(), &root, mContext.get());
  if (ret < GP_OK) {
    qWarning() << "Unable to get root option from gphoto";
    return false;
  }

  // Get widget pointer
  CameraWidget *option;
  ret = gp_widget_get_child_by_name(root, qPrintable(name), &option);
  if (ret < GP_OK) {
    qWarning() << "Unable to get option" << qPrintable(name) << "from gphoto";
    return false;
  }

  // Get option type
  CameraWidgetType type;
  ret = gp_widget_get_type(option, &type);
  if (ret < GP_OK) {
    qWarning() << "Unable to get option type from gphoto";
    gp_widget_free(option);
    return false;
  }

  if (type == GP_WIDGET_RADIO) {
    if (value.type() == QVariant::String) {
      // String, need no conversion
      ret = gp_widget_set_value(option, qPrintable(value.toString()));

      if (ret < GP_OK) {
        qWarning() << "Failed to set value" << value << "to" << name
                   << "option:" << ret;
        return false;
      }

      ret = gp_camera_set_config(mCamera.get(), root, mContext.get());

      if (ret < GP_OK) {
        qWarning() << "Failed to set config to camera";
        return false;
      }

      waitForOperationCompleted();
      return true;
    } else if (value.type() == QVariant::Double) {
      // Trying to find nearest possible value (with the distance of 0.1) and
      // set it to property
      double v = value.toDouble();

      int count = gp_widget_count_choices(option);
      for (int i = 0; i < count; ++i) {
        const char *choice;
        gp_widget_get_choice(option, i, &choice);

        // We use a workaround for flawed russian i18n of gphoto2 strings
        bool ok;
        double choiceValue =
            QString::fromLocal8Bit(choice).replace(',', '.').toDouble(&ok);
        if (!ok) {
          qDebug() << "Failed to convert value" << choice << "to double";
          continue;
        }

        if (qAbs(choiceValue - v) < 0.1) {
          ret = gp_widget_set_value(option, choice);
          if (ret < GP_OK) {
            qWarning() << "Failed to set value" << choice << "to" << name
                       << "option:" << ret;
            return false;
          }

          ret = gp_camera_set_config(mCamera.get(), root, mContext.get());
          if (ret < GP_OK) {
            qWarning() << "Failed to set config to camera";
            return false;
          }

          waitForOperationCompleted();
          return true;
        }
      }

      qWarning() << "Can't find value matching to" << v << "for option" << name;
      return false;
    } else if (value.type() == QVariant::Int) {
      // Little hacks for 'ISO' option: if the value is -1, we pick the first
      // non-integer value we found and set it as a parameter
      int v = value.toInt();

      int count = gp_widget_count_choices(option);
      for (int i = 0; i < count; ++i) {
        const char *choice;
        gp_widget_get_choice(option, i, &choice);

        bool ok;
        int choiceValue = QString::fromLocal8Bit(choice).toInt(&ok);

        if ((ok && choiceValue == v) || (!ok && v == -1)) {
          ret = gp_widget_set_value(option, choice);
          if (ret < GP_OK) {
            qWarning() << "Failed to set value" << choice << "to" << name
                       << "option:" << ret;
            return false;
          }

          ret = gp_camera_set_config(mCamera.get(), root, mContext.get());
          if (ret < GP_OK) {
            qWarning() << "Failed to set config to camera";
            return false;
          }

          waitForOperationCompleted();
          return true;
        }
      }

      qWarning() << "Can't find value matching to" << v << "for option" << name;
      return false;
    } else {
      qWarning() << "Failed to set value" << value << "to" << name
                 << "option. Type" << value.type() << "is not supported";
      gp_widget_free(option);
      return false;
    }
  } else if (type == GP_WIDGET_TOGGLE) {
    int v = 0;
    if (value.canConvert<int>()) {
      v = value.toInt();
    } else {
      qWarning() << "Failed to set value" << value << "to" << name
                 << "option. Type" << value.type() << "is not supported";
      gp_widget_free(option);
      return false;
    }

    ret = gp_widget_set_value(option, &v);
    if (ret < GP_OK) {
      qWarning() << "Failed to set value" << v << "to" << name
                 << "option:" << ret;
      return false;
    }

    ret = gp_camera_set_config(mCamera.get(), root, mContext.get());
    if (ret < GP_OK) {
      qWarning() << "Failed to set config to camera";
      return false;
    }

    waitForOperationCompleted();
    return true;
  } else {
    qWarning() << "Options of type" << type << "are currently not supported";
  }

  gp_widget_free(option);
  return false;
}
