#include "myhelper.h"
#include <QFile>
#include <QtPrintSupport/QPrinter>

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

}
