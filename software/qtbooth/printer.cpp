#include "printer.h"
#include "singleton.h"
#include <QFile>
#include <QRegExp>
#include <QTextStream>
#include <QDebug>


Printer::Printer(QObject *parent) : QObject(parent)
{
    QObject::connect(&mPrinterWatcher, SIGNAL(finished()), this, SLOT(finished()));
}


Printer* Printer::createInstance()
{
    return new Printer();
}


Printer* Printer::instance()
{
    return Singleton<Printer>::instance(Printer::createInstance);
}

QString Printer::getPrinterIp()
{
    QString ip;

    QFile dnsleases("/var/lib/misc/dnsmasq.leases");
    if(dnsleases.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&dnsleases);

        for(;!stream.atEnd();)
        {
            QString line;
            stream.readLineInto(&line);
            if(line.toUpper().contains("SELPHY"))
            {
                qDebug() << "Dnsleases - Found match: " << line;
                QRegExp regex("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}");
                if(regex.indexIn(line) != -1)
                {
                    ip = regex.capturedTexts().first();
                    qDebug() << "Printer - Found IP: " << ip;
                }
                break;
            }
        }

    }
    else
    {
        qDebug() << "ERROR: Could not open dns leases file!";
    }

    return ip;
}


QSize Printer::getPrintSize()
{
    return QSize(3570,2380); //hard coded pixel size
}

int Printer::printImage(const QString &filename)
{
    QString ip = getPrinterIp();
    if(ip.length() > 0)
    {
        QStringList filenames;
        filenames.push_back(filename);
        /*std::function<int(const QString &)> print_func
                = [&ip](const QString& filename) -> int
        {
           return print((char*)filename.toStdString().c_str(),
                 (char*) ip.toStdString().c_str(),
                 NULL);
        };
        mPrinterWatcher.setFuture(QtConcurrent::mapped(filenames, print_func));*/
        return 0;
    }
    else
        qDebug() << "Print failed. No printer connected!";

    return -1;
}

void Printer::finished()
{
    if(mPrinterWatcher.result() < 0)
        emit failed();
    else
        emit success();
}

