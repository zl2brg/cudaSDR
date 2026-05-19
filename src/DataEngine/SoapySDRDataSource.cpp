#include "SoapySDRDataSource.h"

#ifdef HAVE_SOAPYSDR

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#include <QDebug>
#include <algorithm>

SoapySDRDataSource::SoapySDRDataSource(THPSDRParameter *ioData)
    : QObject(nullptr)
    , set(Settings::instance())
    , io(ioData)
    , m_device(nullptr)
    , m_rxStream(nullptr)
    , m_stopped(false)
    , m_sampleRate(set->getSampleRate())
    , m_rfSampleRate(set->getSampleRate())
    , m_decimRatio(1)
    , m_minSampleRate(0)
    , m_numChannels(1)
    , m_minFrequency(0)
    , m_maxFrequency(0)
    , m_pendingFreq(0.0)
    , m_freqPending(false)
    , m_pendingRfSampleRate(0)
    , m_pendingDecimRatio(1)
    , m_sampleRatePending(false)
    , m_lastKnownVfo(0)
{
}

SoapySDRDataSource::~SoapySDRDataSource() {
    stop();
}

void SoapySDRDataSource::init() {
    try {
        SoapySDR::Kwargs args;
        TSoapyDevice selected = set->getCurrentSoapyDevice();
        
        if (!selected.driver.isEmpty()) {
            args["driver"] = selected.driver.toStdString();
            if (!selected.serial.isEmpty()) {
                args["serial"] = selected.serial.toStdString();
            }
            qDebug() << "SoapySDRDataSource: Opening device" << selected.label;
            m_device = SoapySDR::Device::make(args);
        } else {
            auto results = SoapySDR::Device::enumerate();
            if (results.empty()) {
                QString msg = "SoapySDRDataSource: No devices found!";
                qCritical() << msg;
                emit messageEvent(msg);
                return;
            }
            m_device = SoapySDR::Device::make(results[0]);
        }

        if (!m_device) {
            QString msg = "SoapySDRDataSource: Failed to create device!";
            qCritical() << msg;
            emit messageEvent(msg);
            return;
        }

        qDebug() << "SoapySDRDataSource: Using device" << QString::fromStdString(m_device->getHardwareKey());

        // Query device frequency range before any hardware configuration so we
        // know the valid range even if subsequent calls throw.
        try {
            SoapySDR::RangeList ranges = m_device->getFrequencyRange(SOAPY_SDR_RX, 0);
            if (!ranges.empty()) {
                m_minFrequency = static_cast<long>(ranges.front().minimum());
                m_maxFrequency = static_cast<long>(ranges.back().maximum());
                qDebug() << "SoapySDRDataSource: Device freq range"
                         << m_minFrequency / 1.0e6 << "to" << m_maxFrequency / 1.0e6 << "MHz";
                set->setMaxFrequency(m_maxFrequency);
            }
        } catch (const std::exception &e) {
            qDebug() << "SoapySDRDataSource: Could not query frequency range:" << e.what();
        }

        // Choose an RF sample rate that is an exact integer multiple of the DSP
        // rate so that averaging m_decimRatio hardware samples produces exactly
        // one sample at m_sampleRate for WDSP.
        m_sampleRate    = set->getSampleRate();
        m_rfSampleRate  = m_sampleRate;
        m_decimRatio    = 1;
        m_minSampleRate = 0;

        // Step 1: check whether device advertises discrete rates that include ours.
        bool exactRateMatch = false;
        try {
            std::vector<double> discrete = m_device->listSampleRates(SOAPY_SDR_RX, 0);
            for (double r : discrete) {
                if (std::abs(r - m_sampleRate) < 1.0) { exactRateMatch = true; break; }
            }
            if (exactRateMatch)
                qDebug() << "SoapySDRDataSource: device natively supports DSP rate" << m_sampleRate << "Hz";
        } catch (...) {
            // Device doesn't implement listSampleRates (e.g. LimeSDR — continuous range).
        }

        // Step 2: if no exact discrete match, query the continuous range and compute
        // the smallest integer multiple of the DSP rate that meets the device minimum.
        if (!exactRateMatch) {
            try {
                SoapySDR::RangeList srRanges = m_device->getSampleRateRange(SOAPY_SDR_RX, 0);
                if (!srRanges.empty()) {
                    m_minSampleRate = static_cast<int>(srRanges.front().minimum());
                    int maxSR       = static_cast<int>(srRanges.back().maximum());
                    qDebug() << "SoapySDRDataSource: Device sample rate range"
                             << m_minSampleRate / 1.0e6 << "to" << maxSR / 1.0e6 << "MSPS";
                    if (m_rfSampleRate < m_minSampleRate) {
                        m_decimRatio   = (int)std::ceil((double)m_minSampleRate / m_sampleRate);
                        m_rfSampleRate = m_decimRatio * m_sampleRate;
                        qWarning() << "SoapySDRDataSource: DSP rate" << m_sampleRate
                                   << "Hz below device minimum; RF rate" << m_rfSampleRate
                                   << "Hz (decimate by" << m_decimRatio << ")";
                    }
                }
            } catch (const std::exception &e) {
                qWarning() << "SoapySDRDataSource: Could not query sample rate range:" << e.what();
            }
        }

        // Fallback: if neither query method succeeded, use a conservative 2 MHz minimum.
        if (m_minSampleRate == 0 && m_rfSampleRate < 2000000) {
            m_minSampleRate = 2000000;
            m_decimRatio    = static_cast<int>(std::ceil(static_cast<double>(m_minSampleRate) / m_sampleRate));
            m_rfSampleRate  = m_decimRatio * m_sampleRate;
            qWarning() << "SoapySDRDataSource: Rate range unknown; using 2 MHz fallback minimum."
                       << "RF rate" << m_rfSampleRate << "Hz (decimate by" << m_decimRatio << ")";
        }

        // Hardware-key corrections: some drivers report an overly optimistic minimum
        // sample rate that the actual RF frontend cannot achieve.  Override with the
        // known real hardware floor so setSampleRate() doesn't silently fail.
        // LimeSDR-Mini / LimeSDR-Mini 2.0: LimeSuite reports 0.1 MSPS but the
        // LMS7002M VCO minimum means the ADC rate floor is ~1.25 MSPS.
        {
            const std::string hwKey = m_device->getHardwareKey();
            int hwMinHz = 0;
            if (hwKey == "LimeSDR-Mini" || hwKey == "LimeSDR-Mini 2.0")
                hwMinHz = 1300000; // 1.3 MSPS — safe margin above LMS7002M minimum
            if (hwMinHz > 0 && m_rfSampleRate < hwMinHz) {
                m_minSampleRate = hwMinHz;
                m_decimRatio    = static_cast<int>(std::ceil(static_cast<double>(hwMinHz) / m_sampleRate));
                m_rfSampleRate  = m_decimRatio * m_sampleRate;
                qWarning() << "SoapySDRDataSource:" << QString::fromStdString(hwKey)
                           << "driver minimum overridden to" << hwMinHz / 1.0e6 << "MHz;"
                           << "RF rate" << m_rfSampleRate << "Hz (decimate by" << m_decimRatio << ")";
            }
        }

        m_device->setSampleRate(SOAPY_SDR_RX, 0, m_rfSampleRate);
        // Read back actual rate — device may round to nearest supported value.
        try {
            double actual = m_device->getSampleRate(SOAPY_SDR_RX, 0);
            if (std::abs(actual - m_rfSampleRate) > 1.0) {
                m_rfSampleRate = static_cast<int>(std::round(actual));
                m_decimRatio   = std::max(1, static_cast<int>(std::round(actual / m_sampleRate)));
                qDebug() << "SoapySDRDataSource: Actual RF rate" << (int)actual
                         << "Hz, decimate-by adjusted to" << m_decimRatio;
            }
        } catch (...) {}

        // Try to find a better antenna for LimeSDR (LNAH is usually better for HF/VHF)
        try {
            std::vector<std::string> antennas = m_device->listAntennas(SOAPY_SDR_RX, 0);
            QString antennaList;
            for (const auto& a : antennas) antennaList += QString::fromStdString(a) + " ";
            qDebug() << "SoapySDRDataSource: Available antennas:" << antennaList;
            
            QString targetAntenna = "LNAH"; 
            bool found = false;
            for (const auto& a : antennas) {
                if (QString::fromStdString(a) == targetAntenna) {
                    found = true;
                    break;
                }
            }
            
            if (!found && !antennas.empty()) targetAntenna = QString::fromStdString(antennas[0]);
            
            m_device->setAntenna(SOAPY_SDR_RX, 0, targetAntenna.toStdString());
            qDebug() << "SoapySDRDataSource: Selected antenna:" << targetAntenna;

            // Set high gain
            if (m_device->getHardwareKey() == "LimeSDR-Mini") {
                // LNA 25 dB: good noise figure with headroom before ADC saturation.
                // TIA 12 dB: only valid values are 0/9/12; max always preferred for NF.
                // PGA 12 dB: mid-range trim (~2/3 of max), leaves room to increase.
                // Total: 49 dB — receivable on most bands without saturating on
                // strong HF broadcast / VHF signals.
                m_device->setGain(SOAPY_SDR_RX, 0, "LNA", 25.0);
                m_device->setGain(SOAPY_SDR_RX, 0, "TIA", 12.0);
                m_device->setGain(SOAPY_SDR_RX, 0, "PGA", 12.0);
            } else {
                m_device->setGain(SOAPY_SDR_RX, 0, 60.0);
            }
            m_device->setBandwidth(SOAPY_SDR_RX, 0, 5e6);
        } catch (const std::exception &e) {
            qDebug() << "SoapySDRDataSource: HW init warning:" << e.what();
        }
        
        m_rxStream = m_device->setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32);
        if (!m_rxStream)
            throw std::runtime_error("setupStream returned null");
        m_device->activateStream(m_rxStream);
        qDebug() << "[SoapySDR] init: stream active, RF" << m_rfSampleRate
                 << "Hz (DSP" << m_sampleRate << "Hz, decimate by" << m_decimRatio << ")";

        // Tune to current VFO frequency, clamped to the device's valid range
        long vfo = set->getVfoFrequency(0);
        if (vfo <= 0) vfo = 14200000L;
        if (m_minFrequency > 0 && vfo < m_minFrequency) {
            qDebug() << "SoapySDRDataSource: VFO" << vfo / 1.0e6
                     << "MHz below device minimum, clamping to" << m_minFrequency / 1.0e6 << "MHz";
            vfo = m_minFrequency;
        }
        if (m_maxFrequency > 0 && vfo > m_maxFrequency)
            vfo = m_maxFrequency;
        qDebug() << "[SoapySDR] init: setting RX center freq to" << vfo / 1.0e6 << "MHz";
        try {
            m_device->setFrequency(SOAPY_SDR_RX, 0, static_cast<double>(vfo));
            double actual = m_device->getFrequency(SOAPY_SDR_RX, 0);
            qDebug() << "[SoapySDR] init: hardware confirmed RX freq" << actual / 1.0e6 << "MHz";
        } catch (const std::exception &e) {
            qCritical() << "[SoapySDR] init: setFrequency(" << vfo / 1.0e6 << "MHz) failed:" << e.what();
            throw;
        }

        qDebug() << "SoapySDRDataSource: Stream activated at RF" << m_rfSampleRate
                 << "Hz (DSP" << m_sampleRate << "Hz), freq:" << vfo / 1.0e6 << "MHz";

        // DirectConnection: slots run on the caller's (UI) thread, writing only
        // atomics — safe because runStream() blocks the IO thread's event loop.
        connect(set, &Settings::sampleRateChanged, this, &SoapySDRDataSource::setSampleRate, Qt::DirectConnection);
        // vfoFrequencyChanged carries (mode, rx, frequency) — adapt with a lambda
        // so the slot receives the correct rx and frequency values.
        connect(set, &Settings::vfoFrequencyChanged, this,
                [this](int mode, int rx, long frequency) {
                    qDebug() << "[SoapySDR] vfoFrequencyChanged lambda: mode=" << mode
                             << "rx=" << rx << "freq=" << frequency / 1.0e6 << "MHz";
                    setFrequency(rx, frequency);
                }, Qt::DirectConnection);
        qDebug() << "[SoapySDR] init() complete — frequency and sample-rate signals connected";

    } catch (const std::exception &ex) {
        qCritical() << "SoapySDRDataSource init error:" << ex.what();
    }
}

void SoapySDRDataSource::stop() {
    m_stopped = true;
    if (m_device && m_rxStream) {
        m_device->deactivateStream(m_rxStream);
        m_device->closeStream(m_rxStream);
        m_rxStream = nullptr;
    }
    if (m_device) {
        SoapySDR::Device::unmake(m_device);
        m_device = nullptr;
    }
}

void SoapySDRDataSource::runStream() {
    if (!m_device || !m_rxStream) return;

    const size_t numSamples = 1024; // Match cudaSDR's BUFFER_SIZE
    std::vector<float> buff(numSamples * 2); // complex samples
    void *buffs[] = {buff.data()};

    // Decimation state — local to this streaming session; also reset on
    // runtime sample-rate changes inside the loop below.
    std::vector<float> decimBuff(numSamples * 2, 0.0f);
    int    decimOut = 0;
    double accumI = 0.0, accumQ = 0.0;
    int    accumN = 0;

    m_stopped = false;
    uint32_t packetCount = 0;
    m_lastKnownVfo = set->getVfoFrequency(0); // baseline so first poll doesn't retune

    while (!m_stopped) {
        // Poll Settings for VFO changes — reliable fallback for when the signal
        // connection from init() didn't fire (e.g. init() threw before connect()).
        {
            long polledVfo = set->getVfoFrequency(0);
            if (polledVfo != m_lastKnownVfo) {
                m_lastKnownVfo = polledVfo;
                long clamped = polledVfo;
                if (m_minFrequency > 0 && clamped < m_minFrequency) clamped = m_minFrequency;
                if (m_maxFrequency > 0 && clamped > m_maxFrequency) clamped = m_maxFrequency;
                m_pendingFreq.store(static_cast<double>(clamped), std::memory_order_relaxed);
                m_freqPending.store(true, std::memory_order_release);
                qDebug() << "[SoapySDR] runStream: VFO poll detected" << polledVfo / 1.0e6 << "MHz";
            }
        }

        // Apply pending frequency change (set by UI thread via DirectConnection slot
        // OR by the VFO poll above).
        if (m_freqPending.load(std::memory_order_acquire)) {
            m_freqPending.store(false, std::memory_order_relaxed);
            double freq = m_pendingFreq.load(std::memory_order_relaxed);
            qDebug() << "[SoapySDR] runStream: applying frequency" << freq / 1.0e6 << "MHz to hardware";
            try {
                m_device->setFrequency(SOAPY_SDR_RX, 0, freq);
                double actual = m_device->getFrequency(SOAPY_SDR_RX, 0);
                qDebug() << "[SoapySDR] hardware freq confirmed:" << actual / 1.0e6 << "MHz";
            } catch (const std::exception &e) {
                qWarning() << "[SoapySDR] setFrequency failed:" << e.what();
            }
        }

        // Apply pending sample-rate change (requires stream restart)
        if (m_sampleRatePending.load(std::memory_order_acquire)) {
            m_sampleRatePending.store(false, std::memory_order_relaxed);
            int newRfRate     = m_pendingRfSampleRate.load(std::memory_order_relaxed);
            int newDecimRatio = m_pendingDecimRatio.load(std::memory_order_relaxed);
            try {
                m_device->deactivateStream(m_rxStream);
                m_device->closeStream(m_rxStream);
                m_rxStream = nullptr;
                m_device->setSampleRate(SOAPY_SDR_RX, 0, newRfRate);
                m_rfSampleRate = newRfRate;
                m_decimRatio   = newDecimRatio;
                // Reset decimation state for the new rate.
                decimOut = 0; accumI = accumQ = 0.0; accumN = 0;
                m_rxStream = m_device->setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32);
                m_device->activateStream(m_rxStream);
                qDebug() << "SoapySDRDataSource: Sample rate applied:" << newRfRate
                         << "Hz (decimate by" << newDecimRatio << ")";
            } catch (const std::exception &e) {
                qWarning() << "SoapySDRDataSource: setSampleRate restart failed:" << e.what();
                m_stopped = true;
                break;
            }
            if (!m_rxStream) {
                qWarning() << "SoapySDRDataSource: stream null after sample rate change, stopping";
                m_stopped = true;
                break;
            }
        }

        int flags;
        long long timeNs;
        int ret = m_device->readStream(m_rxStream, buffs, numSamples, flags, timeNs, 1000000);

        if (ret > 0) {
            packetCount++;

            // Averaging decimator: accumulate m_decimRatio RF samples per output
            // sample, then emit full 1024-sample blocks to the DSP queue.
            // When m_decimRatio == 1 this is a zero-overhead pass-through.
            for (int i = 0; i < ret; ++i) {
                accumI += buff[i * 2];
                accumQ += buff[i * 2 + 1];
                if (++accumN == m_decimRatio) {
                    decimBuff[decimOut * 2]     = static_cast<float>(accumI / m_decimRatio);
                    decimBuff[decimOut * 2 + 1] = static_cast<float>(accumQ / m_decimRatio);
                    accumI = accumQ = 0.0;
                    accumN = 0;
                    if (++decimOut == static_cast<int>(numSamples)) {
                        QList<double> out;
                        out.reserve(static_cast<int>(numSamples) * 2);
                        for (float v : decimBuff) out.append(static_cast<double>(v));
                        io->data_queue.enqueue(out);
                        emit readydata();
                        decimOut = 0;
                    }
                }
            }

            // Simple backpressure: if DataProcessor is not clearing the queue, slow down reading
            if (io->data_queue.count() > 50) {
                QThread::msleep(10);
            }
        } else if (ret < 0) {
            qCritical() << "SoapySDRDataSource: readStream error" << ret;
        }
    }
}

// Runs on the UI thread (Qt::DirectConnection) — must only write atomics.
void SoapySDRDataSource::setSampleRate(int value) {
    m_sampleRate = value;
    // Recompute decimation ratio for the new DSP rate using the stored device minimum.
    int newDecimRatio = (m_minSampleRate > 0 && value < m_minSampleRate)
                        ? static_cast<int>(std::ceil(static_cast<double>(m_minSampleRate) / value))
                        : 1;
    int newRfRate = newDecimRatio * value;
    m_pendingDecimRatio.store(newDecimRatio, std::memory_order_relaxed);
    m_pendingRfSampleRate.store(newRfRate, std::memory_order_relaxed);
    m_sampleRatePending.store(true, std::memory_order_release);
    qDebug() << "SoapySDRDataSource: DSP rate" << value << "Hz queued (RF"
             << newRfRate << "Hz, decimate by" << newDecimRatio << ")";
}

// Runs on the UI thread (Qt::DirectConnection) — must only write atomics.
void SoapySDRDataSource::setFrequency(int rx, long frequency) {
    qDebug() << "[SoapySDR] setFrequency entry: rx=" << rx << "freq=" << frequency / 1.0e6 << "MHz"
             << "(minFreq=" << m_minFrequency / 1.0e6 << "maxFreq=" << m_maxFrequency / 1.0e6 << ")";
    Q_UNUSED(rx);
    long clamped = frequency;
    if (m_minFrequency > 0 && clamped < m_minFrequency) clamped = m_minFrequency;
    if (m_maxFrequency > 0 && clamped > m_maxFrequency) clamped = m_maxFrequency;
    m_pendingFreq.store(static_cast<double>(clamped), std::memory_order_relaxed);
    m_freqPending.store(true, std::memory_order_release);
    if (clamped != frequency)
        qDebug() << "[SoapySDR] freq" << frequency / 1.0e6 << "MHz queued (clamped to" << clamped / 1.0e6 << "MHz)";
    else
        qDebug() << "[SoapySDR] freq" << clamped / 1.0e6 << "MHz queued";
}

#endif // HAVE_SOAPYSDR
