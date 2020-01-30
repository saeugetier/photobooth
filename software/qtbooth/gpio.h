#ifndef GPIO_H
#define GPIO_H

#include <QObject>

class GPIO : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int pin READ pin WRITE setPin NOTIFY pinChanged)
    Q_PROPERTY(float value READ value WRITE setValue NOTIFY valueChanged)
public:
    explicit GPIO(QObject *parent = nullptr);
    ~GPIO();
    int pin() const;
    void setPin(int pin);
    float value() const;
    void setValue(float value);
signals:
    void pinChanged(int);
    void valueChanged(float);
protected:
    int pinNumber = -1; //pin is not set
    float pinValue = 0.0;
signals:

};

#endif // GPIO_H
