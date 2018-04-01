#ifndef MYHELPER_H
#define MYHELPER_H

#include <QObject>

class MyHelper : public QObject
{
    Q_OBJECT
public:
    explicit MyHelper(QObject *parent = nullptr);

public:
    Q_INVOKABLE void removeFile(const QString &filename);
    Q_INVOKABLE void printImage(const QString &filename);
};

#endif // MYHELPER_H
