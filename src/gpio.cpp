#include "gpio.h"
#include <QFile>
#include <cmath>
#include <QDebug>
#include "assert.h"

#define IO_DEVICE "/dev/pi-blaster"

bool cmpf(float A, float B, float epsilon = 0.005f)
{
    return (fabs(A - B) < epsilon);
}

template<class T>
constexpr const T& clamp( const T& v, const T& lo, const T& hi )
{
    assert( !(hi < lo) );
    return (v < lo) ? lo : (hi < v) ? hi : v;
}

GPIO::GPIO(QObject *parent) : QObject(parent), pinValue(0.0f)
{

}

GPIO::~GPIO()
{
    if(pinNumber >= 0)
    {
        QFile io(IO_DEVICE);
        if(io.open(QFile::WriteOnly | QIODevice::Text))
        {
            QString io_string = QString("release ") + QString::number(pinNumber);
            io.write(io_string.toLatin1());
            io.flush();
            io.close();
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

        qDebug() << "Allocated pin: " << pinNumber;
    }
}

float GPIO::value() const
{
    return pinValue;
}

void GPIO::setValue(float value)
{
    float _value = clamp<float>(value, 0.0, 1.0);

    if(!cmpf(_value,pinValue))
    {
        pinValue = _value;
        valueChanged(pinValue);
    }

    if(pinNumber >= 0)
    {
        QFile io(IO_DEVICE);
        if(io.open(QFile::WriteOnly | QIODevice::Text))
        {
            QString io_string = QString::number(pinNumber) + QString("=") + QString::number(pinValue);
            io.write(io_string.toLatin1());
            io.flush();
            io.close();
        }
        else
        {
            qDebug() << "Could not access pi-blaster";
        }

        qDebug() << "Pin: " << pinNumber << " is set to " << pinValue;
    }
}
