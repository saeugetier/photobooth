#include "system.h"
#include <QFile>
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

bool System::supportsRknn() const
{
    QFile compatibleFile("/proc/device-tree/compatible");
    if (compatibleFile.open(QIODevice::ReadOnly))
    {
        const QByteArray compatible = compatibleFile.readAll().toLower();
        if (compatible.contains("rk3566"))
        {
            return true;
        }
    }
    else
    {
        qWarning() << "Failed to open /proc/device-tree/compatible";
    }

    QFile modelFile("/proc/device-tree/model");
    if (modelFile.open(QIODevice::ReadOnly))
    {
        const QByteArray model = modelFile.readAll().toLower();
        if (model.contains("rk3566"))
        {
            return true;
        }
    }
    else
    {
        qWarning() << "Failed to open /proc/device-tree/model";
    }

    return false;
}
