#ifndef MYHELPER_H
#define MYHELPER_H

#include <QObject>
#include <QSize>

class MyHelper : public QObject
{
    Q_OBJECT
private:
    MyHelper(QObject *parent = nullptr);
    static MyHelper* createInstance();
public:
    static MyHelper* instance();
public:
    Q_INVOKABLE void removeFile(const QString &filename);
    Q_INVOKABLE void printImage(const QString &filename);
    Q_INVOKABLE QString getImagePath();
    Q_INVOKABLE QSize getPrintSize();
};

#endif // MYHELPER_H
