#include "selphyprinter.h"
#include <QFile>
#include <QRegExp>
#include <QTextStream>
#include <QDebug>

SelphyPrinter::SelphyPrinter(const QString &name, QObject *parent) : AbstractPrinter(parent), mIp("")
{
    QRegExp regex("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}");
    if(regex.indexIn(name) != -1)
    {
        mIp = regex.capturedTexts().first();
    }

    QObject::connect(&mPrinterProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(finished(int,QProcess::ExitStatus)));
    QObject::connect(&mPrinterProcess, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SIGNAL(failed()));
}

bool SelphyPrinter::busy()
{
    return mPrinterProcess.state() != QProcess::NotRunning;
}

bool SelphyPrinter::printerOnline()
{
    if(mIp.length() == 0)
        return false;

    QStringList arguments;
    arguments << "-c" << "1" << mIp;
    int exitcode = QProcess::execute("ping", arguments);
    if(exitcode == 0)
    {
        qDebug() << "SelphyPrinter on IP " << mIp << " is online.";
        return true;
    }
    else
    {
        qDebug() << "SelphyPrinter on IP " << mIp << " seems to be offline.";
        return false;
    }
}


QSize SelphyPrinter::getPrintSize()
{
    return QSize(3570,2380); //hard coded pixel size
}

int SelphyPrinter::printImage(const QString &filename)
{
    if(mIp.length() > 0)
    {
        QString imageMagickCommand = "convert " + filename + " -quality 100% " + filename + ".jpg";
        QString selphyCommand = "selphy -printer_ip=" + mIp + " " + filename + ".jpg";
        QStringList shParameters;
        shParameters << "-c";
        shParameters << imageMagickCommand + " && " + selphyCommand;

        if(mPrinterProcess.state() == QProcess::NotRunning)
        {
            if(printerOnline())
            {
                emit busyChanged(true);
                mPrinterProcess.start("sh", shParameters);
                qDebug() << shParameters;
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
    emit busyChanged(false);
    if(code != 0)
    {
        qDebug() << "Selphy Error: \n" << mPrinterProcess.readAllStandardError();
        qDebug() << "Code: " << code << " - Status: " << status;
        emit failed();
    }
    else
        emit success();
}

QStringList SelphyPrinter::getAvailablePrintersInternal()
{
    QStringList list;

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
                    QString ip = regex.capturedTexts().first();
                    qDebug() << "Selphy Printer - Found IP: " << ip;

                    list.append("Selphy " + ip);
                }
                break;
            }
        }

    }
    else
    {
        qDebug() << "ERROR: Could not open dns leases file!";
    }

    return list;
}

SelphyPrinter *SelphyPrinter::createInternal(const QString &name)
{
    Q_UNUSED(name);
    return new SelphyPrinter(name);
}
