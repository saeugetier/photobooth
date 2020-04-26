#ifndef NOPRINTER_H
#define NOPRINTER_H

#include "abstractprinter.h"

class NoPrinter : public AbstractPrinter, public PrinterList<NoPrinter>
{
    friend class PrinterList<NoPrinter>;
    Q_OBJECT
    Q_INTERFACES(AbstractPrinter)
public:
    QSize getPrintSize() override;
    bool printerOnline() override;
    bool busy() override;
    int printImage(const QString &filename) override;
protected:
    explicit NoPrinter(QObject *parent = nullptr);
    static QStringList getAvailablePrintersInternal();
    static NoPrinter* createInternal(const QString &name);
};

#endif // NOPRINTER_H
