#include "ledflash.h"
#include <QProcess>
#include <QDebug>
#ifdef WIRINGPI
    #include <wiringPi.h>
    #pragma message ("Using wiringpi")
#else
    #pragma message ("Not using wiringpi on x86")
#endif

#define PWM_PIN 4
#define ENABLE_PIN 1

LedFlash::LedFlash(QObject *parent) : QObject(parent), m_brightness(0)
{
    qDebug() << "Initializing LED flash";
#ifdef WIRINGPI
    wiringPiSetup () ;
    pwmSetClock(4);
    pwmSetRange(1024);
    pwmSetMode(PWM_MODE_MS);
    pinMode(ENABLE_PIN, OUTPUT);
    pinMode(PWM_PIN, PWM_OUTPUT);
#else

#endif
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
        QProcess::execute(GPIO_COMMAND, {GPIO_ENABLE_PIN_OUTPUT + "0"});
    }
    else
    {
        int value = brightness * 1023;
        QProcess::execute(GPIO_COMMAND, {GPIO_PWM_PIN_OUTPUT + QString(value)});
        QProcess::execute(GPIO_COMMAND, {GPIO_ENABLE_PIN_OUTPUT + "1"});
    }

    m_brightness = brightness;
}

void LedFlash::setFlash(bool flash)
{
    if(flash)
    {
        QProcess::execute(GPIO_COMMAND, {GPIO_PWM_PIN_OUTPUT + "1023"});
        QProcess::execute(GPIO_COMMAND, {GPIO_ENABLE_PIN_OUTPUT + "1"});
    }
    else
    {
        setBrightness(m_brightness);
    }
}
