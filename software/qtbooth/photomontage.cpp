#include "photomontage.h"
#include <list>
#include <functional>
#include <Magick++.h>
#include <QtConcurrent/QtConcurrent>
#include <QDebug>

PhotoMontage::PhotoMontage(QObject *parent) : QObject(parent), m_future(this)
{
    connect(&m_future, &QFutureWatcher<QString>::finished, this, &PhotoMontage::processed);
}


QStringList PhotoMontage::filenames()
{
    return m_filenames;
}

int PhotoMontage::addFile(QString file)
{
   m_filenames.append(file);
   return m_filenames.count();
}

void PhotoMontage::clearFiles()
{
    m_filenames.clear();
}

bool PhotoMontage::generate(QString outputFilename)
{
    qDebug() << "Generating collage with filename: " << outputFilename;
    if(m_filenames.count() != 4)
        return false;

    const QStringList filenames = m_filenames;

    std::function<QString(const QString&)> process = [filenames](const QString &imageFileName) {

        std::list<Magick::Image> sourceImageList;
        Magick::Image image;

        Magick::MontageFramed montageSettings;

        montageSettings.backgroundColor(Magick::Color("#FFFFFF"));
        montageSettings.tile("2x2");
        //montageSettings.geometry(Magick::Geometry("792x1224>"));
        montageSettings.geometry(Magick::Geometry("1224x792>"));
        montageSettings.shadow(true);
        montageSettings.borderWidth(20);

        for(auto iter = filenames.begin(); iter != filenames.end(); iter++)
        {
            if(iter->contains("file://"))
            {
                qDebug() << "Loading file: " << iter->right(iter->length() - QString("file://").length());
                image.read(iter->right(iter->length() - QString("file://").length()).toStdString());
            }
            else
            {
                qDebug() << "Loading file: " << *iter;
                image.read(iter->toStdString());
            }
            image.resize("25x25%");
            sourceImageList.push_back(image);
        }

        std::list<Magick::Image> montagelist;
        Magick::montageImages( &montagelist, sourceImageList.begin(), sourceImageList.end(), montageSettings);

        if(imageFileName.contains("file://"))
        {
            Magick::writeImages(montagelist.begin(), montagelist.end(), (imageFileName.right(imageFileName.length() - QString("file://").length())).toStdString());
        }
        else
        {
            Magick::writeImages(montagelist.begin(), montagelist.end(), imageFileName.toStdString());
        }
        return imageFileName;
    };

    QStringList files;
    files.append(outputFilename);
    m_future.setFuture(QtConcurrent::mapped(files, process));

    return true;
}

void PhotoMontage::processed()
{
    emit montageReady(m_future.result());
}
