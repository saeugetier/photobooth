#include "printerfactory.h"
#include "noprinter.h"
#include "fakeprinter.h"
#include "selphyprinter.h"

PrinterFactory::PrinterFactory(QObject *parent) : QObject(parent)
{

}

QStringList PrinterFactory::printers() const
{
    QStringList printerList;
    printerList.append(NoPrinter::getAvailablePrinters());
    printerList.append(FakePrinter::getAvailablePrinters());
    printerList.append(SelphyPrinter::getAvailablePrinters());
    return printerList;
}

AbstractPrinter *PrinterFactory::getPrinter(const QString &name)
{
    if(NoPrinter::getAvailablePrinters().contains(name))
    {
        decltype (mCurrentPrinter) ptr(NoPrinter::create(name));
        mCurrentPrinter.swap(ptr);
    }
    else if(FakePrinter::getAvailablePrinters().contains(name))
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
    return NoPrinter::getAvailablePrinters().first();
}
