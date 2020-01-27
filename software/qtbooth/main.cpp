#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QFontDatabase>
#include <QDebug>
#include <QResource>
#include <QSettings>
#include <QQmlContext>
#include <QTranslator>
#include "myhelper.h"
#include "selphyprinter.h"
#include "collagemodelfactory.h"
#include "gpio.h"
#include "fileio.h"
#include "system.h"

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

    qmlRegisterType<CollageModelFactory>("CollageModel", 1, 0, "CollageModelFactory");
    qmlRegisterType<CollageIconModel>("CollageModel", 1, 0, "CollageIconModel");
    qmlRegisterType<CollageImageModel>("CollageModel", 1, 0, "CollageImageModel");

    qmlRegisterType<GPIO>("GPIO", 1, 0, "GPIO");

    qmlRegisterType<FileIO>("FileIO", 1, 0, "FileIO");

    qmlRegisterType<System>("Syetem", 1, 0, "System");

    qmlRegisterType<SelphyPrinter>("Selphy", 1, 0, "Printer");

    QQmlApplicationEngine engine;
    engine.addImportPath("qrc:///");
    engine.load(QUrl(QLatin1String("qrc:/Application.qml")));

    QObject::connect(MyHelper::instance(), SIGNAL(languageChanged()), &engine, SLOT(retranslate()));

    int result = app.exec();

    return result;
}
