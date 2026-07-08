#include "standardprinter.h"
#include <QImage>
#include <QDebug>
#include <QPainter>
#include <QElapsedTimer>
#include <QPointer>

namespace {

QString printerStateName(QPrinter::PrinterState state)
{
    switch(state)
    {
    case QPrinter::Idle:
        return QStringLiteral("Idle");
    case QPrinter::Active:
        return QStringLiteral("Active");
    case QPrinter::Aborted:
        return QStringLiteral("Aborted");
    case QPrinter::Error:
        return QStringLiteral("Error");
    }

    return QStringLiteral("Unknown");
}

QPrinterInfo printerInfoByName(const QString &printerName)
{
    const auto printers = QPrinterInfo::availablePrinters();
    for(const auto &printerInfo : printers)
    {
        if(printerInfo.printerName() == printerName)
        {
            return printerInfo;
        }
    }
    return QPrinterInfo();
}

int printImageInThread(const QString &printerName, const QString &filename, int copyCount)
{
    if(copyCount <= 0)
    {
        qDebug() << "Invalid copyCount:" << copyCount;
        return -1;
    }

    QImage img;
    if(!img.load(filename))
    {
        qDebug() << "Could not load image file" << filename;
        return -1;
    }

    QPrinterInfo printerInfo = printerInfoByName(printerName);
    if(printerInfo.isNull())
    {
        qDebug() << "Printer not available:" << printerName;
        return -1;
    }

    QPrinter printer(printerInfo, QPrinter::HighResolution);
    printer.setColorMode(QPrinter::Color);
    printer.setFullPage(true);
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setCopyCount(copyCount);

    QPainter painter(&printer);
    if(!painter.isActive())
    {
        qDebug() << "Could not start painter for printer";
        return -1;
    }

    const QRect rect = painter.viewport();
    qDebug() << "Rect size:" << rect;
    QSize size = img.size();
    qDebug() << "Image size:" << size;
    size.scale(rect.size(), Qt::KeepAspectRatio);
    qDebug() << "Scaled Image size:" << size;
    painter.setViewport((rect.width() - size.width()), (rect.height() - size.height()), size.width(), size.height());
    painter.drawImage(QRect(QPoint(0, 0), size), img);
    if(!painter.end())
    {
        qDebug() << "Could not finalize print painting";
        return -1;
    }

    qDebug() << "Print job sent to printer:" << printerName << "with copy count:" << copyCount;

    // Some backends report Idle immediately after painter.end() and only switch
    // to Active once the spool job is created. Wait for that transition first.
    QElapsedTimer startTimer;
    startTimer.start();
    QPrinter::PrinterState state = printer.printerState();
    while(state == QPrinter::Idle && startTimer.elapsed() < StandardPrinter::sPrintIdleTimeoutMs)
    {
        if(QThread::currentThread()->isInterruptionRequested())
        {
            qDebug() << "Print thread interruption requested";
            return -1;
        }

        QThread::msleep(StandardPrinter::sPrintPollIntervalMs);
        state = printer.printerState();
    }

    qDebug() << "Printer state after job submission:" << printerStateName(state);

    if(state == QPrinter::PrinterState::Active)
    {
        QElapsedTimer activeTimer;
        activeTimer.start();
        while(state == QPrinter::PrinterState::Active && activeTimer.elapsed() < StandardPrinter::sActiveJobTimeoutMs)
        {
            if(QThread::currentThread()->isInterruptionRequested())
            {
                qDebug() << "Print thread interruption requested";
                return -1;
            }

            QThread::msleep(StandardPrinter::sPrintPollIntervalMs);
            state = printer.printerState();
        }
    }

    qDebug() << "Final printer state:" << printerStateName(state);
    qDebug() << "Print job completed for printer:" << printerName << "with copy count:" << copyCount;

    return 0;
}

}

StandardPrinter::~StandardPrinter()
{
    if(mPrintThread != nullptr)
    {
        mPrintThread->requestInterruption();
        mPrintThread->wait();
        delete mPrintThread;
        mPrintThread = nullptr;
    }
}

bool StandardPrinter::printerOnline()
{
    return true; //cannot determine online state @TODO
}

bool StandardPrinter::busy()
{
    return mBusy;
}

int StandardPrinter::printImage(const QString &filename, int copyCount)
{
    if(mBusy || mPrintThread != nullptr)
    {
        qDebug() << "StandardPrinter is already printing";
        return -1;
    }

    mBusy = true;
    emit busyChanged(true);

    QPointer<StandardPrinter> self(this);
    const QString printerName = mPrinterName;
    mPrintThread = QThread::create([self, printerName, filename, copyCount]() {
        const int result = printImageInThread(printerName, filename, copyCount);

        if(self)
        {
            QMetaObject::invokeMethod(self, [self, result]() {
                if(self)
                {
                    self->finishPrint(result);
                }
            }, Qt::QueuedConnection);
        }
    });

    QObject::connect(mPrintThread, &QThread::finished, this, [this]() {
        if(mPrintThread != nullptr)
        {
            mPrintThread->deleteLater();
            mPrintThread = nullptr;
        }
    });

    mPrintThread->start();
    return 0;
}

void StandardPrinter::finishPrint(int result)
{
    mBusy = false;
    emit busyChanged(false);

    if(result == 0)
    {
        emit success();
    }
    else
    {
        emit failed();
    }
}

StandardPrinter::StandardPrinter(QPrinterInfo& info, QObject *parent)
    : AbstractPrinter(parent), mPrinter(info, QPrinter::HighResolution), mPrinterName(info.printerName())
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
