#include "gpio.h"
#include <QDebug>
#include <QDir>
#include <QVariantMap>
#include <cmath>
#include <algorithm>
#include <chrono>

GPIO::GPIO(QObject *parent)
    : QObject(parent)
{
}

GPIO::~GPIO()
{
    stopPwmThread();
    closeLine();
}

QString GPIO::chipPath() const
{
    return m_chipPath;
}

void GPIO::setChipPath(const QString &chipPath)
{
    if (m_chipPath != chipPath) {
        closeLine();
        m_chipPath = chipPath;
        emit chipPathChanged(m_chipPath);
        if (m_enabled && m_line >= 0)
            openLine();
    }
}

int GPIO::line() const
{
    return m_line;
}

void GPIO::setLine(int line)
{
    if (m_line != line) {
        closeLine();
        m_line = line;
        emit lineChanged(m_line);
        if (m_enabled && m_line >= 0)
            openLine();
    }
}

float GPIO::value() const
{
    return m_value;
}

void GPIO::setValue(float value)
{
    float clamped = std::clamp(value, 0.0f, 1.0f);
    if (std::fabs(clamped - m_value) > 0.005f || (clamped == 0.0f) != (m_value == 0.0f) || (clamped == 1.0f) != (m_value == 1.0f)) {
        m_value = clamped;
        emit valueChanged(m_value);
        applyValue();
    }
}

GPIO::Mode GPIO::mode() const
{
    return m_mode;
}

void GPIO::setMode(Mode mode)
{
    if (m_mode != mode) {
        stopPwmThread();
        m_mode = mode;
        emit modeChanged(m_mode);
        applyValue();
    }
}

int GPIO::pwmFrequency() const
{
    return m_pwmFrequency;
}

void GPIO::setPwmFrequency(int frequency)
{
    if (frequency < 1)
        frequency = 1;
    if (m_pwmFrequency != frequency) {
        m_pwmFrequency = frequency;
        emit pwmFrequencyChanged(m_pwmFrequency);
    }
}

bool GPIO::enabled() const
{
    return m_enabled;
}

void GPIO::setEnabled(bool enabled)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;
        emit enabledChanged(m_enabled);
        if (m_enabled && m_line >= 0) {
            openLine();
            applyValue();
        } else {
            stopPwmThread();
            closeLine();
        }
    }
}

QVariantList GPIO::availableChips()
{
    QVariantList chips;
    QDir devDir("/dev");
    QStringList filters;
    filters << "gpiochip*";
    QStringList entries = devDir.entryList(filters, QDir::System, QDir::Name);

    for (const QString &entry : entries) {
        QString path = "/dev/" + entry;
        struct gpiod_chip *chip = gpiod_chip_open(path.toUtf8().constData());
        if (chip) {
            QString label;
            struct gpiod_chip_info *info = gpiod_chip_get_info(chip);
            if (info) {
                label = QString::fromUtf8(gpiod_chip_info_get_label(info));
                gpiod_chip_info_free(info);
            }
            QVariantMap item;
            item["value"] = path;
            if (label.isEmpty())
                item["text"] = entry;
            else
                item["text"] = entry + " (" + label + ")";
            chips.append(item);
            gpiod_chip_close(chip);
        }
    }

    if (chips.isEmpty()) {
        QVariantMap item;
        item["value"] = "";
        item["text"] = "(no GPIO chips found)";
        chips.append(item);
    }

    return chips;
}

QVariantList GPIO::availableLines(const QString &chipPath)
{
    QVariantList lines;

    if (chipPath.isEmpty())
        return lines;

    struct gpiod_chip *chip = gpiod_chip_open(chipPath.toUtf8().constData());
    if (!chip) {
        qWarning() << "GPIO: Cannot open chip" << chipPath << "for line enumeration";
        return lines;
    }

    size_t numLines = 0;
    struct gpiod_chip_info *chipInfo = gpiod_chip_get_info(chip);
    if (chipInfo) {
        numLines = gpiod_chip_info_get_num_lines(chipInfo);
        gpiod_chip_info_free(chipInfo);
    }
    for (size_t i = 0; i < numLines; ++i) {
        struct gpiod_line_info *info = gpiod_chip_get_line_info(chip, i);
        if (!info)
            continue;

        QString name = QString::fromUtf8(gpiod_line_info_get_name(info));
        bool inUse = gpiod_line_info_is_used(info);

        QVariantMap item;
        item["value"] = static_cast<int>(i);
        QString label = QString::number(i);
        if (!name.isEmpty())
            label += " — " + name;
        if (inUse)
            label += " [in use]";
        item["text"] = label;
        lines.append(item);

        gpiod_line_info_free(info);
    }

    gpiod_chip_close(chip);
    return lines;
}

void GPIO::openLine()
{
    closeLine();

    if (m_chipPath.isEmpty() || m_line < 0)
        return;

    m_chip = gpiod_chip_open(m_chipPath.toUtf8().constData());
    if (!m_chip) {
        qWarning() << "GPIO: Cannot open chip" << m_chipPath;
        return;
    }

    struct gpiod_line_settings *lineSettings = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(lineSettings, GPIOD_LINE_DIRECTION_OUTPUT);
    gpiod_line_settings_set_output_value(lineSettings, GPIOD_LINE_VALUE_INACTIVE);

    struct gpiod_line_config *lineConfig = gpiod_line_config_new();
    unsigned int offset = static_cast<unsigned int>(m_line);
    gpiod_line_config_add_line_settings(lineConfig, &offset, 1, lineSettings);

    struct gpiod_request_config *reqConfig = gpiod_request_config_new();
    gpiod_request_config_set_consumer(reqConfig, "qtbooth");

    m_request = gpiod_chip_request_lines(m_chip, reqConfig, lineConfig);

    gpiod_request_config_free(reqConfig);
    gpiod_line_config_free(lineConfig);
    gpiod_line_settings_free(lineSettings);

    if (!m_request) {
        qWarning() << "GPIO: Cannot request line" << m_line << "on" << m_chipPath;
        gpiod_chip_close(m_chip);
        m_chip = nullptr;
        return;
    }

    qDebug() << "GPIO: Opened line" << m_line << "on" << m_chipPath;
}

void GPIO::closeLine()
{
    stopPwmThread();

    if (m_request) {
        gpiod_line_request_set_value(m_request, static_cast<unsigned int>(m_line), GPIOD_LINE_VALUE_INACTIVE);
        gpiod_line_request_release(m_request);
        m_request = nullptr;
    }
    if (m_chip) {
        gpiod_chip_close(m_chip);
        m_chip = nullptr;
    }
}

void GPIO::applyValue()
{
    if (!m_enabled || !m_request)
        return;

    if (m_mode == Output) {
        stopPwmThread();
        enum gpiod_line_value val = (m_value >= 0.5f) ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE;
        gpiod_line_request_set_value(m_request, static_cast<unsigned int>(m_line), val);
        qDebug() << "GPIO: Line" << m_line << "set to" << (val == GPIOD_LINE_VALUE_ACTIVE ? "HIGH" : "LOW");
    } else {
        // PWM mode
        m_pwmDuty.store(m_value, std::memory_order_relaxed);

        if (m_value <= 0.0f) {
            stopPwmThread();
            gpiod_line_request_set_value(m_request, static_cast<unsigned int>(m_line), GPIOD_LINE_VALUE_INACTIVE);
        } else if (m_value >= 1.0f) {
            stopPwmThread();
            gpiod_line_request_set_value(m_request, static_cast<unsigned int>(m_line), GPIOD_LINE_VALUE_ACTIVE);
        } else {
            if (!m_pwmRunning.load(std::memory_order_acquire)) {
                startPwmThread();
            }
        }
    }
}

void GPIO::startPwmThread()
{
    if (m_pwmRunning.load(std::memory_order_acquire))
        return;

    m_pwmRunning.store(true, std::memory_order_release);
    m_pwmThread = std::thread(&GPIO::pwmWorker, this);
    qDebug() << "GPIO: PWM thread started for line" << m_line << "at" << m_pwmFrequency << "Hz";
}

void GPIO::stopPwmThread()
{
    if (m_pwmRunning.load(std::memory_order_acquire)) {
        m_pwmRunning.store(false, std::memory_order_release);
        if (m_pwmThread.joinable())
            m_pwmThread.join();
    }
}

void GPIO::pwmWorker()
{
    const unsigned int lineOffset = static_cast<unsigned int>(m_line);

    while (m_pwmRunning.load(std::memory_order_acquire)) {
        float duty = m_pwmDuty.load(std::memory_order_relaxed);

        // Clamp duty to valid PWM range (edge values handled in applyValue)
        duty = std::clamp(duty, 0.001f, 0.999f);

        // Calculate period in nanoseconds
        long periodNs = 1000000000L / m_pwmFrequency;
        long onTimeNs = static_cast<long>(periodNs * duty);
        long offTimeNs = periodNs - onTimeNs;

        // HIGH phase
        if (m_request)
            gpiod_line_request_set_value(m_request, lineOffset, GPIOD_LINE_VALUE_ACTIVE);

        struct timespec onSleep;
        onSleep.tv_sec = onTimeNs / 1000000000L;
        onSleep.tv_nsec = onTimeNs % 1000000000L;
        nanosleep(&onSleep, nullptr);

        if (!m_pwmRunning.load(std::memory_order_acquire))
            break;

        // LOW phase
        if (m_request)
            gpiod_line_request_set_value(m_request, lineOffset, GPIOD_LINE_VALUE_INACTIVE);

        struct timespec offSleep;
        offSleep.tv_sec = offTimeNs / 1000000000L;
        offSleep.tv_nsec = offTimeNs % 1000000000L;
        nanosleep(&offSleep, nullptr);
    }

    // Ensure line is LOW when thread exits
    if (m_request)
        gpiod_line_request_set_value(m_request, lineOffset, GPIOD_LINE_VALUE_INACTIVE);
}
