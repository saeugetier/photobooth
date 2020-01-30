#ifndef SYSTEM_H
#define SYSTEM_H

#include <QObject>

class System : public QObject
{
    Q_OBJECT
public:
    System();
    Q_INVOKABLE void shutdown();
    Q_INVOKABLE void restart();
};

#endif // SYSTEM_H
