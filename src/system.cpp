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
    QDateTime time(date);

    time_t t = time.toTime_t();
    return (stime(&t) == 0); //return zero on success
}
