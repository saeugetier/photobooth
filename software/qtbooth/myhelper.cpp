#include "myhelper.h"
#include <QFile>
#include <QtPrintSupport/QPrinter>
#include <Magick++.h>
#include <QImage>
#include <QPainter>
#include <QStandardPaths>
#include <QSettings>

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
    QPrinter printer;
    QImage img(filename);
    QPainter painter(&printer);
    painter.drawImage(QPoint(0,0),img);
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

