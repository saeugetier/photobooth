#include "photomontage.h"
#include <list>
#include <functional>
#include <Magick++.h>
#include <QtConcurrent/QtConcurrent>
#include <QtConcurrent/QtConcurrentMap>
#include <QtConcurrent/QtConcurrentFilter>
#include <QDebug>
#include <QPainter>
#include <QImage>
#include <QPair>

//https://stackoverflow.com/questions/6477958/qt-is-there-any-class-for-combine-few-images-into-one
//https://stackoverflow.com/questions/22664217/making-my-own-photo-mosaic-app-with-qt-using-c

PhotoMontage::PhotoMontage(QObject *parent) : QObject(parent), m_future(this), mMontageImage(NULL)
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
    qDebug() << "Input files used: " << m_filenames;

    QList<QPair<int, QString>> file_list;

    for(int i= 0; i < m_filenames.count(); i++ )
    {
        file_list.append(QPair<int, QString>(i, m_filenames[i]));
    }

    if(mMontageImage)
        delete mMontageImage;

    mMontageImage = new QImage(QSize(1000,1000), QImage::Format_RGB888);

    auto load = [] (const QPair<int, QString> &file) -> QPair<int, QImage> { return QPair<int, QImage>(); };

    auto mergeCollage = [] (QImage& collage, QPair<int, QImage> image) -> void
    {
        QPainter painter(&collage);

        painter.setPen(QPen(Qt::green));
        painter.setFont(QFont("Times", 10, QFont::Bold));
        painter.drawLine(collage.rect().bottomLeft().x(), collage.rect().bottomLeft().y()-10,
                   collage.rect().bottomRight().x(), collage.rect().bottomRight().y()-10);  //works!

        painter.drawText(collage.rect(), Qt::AlignCenter, "Help");

        qDebug() << "Processing: " << image.first;
    };

    QFuture<QImage> future = QtConcurrent::mappedReduced(file_list, load, mergeCollage);

    m_future.setFuture(future);
}

/*bool PhotoMontage::generate(QString outputFilename)
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
}*/

void PhotoMontage::processed()
{
    emit montageReady("");
}
