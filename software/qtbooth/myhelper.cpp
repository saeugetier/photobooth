#include "myhelper.h"
#include <QFile>
#include <cups/cups.h>

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
    cups_dest_t *defaultP = cupsGetDest (NULL, NULL,0,NULL);
    cupsPrintFile(defaultP->name,filename.toStdString().c_str(), "Image", defaultP->num_options, defaultP->options);
}
