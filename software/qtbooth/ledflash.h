#ifndef LEDFLASH_H
#define LEDFLASH_H

#include <QObject>
#include <QTimer>
#include "singleton.h"

class LedFlash : public QObject
{
    Q_OBJECT
private:
    LedFlash(QObject *parent = nullptr);
    static LedFlash* createInstance();
public:
    static LedFlash* instance();
    ~LedFlash();
public slots:
    Q_INVOKABLE void setBrightness(float brightness);
    Q_INVOKABLE void setFlash(bool flash);
    Q_INVOKABLE void triggerFocus();
private slots:
    void releaseTriggerFocus();
protected:
    float m_brightness;

    QTimer mTriggerTimer;
};

#endif // LEDFLASH_H
