#ifndef PRINTER_H
#define PRINTER_H

#include <QObject>
#include <QSize>
#include <QProcess>

class SelphyPrinter : public QObject
{
    Q_OBJECT
public:
    SelphyPrinter(QObject *parent = nullptr);

    Q_INVOKABLE bool busy();
signals:
    void progress(int);
    void success();
    void failed();

public slots:
    Q_INVOKABLE QString getPrinterIp();
    Q_INVOKABLE int printImage(const QString &filename);
    Q_INVOKABLE QSize getPrintSize();
    Q_INVOKABLE bool printerOnline();

protected slots:
    void finished(int code, QProcess::ExitStatus status);
protected:
    QProcess mPrinterProcess;
};

#endif // PRINTER_H
