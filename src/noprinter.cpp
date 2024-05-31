#include "noprinter.h"

bool NoPrinter::printerOnline()
{
    return false;
}

bool NoPrinter::busy()
{
    return false;
}

int NoPrinter::printImage(const QString &filename, int copyCount)
{
    Q_UNUSED(filename);
}

NoPrinter::NoPrinter(QObject *parent) : AbstractPrinter(parent)
{

}

QStringList NoPrinter::getAvailablePrintersInternal()
{
    QStringList list;
    list.append("No Printer");
    return list;
}

NoPrinter *NoPrinter::createInternal(const QString &name)
{
    Q_UNUSED(name);
    return new NoPrinter();
}
