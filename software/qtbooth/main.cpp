#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QFontDatabase>
#include <QDebug>
#include <QResource>
#include <QSettings>
#include <QQmlContext>
#include <QTranslator>
#include "translationhelper.h"
#include "fakeprinter.h"
#include "selphyprinter.h"
#include "collagemodelfactory.h"
#include "gpio.h"
#include "fileio.h"
#include "filesystem.h"
#include "system.h"

#define FAKEPRINTER 1

int main(int argc, char *argv[])
{
    qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));

    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    QGuiApplication::setOrganizationName("saeugetier");
    QGuiApplication::setOrganizationDomain("github.com");
    QGuiApplication::setApplicationName("qtbooth");

    QFontDatabase fontDatabase;
    if (fontDatabase.addApplicationFont(":/fontello/font/fontello.ttf") == -1)
        qWarning() << "Failed to load fontello.ttf";

    qmlRegisterType<CollageModelFactory>("CollageModel", 1, 0, "CollageModelFactory");
    qmlRegisterUncreatableType<CollageIconModel>("CollageModel", 1, 0, "CollageIconModel", "CollageIconModel can only be created via CollageModeFactory");
    qmlRegisterUncreatableType<CollageImageModel>("CollageModel", 1, 0, "CollageImageModel", "CollageImageModel can only be created via CollageModeFactory");

    qmlRegisterType<GPIO>("GPIO", 1, 0, "GPIO");

    qmlRegisterType<FileIO>("FileIO", 1, 0, "FileIO");

    qmlRegisterType<System>("Syetem", 1, 0, "System");

#if FAKEPRINTER == 1
    qmlRegisterType<FakePrinter>("Selphy", 1, 0, "Printer");
#else
    qmlRegisterType<SelphyPrinter>("Selphy", 1, 0, "Printer");
#endif

    FileSystem fileSystem;
    TranslationHelper translationHelper;
    QQmlApplicationEngine engine;
    System system;
    engine.addImportPath("qrc:///");
    engine.rootContext()->setContextProperty("translation", &translationHelper);
    engine.rootContext()->setContextProperty("filesystem", &fileSystem);
    engine.rootContext()->setContextProperty("system", &system);
    engine.load(QUrl(QLatin1String("qrc:/Application.qml")));

    QObject::connect(&translationHelper, SIGNAL(languageChanged()), &engine, SLOT(retranslate()));

    int result = app.exec();

    return result;
}
