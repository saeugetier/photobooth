#include "photomontage.h"
#include <list>
#include <functional>
#include <Magick++.h>
#include <QtConcurrent/QtConcurrent>

PhotoMontage::PhotoMontage(QObject *parent) : QObject(parent), m_future(this)
{
    connect(&m_future, &QFutureWatcher<QString>::finished, this, &PhotoMontage::processed);
}


QQmlListProperty<QString> PhotoMontage::filenames()
{
    return QQmlListProperty<QString>(this, 0, &PhotoMontage::append_filename, &PhotoMontage::count_filenames, 0, &PhotoMontage::clear_filenames);
}

bool PhotoMontage::generate(QString outputFilename)
{
    if(m_filenames.count() != 4)
        return false;

    const QStringList filenames = m_filenames;




    std::function<QString(const QString&)> process = [filenames](const QString &imageFileName) {

        std::list<Magick::Image> sourceImageList;
        Magick::Image image;

        Magick::Montage montageSettings;

        montageSettings.backgroundColor(Magick::Color("#FFFFFF"));
        montageSettings.tile("2x2");
        montageSettings.geometry(Magick::Geometry("792x1224>"));
        montageSettings.shadow(true);

        for(auto iter = filenames.begin(); iter != filenames.end(); iter++)
        {
            image.read((*iter).toStdString());
            sourceImageList.push_back(image);
        }

        std::list<Magick::Image> montagelist;
        Magick::montageImages( &montagelist, sourceImageList.begin(), sourceImageList.end(), montageSettings);

        Magick::writeImages(montagelist.begin(), montagelist.end(), imageFileName.toStdString());

            return imageFileName;
        };

    QStringList files;
    files.append(outputFilename);
    m_future.setFuture(QtConcurrent::mapped(files, process));

    return true;
}

void PhotoMontage::append_filename(QQmlListProperty<QString> *list, QString* filename)
{
    PhotoMontage *montage = qobject_cast<PhotoMontage *>(list->object);
    if (montage) {
        montage->m_filenames.append(*filename);
        if(montage->m_filenames.count() > 4)  //do not use more than 4 images...
            montage->m_filenames.pop_front();
    }
}


void PhotoMontage::clear_filenames(QQmlListProperty<QString> *list)
{
    PhotoMontage *montage = qobject_cast<PhotoMontage *>(list->object);
    if (montage) {
        montage->m_filenames.clear();
    }
}

int PhotoMontage::count_filenames(QQmlListProperty<QString> *list)
{
    PhotoMontage *montage = qobject_cast<PhotoMontage *>(list->object);
    if (montage) {
        return montage->m_filenames.count();
    }

    return 0;
}

void PhotoMontage::processed()
{
    emit montageReady(m_future.result());
}
