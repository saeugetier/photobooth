#include "standardprinter.h"
#include <QImage>
#include <QDebug>
#include <QPainter>

bool StandardPrinter::printerOnline()
{
    return true; //cannot determine online state @TODO
}

bool StandardPrinter::busy()
{
    QPrinter::PrinterState state = mPrinter.printerState();

    if(state == QPrinter::PrinterState::Active)
    {
        return true;
    }
    else
    {
        return false;
    }
}

int StandardPrinter::printImage(const QString &filename, int copyCount)
{
    mPrinter.setColorMode(QPrinter::Color);
    mPrinter.setFullPage(true);
    mPrinter.setOrientation(QPrinter::Landscape);
    mPrinter.setCopyCount(copyCount);
    QImage img;
    if(img.load(filename))
    {
        QPainter painter(&mPrinter);
        QRect rect = painter.viewport();
        qDebug() << "Rect size: " << rect;
        QSize size = img.size();
        qDebug() << "Image size: " << size;
        size.scale(rect.size(), Qt::KeepAspectRatio);
        qDebug() << "Scaled Image size: " << size;
        painter.setViewport((rect.width() - size.width()), (rect.height() - size.height()), size.width(), size.height());
        painter.drawImage(QRect(QPoint(0,0), size),img);
        painter.end();
    }
    else
    {
        qDebug() << "Could not load image file " << filename;
    }
}

StandardPrinter::StandardPrinter(QPrinterInfo& info, QObject *parent) : AbstractPrinter(parent), mPrinter(info, QPrinter::HighResolution)
{
}

QStringList StandardPrinter::getAvailablePrintersInternal()
{
    return QPrinterInfo::availablePrinterNames();
}

StandardPrinter *StandardPrinter::createInternal(const QString &name)
{
    StandardPrinter *ptr = nullptr;
    auto printers = QPrinterInfo::availablePrinters();
    for(auto printer: printers)
    {
        if(printer.printerName() == name)
        {
            ptr = new StandardPrinter(printer);
            break;
        }
    }
    return ptr;
}
