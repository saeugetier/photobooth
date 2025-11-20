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

QString System::getGitHash() const
{
    if(QString(QT_STRINGIFY(GIT_CURRENT_TAG)).length() > 0)
    {
        return QString(QT_STRINGIFY(GIT_CURRENT_TAG)) + " (" + QString(QT_STRINGIFY(GIT_CURRENT_SHA1)) + ")";
    }
    return QString(QT_STRINGIFY(GIT_CURRENT_SHA1));
}
