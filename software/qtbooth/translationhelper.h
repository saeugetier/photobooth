#ifndef TRANSLATIONHELPER_H
#define TRANSLATIONHELPER_H

#include <QObject>
#include <QTranslator>
#include <memory>

class TranslationHelper : public QObject
{
    Q_OBJECT
public:
    explicit TranslationHelper(QObject *parent = nullptr);
    QTranslator* getTranslator();
public:
    Q_INVOKABLE void setLanguage(QString code);
    Q_INVOKABLE QStringList getAvailableLanguages();
protected:
    std::unique_ptr<QTranslator> m_Translator;
signals:
    void languageChanged();
};

#endif // TRANSLATIONHELPER_H
