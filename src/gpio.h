#ifndef QTBOOTH_GPIOD_WRAPPER_H
#define QTBOOTH_GPIOD_WRAPPER_H

#include <QObject>
#include <QVariantList>
#include <QString>
#include <atomic>
#include <thread>
#include <pthread.h>
#include <gpiod.h>

class Gpiod : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString chipPath READ chipPath WRITE setChipPath NOTIFY chipPathChanged)
    Q_PROPERTY(int line READ line WRITE setLine NOTIFY lineChanged)
    Q_PROPERTY(float value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(int pwmFrequency READ pwmFrequency WRITE setPwmFrequency NOTIFY pwmFrequencyChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

public:
    enum Mode {
        Output,
        PWM
    };
    Q_ENUM(Mode)

    explicit Gpiod(QObject *parent = nullptr);
    ~Gpiod();

    QString chipPath() const;
    void setChipPath(const QString &chipPath);

    int line() const;
    void setLine(int line);

    float value() const;
    void setValue(float value);

    Mode mode() const;
    void setMode(Mode mode);

    int pwmFrequency() const;
    void setPwmFrequency(int frequency);

    bool enabled() const;
    void setEnabled(bool enabled);

    Q_INVOKABLE QVariantList availableChips();
    Q_INVOKABLE QVariantList availableLines(const QString &chipPath);

signals:
    void chipPathChanged(const QString &chipPath);
    void lineChanged(int line);
    void valueChanged(float value);
    void modeChanged(Mode mode);
    void pwmFrequencyChanged(int frequency);
    void enabledChanged(bool enabled);

private:
    void openLine();
    void closeLine();
    void applyValue();
    void startPwmThread();
    void stopPwmThread();
    void pwmWorker();
    void precisionSleep(long nanoseconds);

    QString m_chipPath = "/dev/gpiochip0";
    int m_line = -1;
    float m_value = 0.0f;
    Mode m_mode = Output;
    std::atomic<int> m_pwmFrequency{1000};
    bool m_enabled = false;

    struct gpiod_chip *m_chip = nullptr;
    struct gpiod_line_request *m_request = nullptr;

    std::thread m_pwmThread;
    std::atomic<float> m_pwmDuty{0.0f};
    std::atomic<bool> m_pwmRunning{false};
};

#endif // GPIOD_H
