#include "filesystem.h"

FileSystem::FileSystem(QObject *parent) : QObject(parent)
{

}

QUrl FileSystem::findFile(QString filename, QList<QUrl> searchPaths, bool searchInResource)
{
    QUrl file;

    if(filename.length() == 0)
        return file;

    //test if filename is a file path
    if(QUrl(filename).isValid())
    {
        if(QUrl(filename).isLocalFile())
        {
            file = QUrl(filename);
        }
    }

    //filename is not a path. search for local file in search paths.
    for(auto iter = searchPaths.begin(); iter != searchPaths.end() && file.isEmpty(); iter++)
    {
        if(iter->isLocalFile())
        {
            QFileInfo info(iter->path());
            if(info.isDir())
            {
                QDir dir(iter->path());
                if(dir.exists(filename))
                {
                    QString filepath = dir.filePath(filename);
                    file = QUrl::fromLocalFile(filepath);
                }
            }
        }
    }

    //if no file is found, search for file in QRC
    if(file.isEmpty())
    {
        QDirIterator it(":", QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString filepath = it.next();
            QFileInfo info(filepath);
            if(info.fileName() == filename)
            {
                file = QUrl("qrc" + it.filePath());
                break;
            }
        }
    }

    return file;
}

QString FileSystem::getImagePath()
{
    QSettings settings("Timmedia", "QML Photo Booth");
    if(settings.contains("Application/foldername"))
        return settings.value("Application/foldername").value<QString>();
    else
        return "file://" + QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
}

bool FileSystem::removableDriveMounted()
{
    if(getRemovableDrivePath().length() > 0)
    {
        return true;
    }
    return false;
}

void FileSystem::unmountRemoveableDrive()
{
    QProcess unmountProcess;
    unmountProcess.setProgram("umount");
    unmountProcess.setArguments(QStringList() << this->getRemovableDrivePath());
}

void FileSystem::startCopyFilesToRemovableDrive()
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
                    qDebug() << "Copy file: " << imagePath + "/" + files[i] << " to: " << removableDrivePath + "/" + files[i] << " Progress: " << progress;
                    if(QFile::exists(removableDrivePath + "/" + files[i]))
                            QFile::remove(removableDrivePath + "/" + files[i]);

                    if(!QFile::copy(imagePath + "/" + files[i], removableDrivePath + "/" + files[i]))
                        qDebug() << "Copying file: " << files[i] << " was not successfull";

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

QString FileSystem::getRemovableDrivePath()
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

void FileSystem::abortCopy()
{
    if(m_copyFuture.isRunning())
    {
        m_copyFuture.cancel();
    }

}

void FileSystem::deleteAllImages()
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

    imageDir.setPath(imagePath + "/collage");
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


    imageDir.setPath(imagePath + "/recycle");
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
