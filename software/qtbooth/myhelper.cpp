#include "myhelper.h"
#include "singleton.h"
#include <QFile>
#include <QImage>
#include <QStandardPaths>
#include <QSettings>
#include <QDebug>
#include <QProcess>


MyHelper::MyHelper(QObject *parent) : QObject(parent)
{
}

MyHelper* MyHelper::createInstance()
{
    return new MyHelper();
}


MyHelper* MyHelper::instance()
{
    return Singleton<MyHelper>::instance(MyHelper::createInstance);
}

void MyHelper::removeFile(const QString &filename) {
    QFile file(filename);
    file.remove();
    qDebug("File deleted: %s", filename.toStdString().c_str());
}


QString MyHelper::getImagePath()
{
    QSettings settings;
    if(settings.contains("Application.foldername"))
        return settings.value("Application.foldername").value<QString>();
    else
        return "file://" + QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
}

void MyHelper::shutdown()
{
    QProcess process;
    process.startDetached("shutdown now -P -h");
}

void MyHelper::restart()
{
    QProcess process;
    process.startDetached("reboot");
}

