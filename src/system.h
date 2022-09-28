#ifndef SYSTEM_H
#define SYSTEM_H

#include <QObject>
#include <QDate>

class System : public QObject
{
    Q_OBJECT
public:
    System();
    Q_INVOKABLE void shutdown();
    Q_INVOKABLE void restart();
    Q_INVOKABLE bool setTime(QDateTime date);
    Q_INVOKABLE QString getGitHash() const;
};

#endif
