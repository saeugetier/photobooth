#include "fakeprinter.h"
#include <QDebug>

QSize FakePrinter::getPrintSize()
{
    return QSize(3570,2380); //hard coded pixel size
}

bool FakePrinter::printerOnline()
{
    return true;
}

bool FakePrinter::busy()
{
    return mBusyTimer.isActive();
}

int FakePrinter::printImage(const QString &filename)
{
    qDebug() << "Fake printer starts printing " << filename;
    mBusyTimer.start(1000 * 30); // 30 seconds;
    emit busyChanged(true);
    QObject::connect(&mBusyTimer, SIGNAL(timeout()), this, SLOT(busyTimeout()));
}

void FakePrinter::busyTimeout()
{
    mBusyTimer.stop();
    emit success();
    emit busyChanged(false);
    qDebug() << "Fake printer has finished";
}
