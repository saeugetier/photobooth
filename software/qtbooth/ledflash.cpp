#include "ledflash.h"
#include <QProcess>

#define GPIO_COMMAND              QString("gpio")

#define GPIO_PWM_CLOCK_INIT       QString("pwmc 4")
#define GPIO_PWM_RANGE_INIT       QString("pwmr 1024")
#define GPIO_PWM_MODE_INIT        QString("pwm-ms")

#define GPIO_PWM_PIN_MODE         QString("mode 1 pwm")

#define GPIO_ENABLE_PIN_MODE      QString("mode 4 out")

#define GPIO_PWM_PIN_OUTPUT       QString("pwm 1 ")
#define GPIO_ENABLE_PIN_OUTPUT    QString("write 4 ")

LedFlash::LedFlash(QObject *parent) : QObject(parent), m_brightness(0)
{
    QProcess::execute(GPIO_COMMAND, {GPIO_PWM_CLOCK_INIT });
    QProcess::execute(GPIO_COMMAND, {GPIO_PWM_RANGE_INIT });
    QProcess::execute(GPIO_COMMAND, {GPIO_PWM_MODE_INIT });

    QProcess::execute(GPIO_COMMAND, {GPIO_PWM_PIN_MODE });
    QProcess::execute(GPIO_COMMAND, {GPIO_ENABLE_PIN_MODE });

    QProcess::execute(GPIO_COMMAND, {GPIO_ENABLE_PIN_OUTPUT + "0"});
    QProcess::execute(GPIO_COMMAND, {GPIO_PWM_PIN_OUTPUT + "1023"});
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
