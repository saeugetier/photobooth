#ifndef PRINTER_H
#define PRINTER_H

#include "abstractprinter.h"
#include <QProcess>

class SelphyPrinter : public AbstractPrinter
{
    Q_OBJECT
public:
    SelphyPrinter(QObject *parent = nullptr);
    QString getPrinterIp();
    Q_INVOKABLE QSize getPrintSize();
    Q_INVOKABLE bool printerOnline();
    bool busy();
public slots:
    Q_INVOKABLE int printImage(const QString &filename);
protected slots:
    void finished(int code, QProcess::ExitStatus status);
protected:
    QProcess mPrinterProcess;
};

#endif // PRINTER_H
