#include "printerfactory.h"
#include "noprinter.h"
#include "fakeprinter.h"
#include "selphyprinter.h"
#include "standardprinter.h"
#include <QDebug>

PrinterFactory::PrinterFactory(QObject *parent) : QObject(parent)
{

}

QStringList PrinterFactory::printers() const
{
    QStringList printerList;
    printerList.append(NoPrinter::getAvailablePrinters());
    printerList.append(FakePrinter::getAvailablePrinters());
    printerList.append(SelphyPrinter::getAvailablePrinters());
    printerList.append(StandardPrinter::getAvailablePrinters());
    return printerList;
}

AbstractPrinter *PrinterFactory::getPrinter(const QString &name)
{
    qDebug() << "Requested printer with name: " << name;
    if(NoPrinter::getAvailablePrinters().contains(name))
    {
        decltype (mCurrentPrinter) ptr(NoPrinter::create(name));
        mCurrentPrinter.swap(ptr);
    }
    else if(FakePrinter::getAvailablePrinters().contains(name))
    {
        qDebug() << "FakePrinter selected";
        decltype (mCurrentPrinter) ptr(FakePrinter::create(name));
        mCurrentPrinter.swap(ptr);
    }
    else if(SelphyPrinter::getAvailablePrinters().contains(name))
    {
        qDebug() << "SelphyPrinter selected";
        decltype (mCurrentPrinter) ptr(SelphyPrinter::create(name));
        mCurrentPrinter.swap(ptr);
    }
    else if(StandardPrinter::getAvailablePrinters().contains(name))
    {
        qDebug() << "StandardPrinter selected";
        decltype (mCurrentPrinter) ptr(StandardPrinter::create(name));
        mCurrentPrinter.swap(ptr);
    }
    else
    {
        qDebug() << "Error: Printer with name \"" << name << "\" is not available!";
    }
    return mCurrentPrinter.get();
}

QString PrinterFactory::defaultPrinterName() const
{
    return NoPrinter::getAvailablePrinters().first();
}
