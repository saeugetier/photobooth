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
    QSettings settings("Timmedia", "QML Photo Booth");
    if(settings.contains("Application/foldername"))
        return settings.value("Application/foldername").value<QString>();
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
    return m_Translator.get();
}

void MyHelper::setLanguage(QString code)
{
    if(m_Translator.get() != NULL)
        QGuiApplication::removeTranslator(m_Translator.get());

    std::unique_ptr<QTranslator> translator(new QTranslator);
    m_Translator.swap(translator);
    m_Translator->load("tr_" + code, ":/qml/");
    QGuiApplication::installTranslator(m_Translator.get());

    emit languageChanged();
}

bool MyHelper::removableDriveMounted()
{
    if(getRemovableDrivePath().length() > 0)
    {
        return true;
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
    imagePath = imagePath.right(imagePath.length() - QString("file://").length());
    QString removableDrivePath = this->getRemovableDrivePath();
    qDebug() << "Try to copy all images to removable drive";

    if(removableDrivePath.length())
    {
        QDir imageDir(imagePath);
        QStringList filters;
        filters << "*.jpg" << "*.JPG";
        imageDir.setFilter(QDir::Files);
        imageDir.setNameFilters(filters);
        if(!imageDir.isEmpty() && imageDir.exists())
        {
            qDebug() << "Image folder not empty and found removable disc. Copying...";
            m_copyFuture = QtConcurrent::run([removableDrivePath,imagePath, this]() {
                QDir imageDir(imagePath);
                QStringList filters;
                filters << "*.jpg" << "*.JPG";
                QStringList files = imageDir.entryList(filters, QDir::Files);

                int i;
                for(i = 0; i < files.count() && !this->m_copyFuture.isCanceled(); i++)
                {
                    int progress = (100 * i + 1) / files.count();
                    qDebug() << "File: " << files[i] << " Progress: " << progress;
                    if(QFile::exists(removableDrivePath + "/" + files[i]))
                            QFile::remove(removableDrivePath + "/" + files[i]);

                    QFile::copy(imagePath + "/" + files[i], removableDrivePath + "/" + files[i]);

                    emit this->copyProgress(progress);
                }

                QProcess progress;
                progress.setProgram("sync");
                progress.start();
                progress.waitForFinished();

                if(!this->m_copyFuture.isCanceled())
                {
                    emit this->copyFinished();
                }
            });
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
    QRegExp regexDrive("\\/dev\\/sd*");
    QRegExp regexBoot("\\/boot");
    QList<QStorageInfo> drives = QStorageInfo::mountedVolumes();
    for(int i = 0; i < drives.count(); i++)
    {
        if(!drives[i].isRoot() && !drives[i].isReadOnly())
        {
            if(-1 != regexDrive.indexIn(drives[i].device()))
            {
                if(-1 == regexBoot.indexIn(drives[i].rootPath()))
                {
                    return drives[i].rootPath();
                }
            }
        }
    }
    return QString();
}

void MyHelper::abortCopy()
{
    if(m_copyFuture.isRunning())
    {
        m_copyFuture.cancel();
    }

}

void MyHelper::deleteAllImages()
{
    QString imagePath = this->getImagePath();
    imagePath = imagePath.right(imagePath.length() - QString("file://").length());
    QDir imageDir(imagePath);
    QStringList filters;
    filters << "*.jpg" << "*.JPG";
    imageDir.setFilter(QDir::Files);
    imageDir.setNameFilters(filters);
    if(!imageDir.isEmpty() && imageDir.exists())
    {
        QStringList files = imageDir.entryList(filters, QDir::Files);

        int i;
        for(i = 0; i < files.count(); i++)
        {
            qDebug() << "removing file: " << files[i];
            QFile::remove(imagePath + "/" + files[i]);
        }
    }
}
