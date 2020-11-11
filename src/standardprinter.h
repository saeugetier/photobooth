#ifndef CUPSPRINTER_H
#define CUPSPRINTER_H

#include "abstractprinter.h"
#include <QPrinter>
#include <QPrinterInfo>

class StandardPrinter : public AbstractPrinter, public PrinterList<StandardPrinter>
{
    friend class PrinterList<StandardPrinter>;
    Q_OBJECT
    Q_INTERFACES(AbstractPrinter)
public:
    QSize getPrintSize() override;
    bool printerOnline() override;
    bool busy() override;
    int printImage(const QString &filename, int copyCount) override;
protected:
    explicit StandardPrinter(QPrinterInfo& info, QObject *parent = nullptr);
    static QStringList getAvailablePrintersInternal();
    static StandardPrinter *createInternal(const QString &name);

    QPrinter mPrinter;
};

#endif // CUPSPRINTER_H
