#ifndef ABSTRACTPRINTER_H
#define ABSTRACTPRINTER_H

#include <QObject>
#include <QSize>

class AbstractPrinter : public QObject
{
    Q_OBJECT
public:
    AbstractPrinter(QObject *parent = nullptr) : QObject(parent) {};
    Q_INVOKABLE virtual QSize getPrintSize() = 0;
    Q_INVOKABLE virtual bool printerOnline() = 0;
    Q_INVOKABLE virtual bool busy() = 0;
signals:
    void progress(int);
    void success();
    void failed();

public slots:
    Q_INVOKABLE virtual int printImage(const QString &filename) = 0;
};

#endif // ABSTRACTPRINTER_H
