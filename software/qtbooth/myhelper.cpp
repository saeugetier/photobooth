#include "myhelper.h"
#include "singleton.h"
#include <QFile>
#include <QImage>
#include <QStandardPaths>
#include <QSettings>
#include <QDebug>
#include <QProcess>
#include <QGuiApplication>
#include <QStorageInfo>

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

QTranslator* MyHelper::getTranslator()
{
    return mTranslator.get();
}

void MyHelper::setLanguage(QString code)
{
    if(mTranslator.get() != NULL)
        QGuiApplication::removeTranslator(mTranslator.get());

    std::unique_ptr<QTranslator> translator(new QTranslator);
    mTranslator.swap(translator);
    mTranslator->load("tr_" + code, ":/qml/");
    QGuiApplication::installTranslator(mTranslator.get());

    emit languageChanged();
}

bool MyHelper::removableDriveMounted()
{
    QList<QStorageInfo> drives = QStorageInfo::mountedVolumes();
    for(int i = 0; i < drives.count(); i++)
    {
        if(!drives[i].isRoot() && !drives[i].isReadOnly())
        {
            return true;
        }
    }
    return false;
}

void MyHelper::unmountRemoveableDrive()
{
    QProcess unmountProcess;
    unmountProcess.setProgram("umount");
    unmountProcess.setArguments(QStringList() << this->getRemovableDrivePath());
}

void MyHelper::startCopyFilesToRemovableDrive()
{
    QString imagePath = this->getImagePath();
    QString removableDrivePath = this->getRemovableDrivePath();

    if(removableDrivePath.length())
    {
        QDir imageDir(imagePath);
        if(!imageDir.isEmpty() && !imageDir.exists())
        {
            //copy files
        }
        else
            emit copyProgress(-1);
    }
    else
        emit copyProgress(-1);
}

QString MyHelper::getRemovableDrivePath()
{
    QList<QStorageInfo> drives = QStorageInfo::mountedVolumes();
    for(int i = 0; i < drives.count(); i++)
    {
        if(!drives[i].isRoot() && !drives[i].isReadOnly())
        {
            return drives[i].name();
        }
    }
    return QString();
}
