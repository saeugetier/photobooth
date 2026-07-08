#ifndef CUPSPRINTER_H
#define CUPSPRINTER_H

#include "abstractprinter.h"
#include <QPrinter>
#include <QPrinterInfo>
#include <QThread>

class StandardPrinter : public AbstractPrinter, public PrinterList<StandardPrinter>
{
    friend class PrinterList<StandardPrinter>;
    Q_OBJECT
    Q_INTERFACES(AbstractPrinter)
public:
    ~StandardPrinter() override;
    bool printerOnline() override;
    bool busy() override;
    int printImage(const QString &filename, int copyCount) override;
private:
    void finishPrint(int result);

protected:
    explicit StandardPrinter(QPrinterInfo& info, QObject *parent = nullptr);
    static QStringList getAvailablePrintersInternal();
    static StandardPrinter *createInternal(const QString &name);

    QPrinter mPrinter;
    QString mPrinterName;
    bool mBusy = false;
    QThread *mPrintThread = nullptr;
};

#endif // CUPSPRINTER_H
