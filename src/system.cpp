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
    QDateTime time(date);
    struct timespec stime;
    time_t t = time.toTime_t();
    stime.tv_sec = t;

    qDebug() << "Set Time " << date << " -- "  << t;
#ifdef __linux__
    result = clock_settime(CLOCK_REALTIME, &stime); //return zero on success
    if(result != 0)
    {
        qDebug() << "Setting time failed with error code: " << strerror(errno) << " (" << errno << ")";
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
