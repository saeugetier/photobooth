#ifndef FAKEPRINTER_H
#define FAKEPRINTER_H

#include "abstractprinter.h"
#include <QTimer>

class FakePrinter : public AbstractPrinter
{
    Q_OBJECT
    Q_INTERFACES(AbstractPrinter)
public:
    explicit FakePrinter(QObject *parent = nullptr);
    QSize getPrintSize() override;
    bool printerOnline() override;
    bool busy() override;
    int printImage(const QString &filename) override;
protected slots:
    void busyTimeout();
protected:
    QTimer mBusyTimer;
};

#endif // FAKEPRINTER_H
