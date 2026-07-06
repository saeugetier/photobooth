#ifndef PRINTERFACTORY_H
#define PRINTERFACTORY_H

#include <QObject>
#include <QTimer>
#include "abstractprinter.h"
#include <memory>

class PrinterFactory : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList printers READ printers NOTIFY printersChanged)
    Q_PROPERTY(QString defaultPrinterName READ defaultPrinterName CONSTANT)
public:
    explicit PrinterFactory(QObject *parent = nullptr);
    QStringList printers() const;
    Q_INVOKABLE AbstractPrinter* getPrinter(const QString &name);
    QString defaultPrinterName() const;
signals:
    void printersChanged();
protected:
    std::unique_ptr<AbstractPrinter> mCurrentPrinter;
    QStringList mLastPrinterList;
    QTimer mPrinterRefreshTimer;
};

#endif // PRINTERFACTORY_H
