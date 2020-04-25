#ifndef PRINTERFACTORY_H
#define PRINTERFACTORY_H

#include <QObject>
#include "abstractprinter.h"
#include <memory>

class PrinterFactory : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList printers READ printers CONSTANT)
    Q_PROPERTY(QString defaultPrinterName READ defaultPrinterName CONSTANT)
public:
    explicit PrinterFactory(QObject *parent = nullptr);
    QStringList printers() const;
    Q_INVOKABLE AbstractPrinter* getPrinter(const QString &name);
    QString defaultPrinterName() const;
signals:
protected:
    std::unique_ptr<AbstractPrinter> mCurrentPrinter;
};

#endif // PRINTERFACTORY_H
