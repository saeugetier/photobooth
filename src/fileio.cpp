#include "fileio.h"
#include <QFile>
#include <QTextStream>

QByteArray FileIO::read()
{
    QString scheme = mSource.scheme();
    QFile file;

    if(scheme == "qrc")
    {
        file.setFileName(mSource.toString().remove(0,3)); //remove qrc
    }
    else
    {
        file.setFileName(mSource.toLocalFile());
    }

    if (!file.open(QIODevice::ReadOnly))
        return QByteArray();

    return file.readAll();
}

bool FileIO::write(const QString& data)
{
    if (mSource.isEmpty())
        return false;

    QFile file(mSource.toLocalFile());
    if (!file.open(QFile::WriteOnly | QFile::Truncate))
        return false;

    QTextStream out(&file);
    out << data;

    file.close();

    return true;
}

bool FileIO::remove()
{
    if (mSource.isEmpty())
        return false;

    QFile file(mSource.toLocalFile());
    if(!file.exists())
        return false;

    return file.remove();
}

bool FileIO::move(const QUrl &filename)
{
    if (mSource.isEmpty())
        return false;

    QFile file(mSource.toLocalFile());
    if(!file.exists())
        return false;

    if(!file.copy(filename.toLocalFile()))
        return false;

    return file.remove();
}
