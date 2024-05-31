#include "fakeprinter.h"
#include <QDebug>

FakePrinter::FakePrinter(QObject *parent) : AbstractPrinter(parent)
{
    QObject::connect(&mBusyTimer, SIGNAL(timeout()), this, SLOT(busyTimeout()));
};

bool FakePrinter::printerOnline()
{
    return true;
}

bool FakePrinter::busy()
{
    return mBusyTimer.isActive();
}

int FakePrinter::printImage(const QString &filename, int copyCount)
{
    qDebug() << "Fake printer starts printing " << copyCount << " copies." << filename;
    mBusyTimer.start(1000 * 30); // 30 seconds;
    emit busyChanged(true);
    return 0;
}

void FakePrinter::busyTimeout()
{
    mBusyTimer.stop();
    emit success();
    emit busyChanged(false);
    qDebug() << "Fake printer has finished";
}

QStringList FakePrinter::getAvailablePrintersInternal()
{
    QStringList list;
    list.append("Fake Printer");
    return list;
}

FakePrinter *FakePrinter::createInternal(const QString &name)
{
    Q_UNUSED(name);
    return new FakePrinter();
}
