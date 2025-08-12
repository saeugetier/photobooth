#include "filesystem.h"
#include <QImageReader>
#include <QRegularExpression>

FileSystem::FileSystem(QObject *parent) : QObject(parent)
{

}

QUrl FileSystem::findFile(QString filename, QList<QUrl> searchPaths, bool searchInResource)
{
    QUrl file; //instance of a "unknown" url

    if(filename.length() == 0) //will return empty url, if no filename is given
    {
        qDebug() << "Filesystem Error: filename is empty!";
        return file;
    }

    //test if filename is a file path
    if(QUrl(filename).isValid())
    {
        if(QUrl(filename).isLocalFile())
        {
            file = QUrl(filename); // filename is a real file --> convert to URL
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
    if(file.isEmpty() && searchInResource)
    {
        QDirIterator it(":", QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString filepath = it.next();
            QFileInfo info(filepath);
            if(info.fileName() == filename)
            {
                file = QUrl("qrc" + it.filePath()); //url has structure "qrc:/some_dir/some_file"
                break;
            }
        }
    }

    if(file.isEmpty())
    {
        qDebug() << "Filesystem Error: Could not find file " << filename;
    }

    return file;
}

QString FileSystem::getImagePath()
{
    QSettings settings("saeugetier", "qtbooth");
    settings.sync();
    if(settings.contains("Application/foldername"))
        return settings.value("Application/foldername").value<QString>();
    else
        return "file://" + QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
}

bool FileSystem::createFolder(const QString &path)
{
    if(path.length() == 0)
    {
        qDebug() << "Filesystem Error: path is empty!";
        return false;
    }

    QDir dir(path);
    if(!dir.exists())
    {
        if(dir.mkpath("."))
        {
            qDebug() << "Created folder: " << path;
            return true;
        }
        else
        {
            qDebug() << "Filesystem Error: Could not create folder: " << path;
            return false;
        }
    }
    else
    {
        qDebug() << "Folder already exists: " << path;
        return true;
    }
}

void FileSystem::checkImageFolders()
{
    QString imagePath = getImagePath();
    imagePath = imagePath.right(imagePath.length() - QString("file://").length());
    if(!QDir(imagePath).exists())
    {
        QDir().mkdir(imagePath);
    }
    QString collagePath = imagePath + "/collage";
    if(!QDir(collagePath).exists())
    {
        QDir().mkdir(collagePath);
    }
    QString rawPath = imagePath + "/raw";
    if(!QDir(rawPath).exists())
    {
        QDir().mkdir(rawPath);
    }
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
    unmountProcess.start();
    unmountProcess.waitForFinished();
}

void FileSystem::startCopyFilesToPath(const QString &path)
{
    QString imagePath = this->getImagePath();
    imagePath = imagePath.right(imagePath.length() - QString("file://").length());

    QString copyPath;
    if (path.startsWith("file://", Qt::CaseInsensitive))
    {
        QUrl url(path);
        copyPath = QUrl(path).toLocalFile();
    }
    else
    {
        copyPath = path;
    }

    if(copyPath.length())
    {
        QDir imageDir(imagePath);
        QStringList filters;
        filters << "*.jpg" << "*.JPG";
        imageDir.setFilter(QDir::Files);
        imageDir.setNameFilters(filters);
        if(!imageDir.isEmpty() && imageDir.exists())
        {
            qDebug() << "Image folder not empty and found removable disc. Copying...";
            m_copyFuture = QtConcurrent::run([copyPath,imagePath, this]() {
                QDir imageDir(imagePath);
                QStringList filters;
                filters << "*.jpg" << "*.JPG";
                QStringList files = imageDir.entryList(filters, QDir::Files);
                filters << "*.png";
                QDir collageDir(imagePath + "/collage");
                files.append(collageDir.entryList(filters, QDir::Files));

                int i;
                for(i = 0; i < files.count() && !this->m_copyFuture.isCanceled(); i++)
                {
                    int progress = (100 * i + 1) / files.count();
                    qDebug() << "Copy file: " << imagePath + "/" + files[i] << " to: " << copyPath + "/" + files[i] << " Progress: " << progress;
                    if(QFile::exists(copyPath + "/" + files[i]))
                        QFile::remove(copyPath + "/" + files[i]);

                    if(!QFile::copy(imagePath + "/" + files[i], copyPath + "/" + files[i]))
                    {
                        if(!QFile::copy(imagePath + "/collage/" + files[i], copyPath + "/" + files[i]))
                        {
                            qDebug() << "Copying file: " << files[i] << " was not successfull";
                        }
                    }

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
    static QRegularExpression regexDrive("\\/dev\\/sd*");
    static QRegularExpression regexBoot("^/(boot|home|tmp)?$");
    QList<QStorageInfo> drives = QStorageInfo::mountedVolumes();
    for(int i = 0; i < drives.count(); i++)
    {
        if(!drives[i].isRoot() && !drives[i].isReadOnly())
        {
            if(regexDrive.match(drives[i].device()).hasMatch())
            {
                if(!regexBoot.match(drives[i].rootPath()).hasMatch())
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
    filters << "*.jpg" << "*.JPG" << "*.png" << "*.PNG";
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
    imageDir.setFilter(QDir::Files);
    imageDir.setNameFilters(filters);
    if(!imageDir.isEmpty() && imageDir.exists())
    {
        QStringList files = imageDir.entryList(filters, QDir::Files);

        int i;
        for(i = 0; i < files.count(); i++)
        {
            qDebug() << "removing file: " << files[i];
            QFile::remove(imagePath + "/collage/" + files[i]);
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

bool FileSystem::layoutFilesOnRemovableDrive()
{
    bool result = false;
    if(removableDriveMounted())
    {
        QString path = getRemovableDrivePath();
        while(path.endsWith( '/' )) path.chop(1);
        if(QFile::exists(path + "/layout/" + "Collages.xml"))
        {
            result = true;
        }
    }
    return result;
}

void FileSystem::copyLayoutFiles()
{
    QStringList fileNameFilter;
    fileNameFilter << "*.xml" << "*.png" << "*.PNG" << "*.jpg" << "*.JPG" << "*.svg" << "*.SVG";

    QString path = getRemovableDrivePath();
    while(path.endsWith( '/' )) path.chop(1);
    QDir layoutDirectory(path + "/layout/");
    layoutDirectory.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    layoutDirectory.setNameFilters(fileNameFilter);
    QStringList files = layoutDirectory.entryList();

    QString destiny = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir destinyDirectory = QDir(destiny);
    destinyDirectory.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    destinyDirectory.setNameFilters(fileNameFilter);
    QStringList oldFiles = destinyDirectory.entryList();

    //first remove the files
    for(const QString& file : oldFiles)
    {
        QFile::remove(destiny + "/" + file);
    }

    //then copy new files
    for(const QString& file : files)
    {
        QFile::copy(path + "/layout/" + file, destiny + "/" + file);
    }
}

QSize FileSystem::getImageSize(QString filename)
{
    if(filename.contains("file:"))
    {
        filename = filename.right(filename.length() - QString("file:").length());
    }
    QImageReader reader(filename);
    return reader.size();
}
