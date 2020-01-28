#include "gpio.h"
#include <QFile>
#include <algorithm>
#include <cmath>

#define IO_DEVICE "/dev/pi-blaster"

bool cmpf(float A, float B, float epsilon = 0.005f)
{
    return (fabs(A - B) < epsilon);
}

GPIO::GPIO(QObject *parent) : QObject(parent), pinValue(0.0f)
{

}

GPIO::~GPIO()
{
    if(pinNumber >= 0)
    {
        QFile io(IO_DEVICE);
        if(io.open(QFile::WriteOnly))
        {
            QString io_string = QString("release ") + QString::number(pinNumber);
            io.write(io_string.toLatin1());
        }
    }
}

int GPIO::pin() const
{
    return pinNumber;
}

void GPIO::setPin(int pin)
{
    if(pin >= 0 && pin <= 26)
    {
        if(pin != pinNumber)
        {
            pinNumber = pin;
            emit pinChanged(pin);
        }
    }
}

float GPIO::value() const
{
    return pinValue;
}

void GPIO::setValue(float value)
{
    float _value = std::clamp<float>(value, 0.0, 1.0);

    if(!cmpf(_value,pinValue))
    {
        pinValue = _value;
        valueChanged(pinValue);
    }

    if(pinNumber >= 0)
    {
        QFile io(IO_DEVICE);
        if(io.open(QFile::WriteOnly))
        {
            QString io_string = QString::number(pinNumber) + QString("=") + QString::number(pinValue);
            io.write(io_string.toLatin1());
        }
    }
}
