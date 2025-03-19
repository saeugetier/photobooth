#include "system.h"
#include <QProcess>
#include <QDebug>

System::System()
{

}

void System::shutdown()
{
    QProcess process;
    process.startDetached("shutdown now -P -h");
}

void System::restart()
{
    QProcess process;
    process.startDetached("reboot");
}

bool System::setTime(QDateTime date)
{
    int result = -1;

    qDebug() << "Set Time " << date.toString("yyyy-MM-dd hh::mm::ss");
#ifdef __linux__
    QProcess process;
    QStringList arguments;
    arguments.append("-s");
    arguments.append(date.toString("yyyy-MM-dd hh:mm:ss"));
    process.start("date", arguments);
    process.waitForFinished();
    result = process.exitCode();
    if(result != 0)
    {
        qDebug() << "Setting time failed. Exit code: " << result;
        qDebug() << process.readAllStandardOutput() << "\n" << process.readAllStandardError() << "\n" << process.errorString();
    }
    else
    {
        process.start("hwclock -w");
        process.waitForFinished();
        result = process.exitCode();
        if(result != 0)
        {
            qDebug() << "Writing clock to hwclock failed with exit code:" << result;
            qDebug() << process.readAllStandardOutput() << "\n" << process.readAllStandardError() << "\n" << process.errorString();
        }
    }
#elif __APPLE__
#pragma message ( "setting the time is not implemented for MacOS" )
#elif _WIN32
#pragma message ( "setting the time is not implemented for Windows" )
#endif
    return (result == 0);
}

QString System::getGitHash() const
{
    return QString(QT_STRINGIFY(GIT_CURRENT_SHA1));
}
