#ifndef PRINTER_H
#define PRINTER_H

#include "abstractprinter.h"
#include <QProcess>

class SelphyPrinter : public AbstractPrinter, public PrinterList<SelphyPrinter>
{
    friend PrinterList<SelphyPrinter>;
    Q_OBJECT
    Q_INTERFACES(AbstractPrinter)
public:
    Q_INVOKABLE bool printerOnline();
    bool busy();
public slots:
    Q_INVOKABLE int printImage(const QString &filename, int copyCount);
protected slots:
    void finished(int code, QProcess::ExitStatus status);
protected:
    explicit SelphyPrinter(const QString &name, QObject *parent = nullptr);
    QString mIp;
    QProcess mPrinterProcess;
    static QStringList getAvailablePrintersInternal();
    static SelphyPrinter* createInternal(const QString &name);
};

#endif // PRINTER_H
