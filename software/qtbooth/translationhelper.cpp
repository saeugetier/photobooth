#include "translationhelper.h"
#include <QGuiApplication>
#include <QDirIterator>

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
    if(m_Translator->load("tr_" + code, ":/qml/"))
    {
        QGuiApplication::installTranslator(m_Translator.get());
    }

    emit languageChanged();
}

QStringList TranslationHelper::getAvailableLanguages()
{
    QStringList translationFiles("en");
    QDirIterator it(":", QStringList("*.qm"), QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString filepath = it.next();
        QFileInfo info(filepath);
        QString filename = info.fileName();
        filename.remove("tr_");
        filename.remove(".qm");
        translationFiles << filename;
    }
    return translationFiles;
}
