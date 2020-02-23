#include "translationhelper.h"
#include <QGuiApplication>

TranslationHelper::TranslationHelper(QObject *parent) : QObject(parent)
{

}

QTranslator* TranslationHelper::getTranslator()
{
    return m_Translator.get();
}

void TranslationHelper::setLanguage(QString code)
{
    if(m_Translator.get() != NULL)
        QGuiApplication::removeTranslator(m_Translator.get());

    std::unique_ptr<QTranslator> translator(new QTranslator);
    m_Translator.swap(translator);
    m_Translator->load("tr_" + code, ":/qml/");
    QGuiApplication::installTranslator(m_Translator.get());

    emit languageChanged();
}

QStringList TranslationHelper::getAvailableLanguages()
{

}
