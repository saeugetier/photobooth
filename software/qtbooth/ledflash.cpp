#include "ledflash.h"
#include <QProcess>
#include <QDebug>
#ifdef WIRINGPI
extern "C"
{
    #include <wiringPi.h>
    #pragma message ("Using wiringpi")
}
#else
    #pragma message ("Not using wiringpi on x86")
#endif

#define PWM_PIN 1
#define ENABLE_PIN 4
#define TRIGGER_PIN 6

LedFlash::LedFlash(QObject *parent) : QObject(parent), m_brightness(0)
{
    qDebug() << "Initializing LED flash";
#ifdef WIRINGPI
    wiringPiSetup () ;
    pwmSetClock(4);
    pwmSetRange(1024);
    pwmSetMode(PWM_MODE_MS);
    pinMode(ENABLE_PIN, OUTPUT);
    pinMode(TRIGGER_PIN, OUTPUT);
    pinMode(PWM_PIN, PWM_OUTPUT);
#else

#endif
    QObject::connect(&mTriggerTimer, SIGNAL(timeout()), this, SLOT(releaseTriggerFocus()));
}

LedFlash* LedFlash::createInstance()
{
    return new LedFlash();
}


LedFlash* LedFlash::instance()
{
    return Singleton<LedFlash>::instance(LedFlash::createInstance);
}


void LedFlash::setBrightness(float brightness)
{
    if(brightness < 0.0f)
        brightness = 0.0f;
    if(brightness > 1.0f)
        brightness = 1.0f;

    if(brightness == 0.0f)
    {
#ifdef WIRINGPI
        digitalWrite(ENABLE_PIN, 0);
#else
        qDebug() << "LED off";
#endif
    }
    else
    {
#ifdef WIRINGPI
        int value = 1023 - brightness * 1023;
        pwmWrite(PWM_PIN, value);
        digitalWrite(ENABLE_PIN, 1);
#else
        qDebug() << "LED brightness set to " << brightness;
#endif
    }

    m_brightness = brightness;
}

void LedFlash::setFlash(bool flash)
{
    if(flash)
    {
#ifdef WIRINGPI
        pwmWrite(PWM_PIN, 0);
        digitalWrite(ENABLE_PIN, 1);
#else
        qDebug() << "Flash on";
#endif
    }
    else
    {
        setBrightness(m_brightness);
    }
}

void LedFlash::triggerFocus()
{
#ifdef WIRINGPI
    digitalWrite(TRIGGER_PIN, 1);  //maybe a delay is needed?

#endif
    mTriggerTimer.start(500);
    qDebug() << "Triggered focus";
}

void LedFlash::releaseTriggerFocus()
{
#ifdef WIRINGPI
    digitalWrite(TRIGGER_PIN, 0);
#endif
    mTriggerTimer.stop();
    qDebug() << "Released trigger of focus";
}

LedFlash::~LedFlash()
{
    setBrightness(0);
}
