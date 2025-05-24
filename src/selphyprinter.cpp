#include "selphyprinter.h"
#include <QFile>
#include <QRegularExpression>>
#include <QTextStream>
#include <QDebug>

SelphyPrinter::SelphyPrinter(const QString &name, QObject *parent) : AbstractPrinter(parent), mIp("")
{
    QRegularExpression regex("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}");
    QRegularExpressionMatch match = regex.match(name);
    if(match.hasMatch())
    {
        mIp = match.capturedTexts().first();
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

int SelphyPrinter::printImage(const QString &filename, int copyCount)
{
    if(mIp.length() > 0)
    {
        QString imageMagickCommand;
        if(filename.endsWith(".jpg") || filename.endsWith(".jpg"))
        {
            imageMagickCommand = ":"; // file is already a JPG. No conversion to be done
        }
        else
        {
            // file is not a JPG. First convert to JPG.
            imageMagickCommand = "convert " + filename + " -quality 100% " + filename + ".jpg && rm " + filename;
        }

        QString selphyCommand = "selphy -printer_ip=" + mIp + " " + filename + ".jpg";
        QString printCommand = imageMagickCommand;
        for(int i = 0; i < copyCount; i++)
        {
            printCommand = printCommand + " && " + selphyCommand;
        }
        QStringList shParameters;
        shParameters << "-c";
        shParameters << printCommand;

        if(mPrinterProcess.state() == QProcess::NotRunning)
        {
            if(printerOnline())
            {
                emit busyChanged(true);
                mPrinterProcess.start("sh", shParameters);
                qDebug() << "Runnings sh with parameters: " << shParameters;
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
                QRegularExpression regex("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}");
                QRegularExpressionMatch match = regex.match(line);
                if(match.hasMatch())
                {
                    QString ip = match.capturedTexts().first();
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
