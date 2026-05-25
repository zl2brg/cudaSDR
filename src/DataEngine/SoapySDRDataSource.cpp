#include "SoapySDRDataSource.h"

#ifdef HAVE_SOAPYSDR

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#include <QDebug>
#include <SoapySDR/Errors.hpp>
#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace {

constexpr int kReadTimeoutUs = 200000;
constexpr int kStreamTimeoutsBeforeRestart = 5;

bool listLooksLikeRangeEndpoints(const std::vector<double>& rates) {
    if (rates.size() < 2)
        return true;
    if (rates.size() == 2) {
        const double lo = std::min(rates[0], rates[1]);
        const double hi = std::max(rates[0], rates[1]);
        if (lo <= 0.0)
            return true;
        return (hi / lo) > 50.0;
    }
    return false;
}

} // namespace

bool SoapySDRDataSource::isSampleRateCompatible(double rfHz, int dspHz) {
    if (dspHz <= 0 || rfHz + 0.5 < static_cast<double>(dspHz))
        return false;
    const int ratio = static_cast<int>(std::lround(rfHz / static_cast<double>(dspHz)));
    if (ratio < 1)
        return false;
    return std::abs(rfHz - ratio * static_cast<double>(dspHz)) < 1.0;
}

int SoapySDRDataSource::hardwareMinSampleRateHz(const std::string& hwKey, int driverReportedMin) {
    if (hwKey == "LimeSDR-Mini" || hwKey == "LimeSDR-Mini 2.0")
        return std::max(driverReportedMin, 1300000);
    return driverReportedMin;
}

void SoapySDRDataSource::applyBandwidthForRfRate(int rfSampleRate) {
    if (!m_device)
        return;
    // Keep analog bandwidth close to the RF rate we actually use (not a fixed 5 MHz).
    const double bw = std::min(5.0e6, std::max(static_cast<double>(rfSampleRate) * 1.25,
                                                static_cast<double>(m_sampleRate) * 1.5));
    try {
        m_device->setBandwidth(SOAPY_SDR_RX, 0, bw);
    } catch (const std::exception& e) {
        qWarning() << "SoapySDRDataSource: setBandwidth failed:" << e.what();
    }
}

SoapySDRDataSource::RfRatePlan SoapySDRDataSource::chooseRfSampleRate(int dspRate) const {
    RfRatePlan plan;
    plan.rfSampleRate = dspRate;
    plan.decimRatio = 1;

    if (dspRate <= 0)
        return plan;

    int rangeMinHz = 0;
    int rangeMaxHz = 65000000;
    std::vector<double> listedRates;

    if (m_device) {
        try {
            const SoapySDR::RangeList srRanges = m_device->getSampleRateRange(SOAPY_SDR_RX, 0);
            if (!srRanges.empty()) {
                rangeMinHz = static_cast<int>(srRanges.front().minimum());
                rangeMaxHz = static_cast<int>(srRanges.back().maximum());
            }
        } catch (const std::exception& e) {
            qWarning() << "SoapySDRDataSource: getSampleRateRange failed:" << e.what();
        }

        try {
            listedRates = m_device->listSampleRates(SOAPY_SDR_RX, 0);
        } catch (...) {
            listedRates.clear();
        }
    }

    int driverMin = rangeMinHz;
    if (driverMin <= 0 && !listedRates.empty())
        driverMin = static_cast<int>(*std::min_element(listedRates.begin(), listedRates.end()));

    const std::string hwKey = m_device ? m_device->getHardwareKey() : std::string();
    plan.effectiveMinHz = hardwareMinSampleRateHz(hwKey, driverMin);
    if (plan.effectiveMinHz <= 0)
        plan.effectiveMinHz = 225000; // conservative RTL-ish fallback when the driver reports nothing

    std::vector<double> candidates;

    if (!listedRates.empty() && !listLooksLikeRangeEndpoints(listedRates)) {
        for (double rate : listedRates) {
            if (!isSampleRateCompatible(rate, dspRate))
                continue;
            if (rate + 0.5 < plan.effectiveMinHz)
                continue;
            if (rate > rangeMaxHz + 1.0)
                continue;
            candidates.push_back(rate);
        }
        if (!candidates.empty()) {
            qDebug() << "SoapySDRDataSource: picked from" << candidates.size()
                     << "listed rates compatible with DSP" << dspRate << "Hz";
        }
    } else if (!listedRates.empty()) {
        qDebug() << "SoapySDRDataSource: listSampleRates looks like min/max endpoints only"
                 << "(count" << listedRates.size() << ") — using integer multiples of DSP rate";
    }

    // Native DSP rate is best when the device supports it (decimRatio == 1).
    if (static_cast<double>(dspRate) >= plan.effectiveMinHz - 1.0
        && static_cast<double>(dspRate) <= rangeMaxHz + 1.0) {
        candidates.push_back(static_cast<double>(dspRate));
    }

    if (candidates.empty()) {
        const int kMin = std::max(1, static_cast<int>(std::ceil(
            static_cast<double>(plan.effectiveMinHz) / static_cast<double>(dspRate))));
        const int kMax = std::min(kMin + 128, std::max(kMin, rangeMaxHz / dspRate));
        for (int k = kMin; k <= kMax; ++k) {
            const double rate = static_cast<double>(k) * static_cast<double>(dspRate);
            if (rate <= rangeMaxHz + 1.0)
                candidates.push_back(rate);
        }
    }

    double bestHz = std::numeric_limits<double>::max();
    for (double rate : candidates) {
        if (rate < bestHz)
            bestHz = rate;
    }

    if (bestHz == std::numeric_limits<double>::max()) {
        plan.decimRatio = std::max(1, static_cast<int>(std::ceil(
            static_cast<double>(plan.effectiveMinHz) / static_cast<double>(dspRate))));
        plan.rfSampleRate = plan.decimRatio * dspRate;
        qWarning() << "SoapySDRDataSource: no compatible rate found; using RF"
                   << plan.rfSampleRate << "Hz (decimate by" << plan.decimRatio << ")";
    } else {
        plan.rfSampleRate = static_cast<int>(std::lround(bestHz));
        plan.decimRatio = std::max(1, static_cast<int>(std::lround(bestHz / dspRate)));
        qDebug() << "SoapySDRDataSource: smallest compatible RF rate" << plan.rfSampleRate
                 << "Hz for DSP" << dspRate << "Hz (decimate by" << plan.decimRatio << ")";
    }

    return plan;
}

bool SoapySDRDataSource::syncRfRateFromHardware(int dspRate) {
    if (!m_device || dspRate <= 0)
        return false;

    try {
        const double actual = m_device->getSampleRate(SOAPY_SDR_RX, 0);
        const int actualHz = static_cast<int>(std::lround(actual));
        int ratio = std::max(1, static_cast<int>(std::lround(actualHz / static_cast<double>(dspRate))));
        const int snappedRf = ratio * dspRate;

        if (snappedRf != m_rfSampleRate || ratio != m_decimRatio) {
            qDebug() << "SoapySDRDataSource: RF rate readback" << actualHz
                     << "Hz -> use" << snappedRf << "Hz (decimate by" << ratio << ")";
            if (std::abs(actualHz - snappedRf) > 1) {
                m_device->setSampleRate(SOAPY_SDR_RX, 0, snappedRf);
            }
            m_rfSampleRate = snappedRf;
            m_decimRatio = ratio;
        }
        return true;
    } catch (const std::exception& e) {
        qWarning() << "SoapySDRDataSource: syncRfRateFromHardware failed:" << e.what();
        return false;
    }
}

bool SoapySDRDataSource::restartRxStream() {
    if (!m_device)
        return false;

    try {
        if (m_rxStream) {
            m_device->deactivateStream(m_rxStream);
            m_device->closeStream(m_rxStream);
            m_rxStream = nullptr;
        }
        m_rxStream = m_device->setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32);
        if (!m_rxStream)
            return false;
        m_device->activateStream(m_rxStream);
        m_streamTimeouts = 0;
        qDebug() << "SoapySDRDataSource: RX stream restarted";
        return true;
    } catch (const std::exception& e) {
        qWarning() << "SoapySDRDataSource: restartRxStream failed:" << e.what();
        m_rxStream = nullptr;
        return false;
    }
}

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
    , m_streamTimeouts(0)
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

        m_sampleRate = set->getSampleRate();
        const RfRatePlan plan = chooseRfSampleRate(m_sampleRate);
        m_rfSampleRate = plan.rfSampleRate;
        m_decimRatio = plan.decimRatio;
        m_minSampleRate = plan.effectiveMinHz;

        m_device->setSampleRate(SOAPY_SDR_RX, 0, m_rfSampleRate);
        syncRfRateFromHardware(m_sampleRate);

        // Publish hardware key to Settings so the UI can show appropriate controls
        std::string hwKey = m_device->getHardwareKey();
        set->setSoapyHardwareKey(QString::fromStdString(hwKey));

        // Build and publish antenna list; then select from Settings (with LNAH fallback)
        try {
            std::vector<std::string> antennas = m_device->listAntennas(SOAPY_SDR_RX, 0);
            QStringList antennaQList;
            for (const auto& a : antennas) antennaQList << QString::fromStdString(a);
            set->setSoapyAntennaList(antennaQList);
            qDebug() << "SoapySDRDataSource: Available antennas:" << antennaQList.join(" ");

            // Pick antenna: prefer stored setting, fall back to LNAH, then first available
            QString wantAntenna = set->getSoapyRxAntenna();
            if (wantAntenna.isEmpty() || !antennaQList.contains(wantAntenna)) {
                wantAntenna = antennaQList.contains("LNAH") ? "LNAH"
                            : antennaQList.isEmpty()        ? ""
                            :                                  antennaQList.first();
                if (!wantAntenna.isEmpty())
                    set->setSoapyRxAntenna(wantAntenna);
            }
            if (!wantAntenna.isEmpty()) {
                m_device->setAntenna(SOAPY_SDR_RX, 0, wantAntenna.toStdString());
                qDebug() << "SoapySDRDataSource: Selected antenna:" << wantAntenna;
            }
        } catch (const std::exception &e) {
            qWarning() << "SoapySDRDataSource: Antenna setup warning:" << e.what();
        }

        // Apply gain from Settings
        try {
            if (QString::fromStdString(m_device->getHardwareKey()).contains("LimeSDR", Qt::CaseInsensitive)) {
                m_device->setGain(SOAPY_SDR_RX, 0, "LNA", set->getSoapyLnaGain());
                m_device->setGain(SOAPY_SDR_RX, 0, "TIA", set->getSoapyTiaGain());
                m_device->setGain(SOAPY_SDR_RX, 0, "PGA", set->getSoapyPgaGain());
            } else {
                m_device->setGain(SOAPY_SDR_RX, 0, set->getSoapyOverallGain());
            }
        } catch (const std::exception &e) {
            qWarning() << "SoapySDRDataSource: Gain setup warning:" << e.what();
        }

        // Tune before starting stream (Lime needs stable RF path before activate).
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

        applyBandwidthForRfRate(m_rfSampleRate);

        if (!restartRxStream())
            throw std::runtime_error("setupStream/activateStream failed");

        qDebug() << "SoapySDRDataSource: Stream active at RF" << m_rfSampleRate
                 << "Hz (DSP" << m_sampleRate << "Hz, decimate by" << m_decimRatio
                 << "), freq:" << vfo / 1.0e6 << "MHz";

        // DirectConnection: slots run on the caller's (UI) thread, writing only
        // atomics — safe because runStream() blocks the IO thread's event loop.
        connect(set, &Settings::sampleRateChanged, this, &SoapySDRDataSource::setSampleRate, Qt::DirectConnection);
        // vfoFrequencyChanged carries (mode, rx, frequency) — adapt with a lambda
        // so the slot receives the correct rx and frequency values.
        connect(set, &Settings::vfoFrequencyChanged, this,
                [this](int mode, int rx, long frequency) {
                    Q_UNUSED(mode);
                    setFrequency(rx, frequency);
                }, Qt::DirectConnection);

        // Apply antenna/gain changes from the Radio tab in real-time
        connect(set, &Settings::soapyRxAntennaChanged, this,
                [this](const QString &antenna) {
                    if (m_device) {
                        try { m_device->setAntenna(SOAPY_SDR_RX, 0, antenna.toStdString()); }
                        catch (const std::exception &e) {
                            qWarning() << "SoapySDR: setAntenna failed:" << e.what();
                        }
                    }
                }, Qt::DirectConnection);
        connect(set, &Settings::soapyLnaGainChanged, this,
                [this](int gain) {
                    if (m_device) {
                        try { m_device->setGain(SOAPY_SDR_RX, 0, "LNA", gain); }
                        catch (...) {}
                    }
                }, Qt::DirectConnection);
        connect(set, &Settings::soapyTiaGainChanged, this,
                [this](int gain) {
                    if (m_device) {
                        try { m_device->setGain(SOAPY_SDR_RX, 0, "TIA", gain); }
                        catch (...) {}
                    }
                }, Qt::DirectConnection);
        connect(set, &Settings::soapyPgaGainChanged, this,
                [this](int gain) {
                    if (m_device) {
                        try { m_device->setGain(SOAPY_SDR_RX, 0, "PGA", gain); }
                        catch (...) {}
                    }
                }, Qt::DirectConnection);
        connect(set, &Settings::soapyOverallGainChanged, this,
                [this](int gain) {
                    if (m_device) {
                        try { m_device->setGain(SOAPY_SDR_RX, 0, gain); }
                        catch (...) {}
                    }
                }, Qt::DirectConnection);
        connect(set, &Settings::soapyAutoCalibrateChanged, this,
                [this](bool enabled) {
                    if (m_device) {
                        try {
                            m_device->writeSetting("AUTO_CALIBRATION",
                                                   enabled ? "TRUE" : "FALSE");
                        }
                        catch (...) {}
                    }
                }, Qt::DirectConnection);

        qDebug() << "[SoapySDR] init() complete — signals connected";

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
            }
        }

        // Apply pending sample-rate change (requires stream restart)
        if (m_sampleRatePending.load(std::memory_order_acquire)) {
            m_sampleRatePending.store(false, std::memory_order_relaxed);
            const int newRfRate = m_pendingRfSampleRate.load(std::memory_order_relaxed);
            try {
                if (m_rxStream) {
                    m_device->deactivateStream(m_rxStream);
                    m_device->closeStream(m_rxStream);
                    m_rxStream = nullptr;
                }
                m_device->setSampleRate(SOAPY_SDR_RX, 0, newRfRate);
                m_rfSampleRate = newRfRate;
                syncRfRateFromHardware(m_sampleRate);
                applyBandwidthForRfRate(m_rfSampleRate);
                decimOut = 0;
                accumI = accumQ = 0.0;
                accumN = 0;
                if (!restartRxStream()) {
                    qWarning() << "SoapySDRDataSource: stream restart failed after sample rate change";
                    m_stopped = true;
                    break;
                }
                qDebug() << "SoapySDRDataSource: Sample rate applied:" << m_rfSampleRate
                         << "Hz (decimate by" << m_decimRatio << ")";
            } catch (const std::exception &e) {
                qWarning() << "SoapySDRDataSource: setSampleRate restart failed:" << e.what();
                m_stopped = true;
                break;
            }
        }

        if (!m_rxStream)
            continue;

        int flags;
        long long timeNs;
        int ret = m_device->readStream(m_rxStream, buffs, numSamples, flags, timeNs, kReadTimeoutUs);

        if (ret > 0) {
            packetCount++;
            m_streamTimeouts = 0;

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
            const int err = ret;
            if (++m_streamTimeouts >= kStreamTimeoutsBeforeRestart) {
                qWarning() << "SoapySDRDataSource: readStream"
                           << SoapySDR::errToStr(err) << "(" << err << ") — restarting stream";
                m_streamTimeouts = 0;
                if (restartRxStream()) {
                    decimOut = 0;
                    accumI = accumQ = 0.0;
                    accumN = 0;
                }
            }
        }

        // Tune after readStream so setFrequency never blocks IQ capture.
        while (m_freqPending.exchange(false, std::memory_order_acq_rel)) {
            const double freq = m_pendingFreq.load(std::memory_order_relaxed);
            if (!m_device)
                break;
            try {
                m_device->setFrequency(SOAPY_SDR_RX, 0, freq);
            } catch (const std::exception& e) {
                qWarning() << "SoapySDRDataSource: setFrequency failed:" << e.what();
            }
        }
    }
}

// Runs on the UI thread (Qt::DirectConnection) — must only write atomics.
void SoapySDRDataSource::setSampleRate(int value) {
    m_sampleRate = value;
    const RfRatePlan plan = chooseRfSampleRate(value);
    m_minSampleRate = plan.effectiveMinHz;
    m_pendingDecimRatio.store(plan.decimRatio, std::memory_order_relaxed);
    m_pendingRfSampleRate.store(plan.rfSampleRate, std::memory_order_relaxed);
    m_sampleRatePending.store(true, std::memory_order_release);
    qDebug() << "SoapySDRDataSource: DSP rate" << value << "Hz queued (RF"
             << plan.rfSampleRate << "Hz, decimate by" << plan.decimRatio << ")";
}

// Runs on the UI thread (Qt::DirectConnection) — must only write atomics.
void SoapySDRDataSource::setFrequency(int rx, long frequency) {
    Q_UNUSED(rx);
    long clamped = frequency;
    if (m_minFrequency > 0 && clamped < m_minFrequency) clamped = m_minFrequency;
    if (m_maxFrequency > 0 && clamped > m_maxFrequency) clamped = m_maxFrequency;
    m_pendingFreq.store(static_cast<double>(clamped), std::memory_order_relaxed);
    m_freqPending.store(true, std::memory_order_release);
}

#endif // HAVE_SOAPYSDR
