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
#endif
    return (result == 0);
}
