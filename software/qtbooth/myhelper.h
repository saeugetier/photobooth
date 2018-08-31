#ifndef MYHELPER_H
#define MYHELPER_H

#include <QObject>
#include <QSize>
#include <QTranslator>
#include <memory>
#include <QtConcurrent/QtConcurrent>

class MyHelper : public QObject
{
    Q_OBJECT
private:
    MyHelper(QObject *parent = nullptr);
    static MyHelper* createInstance();
public:
    static MyHelper* instance();

    QTranslator* getTranslator();
public:
    Q_INVOKABLE void removeFile(const QString &filename);
    Q_INVOKABLE QString getImagePath();
    Q_INVOKABLE void shutdown();
    Q_INVOKABLE void restart();
    Q_INVOKABLE void setLanguage(QString code);
    Q_INVOKABLE bool removableDriveMounted();
    Q_INVOKABLE void unmountRemoveableDrive();
    Q_INVOKABLE void startCopyFilesToRemovableDrive();
protected:
    std::unique_ptr<QTranslator> mTranslator;
    QString getRemovableDrivePath();
signals:
    void languageChanged();
    void copyProgress(int);
};

#endif // MYHELPER_H
