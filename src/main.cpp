#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QFontDatabase>
#include <QDebug>
#include <QResource>
#include <QSettings>
#include <QQmlContext>
#include <QTranslator>
#include "gphotocamera.h"
#include "translationhelper.h"
#include "captureprocessor.h"
#include "fakeprinter.h"
#include "selphyprinter.h"
#include "printerfactory.h"
#include "collagemodelfactory.h"
#include "replacebackgroundvideofilter.h"
#include "gpio.h"
#include "fileio.h"
#include "filesystem.h"
#include "system.h"
#include <iostream>

//some constants to parameterize.
const qint64 LOG_FILE_LIMIT = 3000000;
const QString LOG_PATH = "/logs/";
const QString LOG_FILENAME = "qtbooth.log";

//thread safety
QMutex mutex;

void redirectDebugMessages(QtMsgType type, const QMessageLogContext & context, const QString & str)
{
    //thread safety
    mutex.lock();
    QString txt;

    //prepend timestamp to every message
    QString datetime = QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss");
    //prepend a log level label to every message
    switch (type) {
    case QtDebugMsg:
        txt = QString("[Debug*] ");
        break;
    case QtWarningMsg:
        txt = QString("[Warning] ");
        break;
    case QtInfoMsg:
        txt = QString("[Info] ");
        break;
    case QtCriticalMsg:
        txt = QString("[Critical] ");
        break;
    case QtFatalMsg:
        txt = QString("[Fatal] ");
    }

    QDir dir;

    QString filePath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + LOG_PATH;

    if (!dir.exists(filePath))
        dir.mkpath(filePath); // You can check the success if needed

    filePath = filePath + LOG_FILENAME;

    QFile outFile(filePath);

    //if file reached the limit, rotate to filename.1
    if(outFile.size() > LOG_FILE_LIMIT){
        //roll the log file.
        QFile::remove(filePath + ".1");
        QFile::rename(filePath, filePath + ".1");
        QFile::resize(filePath, 0);
    }

    //write message
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << datetime << txt << context.file << str << "\n";

    if(context.file != nullptr && context.line != 0)
    {
        std::cout << datetime.toStdString() <<  " - " << txt.toStdString() << " - " << context.file << "(" << context.line << ") - " << str.toStdString() << std::endl;
    }
    else
    {
        std::cout << datetime.toStdString() <<  " - " << txt.toStdString() << " - " << str.toStdString() << std::endl;
    }

    //close fd
    outFile.close();
    mutex.unlock();
}

int main(int argc, char *argv[])
{
    qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));
    //qputenv("QSG_VISUALIZE", QByteArray("overdraw"));
    //qputenv("QSG_INFO", "1");
    //QLoggingCategory::setFilterRules("qt.scenegraph.general=true");

    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    QGuiApplication::setOrganizationName("saeugetier");
    QGuiApplication::setOrganizationDomain("github.com");
    QGuiApplication::setApplicationName("qtbooth");

    qInstallMessageHandler(redirectDebugMessages);

    if (QFontDatabase::addApplicationFont(":/font/fontello/fontello.ttf") == -1)
        qWarning() << "Failed to load fontello.ttf";
    if (QFontDatabase::addApplicationFont(":/font/DejaVuSerif/DejaVuSerif.ttf") == -1)
        qWarning() << "Failed to load DejaVuSerif.ttf";

    qDebug() << "Standard path: " << QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
    qDebug() << "Standard path: " << QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);

    qmlRegisterType<CollageModelFactory>("CollageModel", 1, 0, "CollageModelFactory");
    qmlRegisterUncreatableType<CollageIconModel>("CollageModel", 1, 0, "CollageIconModel", "CollageIconModel can only be created via CollageModeFactory");
    qmlRegisterUncreatableType<CollageImageModel>("CollageModel", 1, 0, "CollageImageModel", "CollageImageModel can only be created via CollageModeFactory");

    qmlRegisterType<GPIO>("GPIO", 1, 0, "GPIO");

    qmlRegisterType<FileIO>("FileIO", 1, 0, "FileIO");

    qmlRegisterType<System>("System", 1, 0, "System");

    qmlRegisterType<CaptureProcessor>("CaptureProcessor", 1, 0, "CaptureProcessor");
    qmlRegisterType<GPhotoCameraDevice>("GPhotoCamera", 1, 0, "GPhotoCamera");

    qmlRegisterInterface<AbstractPrinter>("AbstractPrinter", 1);
    qmlRegisterUncreatableType<AbstractPrinter>("Printer", 1, 0, "Printer", "Printer can only be created via PrinterFactory");
    qmlRegisterType<PrinterFactory>("Printer", 1, 0, "PrinterFactory");

    qmlRegisterType<ReplaceBackgroundVideoFilter>("BackgroundFilter", 1, 0, "ReplaceBackgroundVideoFilter");

    FileSystem fileSystem;
    TranslationHelper translationHelper;
    QQmlApplicationEngine engine;
    System system;
    engine.addImportPath("qrc:///");
    engine.rootContext()->setContextProperty("translation", &translationHelper);
    engine.rootContext()->setContextProperty("filesystem", &fileSystem);
    engine.rootContext()->setContextProperty("system", &system);
    engine.load(QUrl(QLatin1String("qrc:/qml/Application.qml")));
    engine.retranslate();

    QObject::connect(&translationHelper, SIGNAL(languageChanged()), &engine, SLOT(retranslate()));

    int result = app.exec();

    return result;
}
