#include "gpio.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QVariantMap>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <cerrno>
#include <cstring>
#include <time.h>
#include <pthread.h>
#include <sched.h>
#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#endif

Gpiod::Gpiod(QObject *parent)
    : QObject(parent)
{
}

Gpiod::~Gpiod()
{
    stopPwmThread();
    closeLine();
}

QString Gpiod::chipPath() const
{
    return m_chipPath;
}

void Gpiod::setChipPath(const QString &chipPath)
{
    if (m_chipPath != chipPath) {
        closeLine();
        m_chipPath = chipPath;
        emit chipPathChanged(m_chipPath);
        if (m_enabled && m_line >= 0) {
            openLine();
            applyValue();
        }
    }
}

int Gpiod::line() const
{
    return m_line;
}

void Gpiod::setLine(int line)
{
    if (m_line != line) {
        closeLine();
        m_line = line;
        emit lineChanged(m_line);
        if (m_enabled && m_line >= 0) {
            openLine();
            applyValue();
        }
    }
}

float Gpiod::value() const
{
    return m_value;
}

void Gpiod::setValue(float value)
{
    float clamped = std::clamp(value, 0.0f, 1.0f);
    if (std::fabs(clamped - m_value) > 0.005f || (clamped == 0.0f) != (m_value == 0.0f) || (clamped == 1.0f) != (m_value == 1.0f)) {
        m_value = clamped;
        emit valueChanged(m_value);
        applyValue();
    }
}

Gpiod::Mode Gpiod::mode() const
{
    return m_mode;
}

void Gpiod::setMode(Mode mode)
{
    if (m_mode != mode) {
        stopPwmThread();
        m_mode = mode;
        emit modeChanged(m_mode);
        applyValue();
    }
}

int Gpiod::pwmFrequency() const
{
    return m_pwmFrequency.load();
}

void Gpiod::setPwmFrequency(int frequency)
{
    if (frequency < 1)
        frequency = 1;
    if (frequency > 10000)
        frequency = 10000;
    if (m_pwmFrequency.load() != frequency) {
        m_pwmFrequency.store(frequency);
        emit pwmFrequencyChanged(frequency);
    }
}

bool Gpiod::enabled() const
{
    return m_enabled;
}

void Gpiod::setEnabled(bool enabled)
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

QVariantList Gpiod::availableChips()
{
    QVariantList chips;
    QDir devDir("/dev");
    QStringList filters;
    filters << "gpiochip*";
    QStringList entries = devDir.entryList(filters, QDir::System, QDir::Name);

    qDebug() << "GPIO: Scanning /dev for gpiochip* entries, found" << entries.size() << "candidates";
    if (entries.isEmpty())
        qWarning() << "GPIO: No gpiochip devices found in /dev";

    for (const QString &entry : entries) {
        QString path = "/dev/" + entry;
        QFileInfo fileInfo(path);

        qDebug() << "GPIO: Candidate" << path
                 << "exists=" << fileInfo.exists()
                 << "readable=" << fileInfo.isReadable()
                 << "writable=" << fileInfo.isWritable();

        if (!fileInfo.isReadable() || !fileInfo.isWritable()) {
            qWarning() << "GPIO: Permission warning for" << path
                       << "(readable=" << fileInfo.isReadable()
                       << ", writable=" << fileInfo.isWritable() << ")";
        }

        errno = 0;
        struct gpiod_chip *chip = gpiod_chip_open(path.toUtf8().constData());
        if (!chip) {
            qWarning() << "GPIO: Failed to open chip" << path
                       << "errno=" << errno
                       << "(" << QString::fromLocal8Bit(std::strerror(errno)) << ")";
            continue;
        }

        qDebug() << "GPIO: Successfully opened chip interface" << path;

        QString label;
        errno = 0;
        struct gpiod_chip_info *info = gpiod_chip_get_info(chip);
        if (info) {
            const char *chipName = gpiod_chip_info_get_name(info);
            const char *chipLabel = gpiod_chip_info_get_label(info);
            const size_t numLines = gpiod_chip_info_get_num_lines(info);

            if (chipLabel)
                label = QString::fromUtf8(chipLabel);

            qDebug() << "GPIO: Chip info read ok for" << path
                     << "name=" << (chipName ? chipName : "")
                     << "label=" << (chipLabel ? chipLabel : "")
                     << "lines=" << static_cast<qulonglong>(numLines);

            gpiod_chip_info_free(info);
        } else {
            qWarning() << "GPIO: Failed to read chip info for" << path
                       << "errno=" << errno
                       << "(" << QString::fromLocal8Bit(std::strerror(errno)) << ")";
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

    if (chips.isEmpty()) {
        if (!entries.isEmpty()) {
            qWarning() << "GPIO: gpiochip devices were found in /dev but none could be opened/read. Check permissions and kernel GPIO support.";
        }
        QVariantMap item;
        item["value"] = "";
        item["text"] = "(no GPIO chips found)";
        chips.append(item);
    }

    return chips;
}

QVariantList Gpiod::availableLines(const QString &chipPath)
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

void Gpiod::openLine()
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

void Gpiod::closeLine()
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

void Gpiod::applyValue()
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

void Gpiod::startPwmThread()
{
    if (m_pwmRunning.load(std::memory_order_acquire))
        return;

    m_pwmRunning.store(true, std::memory_order_release);
    m_pwmThread = std::thread(&Gpiod::pwmWorker, this);
    
    // Set high priority for PWM thread to reduce jitter
    int policy = SCHED_FIFO;
    struct sched_param param;
    param.sched_priority = sched_get_priority_max(policy) - 1;
    pthread_setschedparam(m_pwmThread.native_handle(), policy, &param);
    
    const int frequency = m_pwmFrequency.load(std::memory_order_relaxed);
    qDebug() << "GPIO: PWM thread started for line" << m_line << "at" << frequency << "Hz";
}

void Gpiod::stopPwmThread()
{
    if (m_pwmRunning.load(std::memory_order_acquire)) {
        m_pwmRunning.store(false, std::memory_order_release);
        if (m_pwmThread.joinable())
            m_pwmThread.join();
    }
}

void Gpiod::pwmWorker()
{
    using namespace std::chrono;
    const unsigned int lineOffset = static_cast<unsigned int>(m_line);

    while (m_pwmRunning.load(std::memory_order_acquire)) {
        // Cache frequency and duty once per cycle to reduce jitter
        const int frequency = m_pwmFrequency.load(std::memory_order_relaxed);
        float duty = m_pwmDuty.load(std::memory_order_relaxed);
        duty = std::clamp(duty, 0.001f, 0.999f);

        // Pre-calculate timing for the entire period
        const long periodNs = 1000000000L / frequency;
        const long onTimeNs = static_cast<long>(periodNs * duty);
        const long offTimeNs = periodNs - onTimeNs;

        // HIGH phase with high-precision timing
        if (m_request)
            gpiod_line_request_set_value(m_request, lineOffset, GPIOD_LINE_VALUE_ACTIVE);

        precisionSleep(onTimeNs);

        if (!m_pwmRunning.load(std::memory_order_acquire))
            break;

        // LOW phase with high-precision timing
        if (m_request)
            gpiod_line_request_set_value(m_request, lineOffset, GPIOD_LINE_VALUE_INACTIVE);

        precisionSleep(offTimeNs);
    }

    // Ensure line is LOW when thread exits
    if (m_request)
        gpiod_line_request_set_value(m_request, lineOffset, GPIOD_LINE_VALUE_INACTIVE);
}

void Gpiod::precisionSleep(long nanoseconds)
{
    using namespace std::chrono;
    const long BUSY_WAIT_THRESHOLD_NS = 50000; // Only busy-wait for < 50µs
    
    if (nanoseconds <= 0)
        return;
    
    if (nanoseconds > BUSY_WAIT_THRESHOLD_NS) {
        // For longer durations, use clock_nanosleep (more predictable than nanosleep)
        // Leave a small margin for busy-wait to catch up any undersleep
        long sleepNs = nanoseconds - BUSY_WAIT_THRESHOLD_NS;
        struct timespec ts;
        ts.tv_sec = sleepNs / 1000000000L;
        ts.tv_nsec = sleepNs % 1000000000L;
        
        // Use CLOCK_MONOTONIC for predictability (not affected by NTP adjustments)
        clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, nullptr);
    }
    
    // Busy-wait for final precision, but with CPU pause instructions
    // This is much more efficient than tight spin-loop
    auto deadline = high_resolution_clock::now() + std::chrono::nanoseconds(nanoseconds);
    while (high_resolution_clock::now() < deadline) {
        // Pause instruction reduces CPU power usage and helps hyperthreading
        #if defined(__x86_64__) || defined(__i386__)
            __builtin_ia32_pause();  // x86/x64: ~40 cycles per pause
        #elif defined(__arm__) || defined(__aarch64__)
            __asm__ __volatile__("yield");  // ARM: yield to other threads
        #else
            __asm__ __volatile__("" ::: "memory");  // Prevent loop optimization
        #endif
    }
}
