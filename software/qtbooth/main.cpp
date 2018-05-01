#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QFontDatabase>
#include <QDebug>
#include <QResource>
#include <QSettings>
#include <QQmlContext>
#include "myhelper.h"
#include "photomontage.h"

int main(int argc, char *argv[])
{
    //qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    QCoreApplication::setOrganizationName("Timmedia");
    QCoreApplication::setOrganizationDomain("timmedia.de");
    QCoreApplication::setApplicationName("QML Photo Booth");

    QFontDatabase fontDatabase;
    if (fontDatabase.addApplicationFont(":/fontello/font/fontello.ttf") == -1)
        qWarning() << "Failed to load fontello.ttf";

    qmlRegisterType<PhotoMontage>("Montage", 1, 0, "PhotoMontage");

    QQmlApplicationEngine engine;
    engine.addImportPath("qrc:///");
    engine.load(QUrl(QLatin1String("qrc:/main.qml")));

    MyHelper helper;
    engine.rootContext()->setContextProperty("helper", &helper);

    int result = app.exec();

    return result;
}
