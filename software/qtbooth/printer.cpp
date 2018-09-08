#include "printer.h"
#include "singleton.h"
#include <QFile>
#include <QRegExp>
#include <QTextStream>
#include <QDebug>


Printer::Printer(QObject *parent) : QObject(parent)
{
    QObject::connect(&mPrinterProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(finished(int,QProcess::ExitStatus)));
    QObject::connect(&mPrinterProcess, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SIGNAL(failed()));
}


Printer* Printer::createInstance()
{
    return new Printer();
}


Printer* Printer::instance()
{
    return Singleton<Printer>::instance(Printer::createInstance);
}

bool Printer::busy()
{
    return mPrinterProcess.state() != QProcess::NotRunning;
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

bool Printer::printerOnline()
{
    QString ip = getPrinterIp();
    if(ip.length() == 0)
        return false;

    QStringList arguments;
    arguments << " -c 1 " << ip;
    int exitcode = QProcess::execute("ping", arguments);
    if(exitcode == 0)
        return true;
    else
        return false;
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

        QStringList parameters;
        parameters << "-printer_ip=" + ip;
        parameters << filename.right(filename.length() - QString("file://").length());

        if(mPrinterProcess.state() == QProcess::NotRunning)
        {
            if(printerOnline())
            {
                mPrinterProcess.start("selphy", parameters);
                qDebug() << parameters;
                return 0;
            }
            else
                return -1;
        }
        else
            return -1;
    }
    else
        qDebug() << "Print failed. No printer connected!";

    return -1;
}

void Printer::finished(int code, QProcess::ExitStatus status)
{
    if(code != 0)
    {
        qDebug() << "Selphy Error: \n" << mPrinterProcess.readAllStandardError();
        qDebug() << "Code: " << code << " - Status: " << status;
        emit failed();
    }
    else
        emit success();
}

