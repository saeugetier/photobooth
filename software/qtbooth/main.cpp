#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QFontDatabase>
#include <QDebug>
#include <QResource>
#include <QSettings>
#include <QQmlContext>
#include "myhelper.h"
#include "photomontage.h"
#include "ledflash.h"
#include "printer.h"

int main(int argc, char *argv[])
{
    qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));

    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    QGuiApplication::setOrganizationName("Timmedia");
    QGuiApplication::setOrganizationDomain("timmedia.de");
    QGuiApplication::setApplicationName("QML Photo Booth");

    QFontDatabase fontDatabase;
    if (fontDatabase.addApplicationFont(":/fontello/font/fontello.ttf") == -1)
        qWarning() << "Failed to load fontello.ttf";

    qmlRegisterType<PhotoMontage>("Montage", 1, 0, "PhotoMontage");

    QQmlApplicationEngine engine;
    engine.addImportPath("qrc:///");
    engine.rootContext()->setContextProperty("flash", LedFlash::instance());
    engine.rootContext()->setContextProperty("helper", MyHelper::instance());
    engine.rootContext()->setContextProperty("printer", Printer::instance());
    engine.load(QUrl(QLatin1String("qrc:/main.qml")));    

    int result = app.exec();

    return result;
}
