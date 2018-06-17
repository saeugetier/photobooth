#include "myhelper.h"
#include <QFile>
#include <QtPrintSupport/QPrinter>
#include <QImage>
#include <QPainter>
#include <QStandardPaths>
#include <QSettings>
#include <QDebug>

MyHelper::MyHelper(QObject *parent) : QObject(parent)
{
}

void MyHelper::removeFile(const QString &filename) {
    QFile file(filename);
    file.remove();
    qDebug("File deleted: %s", filename.toStdString().c_str());
}

void MyHelper::printImage(const QString &filename)
{
    qDebug() << "File printed: " << filename;
    QPrinter printer(QPrinter::HighResolution);
    printer.setColorMode(QPrinter::Color);
    printer.setFullPage(true);
    printer.setOrientation(QPrinter::Landscape);
    QImage img(filename.right(filename.length() - QString("file://").length()));
    QPainter painter(&printer);
    QRect rect = painter.viewport();
    qDebug() << "Rect size: " << rect;
    QSize size = img.size();
    qDebug() << "Image size: " << size;
    size.scale(rect.size(), Qt::KeepAspectRatio);
    qDebug() << "Scaled Image size: " << size;
    painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
    painter.drawImage(QRect(QPoint(0,0), size),img);
    painter.end();
}

QString MyHelper::getImagePath()
{
    QSettings settings;
    if(settings.contains("Application.foldername"))
        return settings.value("Application.foldername").value<QString>();
    else
        return "file://" + QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
}
