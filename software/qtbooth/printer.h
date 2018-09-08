#ifndef PRINTER_H
#define PRINTER_H

#include <QObject>
#include <QSize>
#include <QProcess>

class Printer : public QObject
{
    Q_OBJECT
private:
    Printer(QObject *parent = nullptr);
    static Printer* createInstance();
public:
    static Printer* instance();

    bool busy();

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
