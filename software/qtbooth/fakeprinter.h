#ifndef FAKEPRINTER_H
#define FAKEPRINTER_H

#include "abstractprinter.h"
#include <QTimer>

class FakePrinter : public AbstractPrinter
{
    Q_OBJECT
public:
    FakePrinter(QObject *parent = nullptr) : AbstractPrinter(parent) {};
    Q_INVOKABLE virtual QSize getPrintSize();
    Q_INVOKABLE virtual bool printerOnline();
    Q_INVOKABLE virtual bool busy();
public slots:
    Q_INVOKABLE virtual int printImage(const QString &filename);
protected slots:
    void busyTimeout();
protected:
    QTimer mBusyTimer;
};

#endif // FAKEPRINTER_H
