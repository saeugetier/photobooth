#ifndef LEDFLASH_H
#define LEDFLASH_H

#include <QObject>

class LedFlash : public QObject
{
    Q_OBJECT
public:
    explicit LedFlash(QObject *parent = nullptr);
signals:

public slots:
    Q_INVOKABLE void setBrightness(float brightness);
    Q_INVOKABLE void setFlash(bool flash);
protected:
    float m_brightness;
};

#endif // LEDFLASH_H
