#ifndef MYHELPER_H
#define MYHELPER_H

#include <QObject>
#include <QSize>
#include <QTranslator>
#include <memory>

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
protected:
    std::unique_ptr<QTranslator> mTranslator;
signals:
    void languageChanged();
};

#endif // MYHELPER_H
