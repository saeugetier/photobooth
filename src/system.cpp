#include "system.h"
#include <QProcess>

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

bool System::setTime(QDate date)
{
    int result = -1;
    QDateTime time(date);
    time_t t = time.toTime_t();
#ifdef __linux__
    result = stime(&t); //return zero on success
#elif __APPLE__
    #pragma message ( "setting the time is not implemented for MacOS" )
#elif _WIN32
    #pragma message ( "setting the time is not implemented for Windows" )
#endif
    return (result == 0);
}
