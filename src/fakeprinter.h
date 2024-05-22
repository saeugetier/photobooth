#ifndef FAKEPRINTER_H
#define FAKEPRINTER_H

#include "abstractprinter.h"
#include <QTimer>

class FakePrinter : public AbstractPrinter, public PrinterList<FakePrinter>
{
    friend class PrinterList<FakePrinter>;
    Q_OBJECT
    Q_INTERFACES(AbstractPrinter)
public:
    QSize getPrintSize() override;
    bool printerOnline() override;
    bool busy() override;
    int printImage(const QString &filename, int copyCount) override;
protected slots:
    void busyTimeout();
protected:
    explicit FakePrinter(QObject *parent = nullptr);
    QTimer mBusyTimer;
    static QStringList getAvailablePrintersInternal();
    static FakePrinter* createInternal(const QString &name);
};

#endif // FAKEPRINTER_H
