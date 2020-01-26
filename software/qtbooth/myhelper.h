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
    Q_INVOKABLE QString getImagePath();
    Q_INVOKABLE void setLanguage(QString code);
    Q_INVOKABLE bool removableDriveMounted();
    Q_INVOKABLE void unmountRemoveableDrive();
    Q_INVOKABLE void startCopyFilesToRemovableDrive();
    Q_INVOKABLE void abortCopy();
    Q_INVOKABLE void deleteAllImages();
protected:
    std::unique_ptr<QTranslator> m_Translator;
    QString getRemovableDrivePath();
    QFuture<void> m_copyFuture;
signals:
    void languageChanged();
    void copyProgress(int progress);
    void copyFinished();
};

#endif // MYHELPER_H
