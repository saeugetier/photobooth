#include "selphyprinter.h"
#include "singleton.h"
#include <QFile>
#include <QRegExp>
#include <QTextStream>
#include <QDebug>


SelphyPrinter::SelphyPrinter(QObject *parent) : QObject(parent)
{
    QObject::connect(&mPrinterProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(finished(int,QProcess::ExitStatus)));
    QObject::connect(&mPrinterProcess, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SIGNAL(failed()));
}


SelphyPrinter* SelphyPrinter::createInstance()
{
    return new SelphyPrinter();
}


SelphyPrinter* SelphyPrinter::instance()
{
    return Singleton<SelphyPrinter>::instance(SelphyPrinter::createInstance);
}

bool SelphyPrinter::busy()
{
    return mPrinterProcess.state() != QProcess::NotRunning;
}

QString SelphyPrinter::getPrinterIp()
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
                    qDebug() << "Selphy Printer - Found IP: " << ip;
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

bool SelphyPrinter::printerOnline()
{
    QString ip = getPrinterIp();
    if(ip.length() == 0)
        return false;

    QStringList arguments;
    arguments << "-c" << "1" << ip;
    int exitcode = QProcess::execute("ping", arguments);
    if(exitcode == 0)
    {
        qDebug() << "SelphyPrinter on IP " << ip << " is online.";
        return true;
    }
    else
    {
        qDebug() << "SelphyPrinter on IP " << ip << " seems to be offline.";
        return false;
    }
}


QSize SelphyPrinter::getPrintSize()
{
    return QSize(3570,2380); //hard coded pixel size
}

int SelphyPrinter::printImage(const QString &filename)
{
    QString ip = getPrinterIp();
    if(ip.length() > 0)
    {

        QStringList parameters;
        parameters << "-SelphyPrinter_ip=" + ip;
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
        qDebug() << "Print failed. No Selphy Printer connected!";

    return -1;
}

void SelphyPrinter::finished(int code, QProcess::ExitStatus status)
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

