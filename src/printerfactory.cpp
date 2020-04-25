#include "printerfactory.h"
#include "fakeprinter.h"
#include "selphyprinter.h"

PrinterFactory::PrinterFactory(QObject *parent) : QObject(parent)
{

}

QStringList PrinterFactory::printers() const
{
    QStringList printerList;
    printerList.append(FakePrinter::getAvailablePrinters());
    printerList.append(SelphyPrinter::getAvailablePrinters());
    return printerList;
}

AbstractPrinter *PrinterFactory::getPrinter(const QString &name)
{
    if(FakePrinter::getAvailablePrinters().contains(name))
    {
        decltype (mCurrentPrinter) ptr(FakePrinter::create(name));
        mCurrentPrinter.swap(ptr);
    }
    else if(SelphyPrinter::getAvailablePrinters().contains(name))
    {
        decltype (mCurrentPrinter) ptr(SelphyPrinter::create(name));
        mCurrentPrinter.swap(ptr);
    }
    return mCurrentPrinter.get();
}

QString PrinterFactory::defaultPrinterName() const
{
    return FakePrinter::getAvailablePrinters().first();
}
