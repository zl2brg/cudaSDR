#include "SoapySDRDataSource.h"

#ifdef HAVE_SOAPYSDR

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#include <QDebug>
#include <QMutexLocker>
#include <SoapySDR/Errors.hpp>
#include <algorithm>
#include <cmath>
#include <complex>
#include <limits>
#include <vector>

namespace {

constexpr int kReadTimeoutUs = 200000;
constexpr int kStreamTimeoutsBeforeRestart = 5;
constexpr int kSoapyWidebandFftSize = 1024;
constexpr qint64 kSoapyWidebandUpdateMs = 50;

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

// IQ tone level for TUNE; drive 128 = max peak (RF gain still from drive slider).
float tuneToneAmplitudeFromDrive(int driveLevel) {
    const float t = qBound(0.0f, driveLevel / 128.0f, 1.0f);
    constexpr float kMaxTuneAmp = 0.25f;
    return kMaxTuneAmp * t;
}

} // namespace

void SoapySDRDataSource::publishWidebandSpectrum(const float* interleavedIQ, int complexSamples)
{
    if (!interleavedIQ || complexSamples < kSoapyWidebandFftSize)
        return;

    static QElapsedTimer s_wbTimer;
    if (!s_wbTimer.isValid())
        s_wbTimer.start();
    if (s_wbTimer.elapsed() < kSoapyWidebandUpdateMs)
        return;
    s_wbTimer.restart();

    std::vector<std::complex<float>> fft(kSoapyWidebandFftSize);
    for (int i = 0; i < kSoapyWidebandFftSize; ++i) {
        const float w = 0.5f - 0.5f * std::cos(2.0f * static_cast<float>(M_PI) * i / (kSoapyWidebandFftSize - 1));
        fft[i] = std::complex<float>(interleavedIQ[2 * i], interleavedIQ[2 * i + 1]) * w;
    }

    // Iterative radix-2 FFT.
    for (int i = 1, j = 0; i < kSoapyWidebandFftSize; ++i) {
        int bit = kSoapyWidebandFftSize >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(fft[i], fft[j]);
    }
    for (int len = 2; len <= kSoapyWidebandFftSize; len <<= 1) {
        const float ang = -2.0f * static_cast<float>(M_PI) / len;
        const std::complex<float> wlen(std::cos(ang), std::sin(ang));
        for (int i = 0; i < kSoapyWidebandFftSize; i += len) {
            std::complex<float> w(1.0f, 0.0f);
            for (int j = 0; j < len / 2; ++j) {
                const std::complex<float> u = fft[i + j];
                const std::complex<float> v = fft[i + j + len / 2] * w;
                fft[i + j] = u + v;
                fft[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }

    // Publish full FFT width and FFT-shift it so the VFO (DC) sits in the center.
    // The previous half-width + quarter-wrap mapping suppressed/warped peaks.
    qVectorFloat spectrum(kSoapyWidebandFftSize);
    for (int i = 0; i < kSoapyWidebandFftSize; ++i) {
        const int idx = (i + kSoapyWidebandFftSize / 2) % kSoapyWidebandFftSize;
        const float pwr = std::norm(fft[idx]) + 1e-12f;
        spectrum[i] = 10.0f * std::log10(pwr) - 65.0f;
    }
    emit widebandSpectrumReady(spectrum);
}

void SoapySDRDataSource::publishWidebandFrequencyRange()
{
    qint64 centerHz = set->getVfoFrequency(0);
    if (centerHz <= 0)
        centerHz = static_cast<qint64>(m_pendingFreq.load(std::memory_order_relaxed));
    if (centerHz <= 0)
        centerHz = 14200000LL;

    const qreal spanHz = static_cast<qreal>(std::max(1, m_rfSampleRate));
    const qreal halfSpan = spanHz * 0.5;
    emit widebandFrequencyRangeReady(centerHz - halfSpan, centerHz + halfSpan);
}

bool SoapySDRDataSource::isLimeHardware() const
{
    if (!m_device)
        return false;
    return QString::fromStdString(m_device->getHardwareKey())
        .contains("LimeSDR", Qt::CaseInsensitive);
}

void SoapySDRDataSource::requestHardwareRetune()
{
    qint64 vfo = set->getVfoFrequency(0);
    if (vfo <= 0)
        vfo = 14200000L;
    if (m_minFrequency > 0 && vfo < m_minFrequency)
        vfo = m_minFrequency;
    if (m_maxFrequency > 0 && vfo > m_maxFrequency)
        vfo = m_maxFrequency;
    m_pendingFreq.store(static_cast<double>(vfo), std::memory_order_relaxed);
    m_freqPending.store(true, std::memory_order_release);
}

void SoapySDRDataSource::applyLimeAutoCalibrate(bool enabled)
{
    if (!m_device || !isLimeHardware())
        return;

    // SoapyLMS7 has no AUTO_CALIBRATION key (unknown keys are parsed with stoi → throws on "TRUE").
    // Use Soapy DC-offset mode plus CALIBRATE_RX bandwidth trigger instead.
    qDebug() << "SoapySDR: Lime auto-calibrate" << (enabled ? "on" : "off")
             << "(DC offset mode + CALIBRATE_RX)";

    if (m_device->hasDCOffsetMode(SOAPY_SDR_RX, 0)) {
        try {
            m_device->setDCOffsetMode(SOAPY_SDR_RX, 0, enabled);
            qDebug() << "SoapySDR: setDCOffsetMode(RX,0)=" << enabled;
        } catch (const std::exception &e) {
            qWarning() << "SoapySDR: setDCOffsetMode failed:" << e.what();
        }
    } else {
        qDebug() << "SoapySDR: device has no Soapy DC offset mode API";
    }

    if (enabled) {
        try {
            const double calBw =
                std::max(500000.0, static_cast<double>(std::max(m_rfSampleRate, m_sampleRate)) * 0.8);
            m_device->writeSetting(SOAPY_SDR_RX, 0, "CALIBRATE_RX", std::to_string(calBw));
            qDebug() << "SoapySDR: CALIBRATE_RX bandwidth" << calBw;
        } catch (const std::exception &e) {
            qWarning() << "SoapySDR: CALIBRATE_RX failed:" << e.what();
        }
        requestHardwareRetune();
    } else {
        try {
            m_device->setGain(SOAPY_SDR_RX, 0, "LNA", set->getSoapyLnaGain());
            m_device->setGain(SOAPY_SDR_RX, 0, "TIA", set->getSoapyTiaGain());
            m_device->setGain(SOAPY_SDR_RX, 0, "PGA", set->getSoapyPgaGain());
        } catch (const std::exception &e) {
            qWarning() << "SoapySDR: manual gain restore failed:" << e.what();
        }
    }
}

bool SoapySDRDataSource::isSampleRateCompatible(double rfHz, int dspHz) {
    if (dspHz <= 0 || rfHz + 0.5 < static_cast<double>(dspHz))
        return false;
    const int ratio = static_cast<int>(std::lround(rfHz / static_cast<double>(dspHz)));
    if (ratio < 1)
        return false;
    return std::abs(rfHz - ratio * static_cast<double>(dspHz)) < 1.0;
}

int SoapySDRDataSource::hardwareMinSampleRateHz(const std::string& hwKey, int driverReportedMin) {
    QString key = QString::fromStdString(hwKey).toUpper();
    if (key.contains("LIMESDR"))
        return std::max(driverReportedMin, 1300000);
    if (key.contains("RTLSDR") || key.contains("RTL-SDR") || key.contains("R820T") || key.contains("RTL2832"))
        return std::max(driverReportedMin, 900001);
    if (key.contains("AIRSPY"))
        return std::max(driverReportedMin, 2500000);
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
        plan.effectiveMinHz = 900001; // conservative fallback (RTL-SDR minimum) when the driver reports nothing

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

bool SoapySDRDataSource::restartTxStream() {
    if (!m_device || !m_txCapable)
        return false;

    try {
        if (m_txStream) {
            m_device->deactivateStream(m_txStream);
            m_device->closeStream(m_txStream);
            m_txStream = nullptr;
        }
        m_txStream = m_device->setupStream(SOAPY_SDR_TX, SOAPY_SDR_CF32);
        if (!m_txStream)
            return false;
        m_device->activateStream(m_txStream);
        double txRate = 0.0;
        double txFreq = 0.0;
        try { txRate = m_device->getSampleRate(SOAPY_SDR_TX, 0); } catch (...) {}
        try { txFreq = m_device->getFrequency(SOAPY_SDR_TX, 0); } catch (...) {}
        qDebug() << "SoapySDRDataSource: TX stream restarted rate =" << txRate
                 << "Hz freq =" << txFreq / 1.0e6 << "MHz";
        return true;
    } catch (const std::exception& e) {
        qWarning() << "SoapySDRDataSource: restartTxStream failed:" << e.what();
        m_txStream = nullptr;
        return false;
    }
}

SoapySDRDataSource::SoapySDRDataSource(THPSDRParameter *ioData)
    : QObject(nullptr)
    , set(Settings::instance())
    , io(ioData)
    , m_device(nullptr)
    , m_rxStream(nullptr)
    , m_txStream(nullptr)
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
    , m_txUnderrunCount(0)
    , m_txErrorCount(0)
    , m_txTonePhase(1.0f, 0.0f)
    , m_txCapable(false)
    , m_txActive(false)
    , m_radioStateValue(static_cast<int>(RadioState::RX))
    , m_lastTxSetFrequency(0)
    , m_txDebugPrimed(false)
    , m_rxResampler(nullptr)
    , m_rxResampIn(nullptr)
    , m_rxResampOut(nullptr)
    , m_txResampler(nullptr)
    , m_txResampIn(nullptr)
    , m_txResampOut(nullptr)
{
}

SoapySDRDataSource::~SoapySDRDataSource() {
    stop();
    if (m_rxResampler) resamp_crcf_destroy(m_rxResampler);
    if (m_rxResampIn) delete[] (float*)m_rxResampIn;
    if (m_rxResampOut) delete[] (float*)m_rxResampOut;
    if (m_txResampler) resamp_crcf_destroy(m_txResampler);
    if (m_txResampIn) delete[] (float*)m_txResampIn;
    if (m_txResampOut) delete[] (float*)m_txResampOut;
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

        m_txCapable = (m_device->getNumChannels(SOAPY_SDR_TX) > 0);
        set->setTxAllowed(m_txCapable);
        if (!m_txCapable) {
            qDebug() << "SoapySDRDataSource: device has no TX channels (RX-only)";
        }

        // Query device frequency range before any hardware configuration so we
        // know the valid range even if subsequent calls throw.
        try {
            SoapySDR::RangeList ranges = m_device->getFrequencyRange(SOAPY_SDR_RX, 0);
            if (!ranges.empty()) {
                m_minFrequency = static_cast<qint64>(ranges.front().minimum());
                m_maxFrequency = static_cast<qint64>(ranges.back().maximum());
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
        if (m_txCapable)
            configureTxSampleRate();

        setupResamplers(m_rfSampleRate, m_sampleRate, m_txSampleRate, kTxIqSampleRate);

        {
            QMutexLocker lock(&io->mutex);
            // Resampler will output exactly at the DSP rate
            io->soapyInputSampleRate = m_sampleRate;
        }

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

        // Configure TX antenna/path where available so transmitted tone reaches the RF output.
        if (m_txCapable) {
            try {
                const std::vector<std::string> txAntennas = m_device->listAntennas(SOAPY_SDR_TX, 0);
                if (!txAntennas.empty()) {
                    QStringList txAntennaQList;
                    for (const auto &a : txAntennas)
                        txAntennaQList << QString::fromStdString(a);
                    set->setSoapyTxAntennaList(txAntennaQList);

                    // Prefer RX-selected antenna if present on TX, else use first TX antenna.
                    QString wantTxAntenna = set->getSoapyTxAntenna();
                    if (wantTxAntenna.isEmpty())
                        wantTxAntenna = set->getSoapyRxAntenna();
                    std::string txAntenna = txAntennas.front();
                    for (const std::string &a : txAntennas) {
                        if (QString::fromStdString(a) == wantTxAntenna) {
                            txAntenna = a;
                            break;
                        }
                    }
                    // Never select an explicit disabled route if alternatives exist.
                    if (QString::fromStdString(txAntenna).compare("NONE", Qt::CaseInsensitive) == 0) {
                        for (const std::string &a : txAntennas) {
                            if (QString::fromStdString(a).compare("NONE", Qt::CaseInsensitive) != 0) {
                                txAntenna = a;
                                break;
                            }
                        }
                    }
                    m_device->setAntenna(SOAPY_SDR_TX, 0, txAntenna);
                    set->setSoapyTxAntenna(QString::fromStdString(txAntenna));
                    qDebug() << "SoapySDR TX: antenna set to" << QString::fromStdString(txAntenna);
                }
                try {
                    const std::string actualTxAntenna = m_device->getAntenna(SOAPY_SDR_TX, 0);
                    qDebug() << "SoapySDR TX: antenna readback" << QString::fromStdString(actualTxAntenna);
                } catch (...) {}
            } catch (const std::exception &e) {
                qWarning() << "SoapySDR TX: antenna setup warning:" << e.what();
            }
        }

        // Gain (Lime auto-cal applied after tune below)
        try {
            if (!isLimeHardware()) {
                m_device->setGain(SOAPY_SDR_RX, 0, set->getSoapyOverallGain());
            } else if (!set->getSoapyAutoCalibrate()) {
                m_device->setGain(SOAPY_SDR_RX, 0, "LNA", set->getSoapyLnaGain());
                m_device->setGain(SOAPY_SDR_RX, 0, "TIA", set->getSoapyTiaGain());
                m_device->setGain(SOAPY_SDR_RX, 0, "PGA", set->getSoapyPgaGain());
            }
        } catch (const std::exception &e) {
            qWarning() << "SoapySDRDataSource: Gain setup warning:" << e.what();
        }

        auto applySoapyTxDrive = [this](int driveValue) {
            if (!m_device || !m_txCapable)
                return;

            // Map UI drive [0..128] to the Soapy TX gain range where available.
            double gainToApply = static_cast<double>(driveValue);
            try {
                const SoapySDR::Range txGainRange = m_device->getGainRange(SOAPY_SDR_TX, 0);
                const double minGain = txGainRange.minimum();
                const double maxGain = txGainRange.maximum();
                if (maxGain > minGain) {
                    const double t = qBound(0.0, driveValue / 128.0, 1.0);
                    gainToApply = minGain + (maxGain - minGain) * t;
                }
            } catch (...) {
                // Keep fallback gainToApply derived from raw drive value.
            }

            try {
                m_device->setGain(SOAPY_SDR_TX, 0, gainToApply);
                qDebug() << "SoapySDR TX: drive level" << driveValue << "-> gain" << gainToApply;
            } catch (const std::exception &e) {
                qWarning() << "SoapySDR TX: set gain from drive level failed:" << e.what();
            }
        };

        // TX gain: use drive slider as primary TX drive control.
        if (m_txCapable) {
            try {
                int drive = set->getDriveLevel();
                if (drive <= 0)
                    drive = 64;
                applySoapyTxDrive(drive);
                try {
                    const double txGainReadback = m_device->getGain(SOAPY_SDR_TX, 0);
                    qDebug() << "SoapySDR TX: gain readback" << txGainReadback;
                } catch (...) {}
            } catch (const std::exception &e) {
                qWarning() << "SoapySDR TX: gain setup warning:" << e.what();
            }
        }

        // Tune before starting stream (Lime needs stable RF path before activate).
        qint64 vfo = set->getVfoFrequency(0);
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
            if (m_txCapable)
                m_device->setFrequency(SOAPY_SDR_TX, 0, static_cast<double>(vfo));
            double actual = m_device->getFrequency(SOAPY_SDR_RX, 0);
            qDebug() << "[SoapySDR] init: hardware confirmed RX freq" << actual / 1.0e6 << "MHz";
        } catch (const std::exception &e) {
            qCritical() << "[SoapySDR] init: setFrequency(" << vfo / 1.0e6 << "MHz) failed:" << e.what();
            throw;
        }

        applyBandwidthForRfRate(m_rfSampleRate);

        if (!restartRxStream())
            throw std::runtime_error("setupStream/activateStream failed");
        // TX stream is opened lazily on first MOX/TUNE — do not activate here.
        // Keeping the TX stream inactive during receive prevents LimeSDR from
        // radiating a carrier on the RX frequency.

        if (isLimeHardware())
            applyLimeAutoCalibrate(set->getSoapyAutoCalibrate());

        qDebug() << "SoapySDRDataSource: Stream active at RF" << m_rfSampleRate
                 << "Hz (DSP" << m_sampleRate << "Hz, decimate by" << m_decimRatio
                 << "), freq:" << vfo / 1.0e6 << "MHz";

        // DirectConnection: slots run on the caller's (UI) thread, writing only
        // atomics — safe because runStream() blocks the IO thread's event loop.
        connect(set, &Settings::sampleRateChanged, this, &SoapySDRDataSource::setSampleRate, Qt::DirectConnection);
        // vfoFrequencyChanged carries (mode, rx, frequency) — adapt with a lambda
        // so the slot receives the correct rx and frequency values.
        connect(set, &Settings::vfoFrequencyChanged, this,
                [this](int mode, int rx, qint64 frequency) {
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
        connect(set, &Settings::soapyTxAntennaChanged, this,
                [this](const QString &antenna) {
                    if (m_device && m_txCapable && !antenna.isEmpty()) {
                        try {
                            m_device->setAntenna(SOAPY_SDR_TX, 0, antenna.toStdString());
                            qDebug() << "SoapySDR TX: antenna changed to" << antenna;
                        } catch (const std::exception &e) {
                            qWarning() << "SoapySDR TX: setAntenna failed:" << e.what();
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
        connect(set, &Settings::driveLevelChanged, this,
                [applySoapyTxDrive](int value) {
                    applySoapyTxDrive(value);
                }, Qt::DirectConnection);
        connect(set, &Settings::soapyAutoCalibrateChanged, this,
                [this](bool enabled) { applyLimeAutoCalibrate(enabled); },
                Qt::DirectConnection);
        connect(set, &Settings::radioStateChanged, this,
                [this](RadioState state) {
                    m_txActive.store(state == RadioState::MOX || state == RadioState::TUNE,
                                     std::memory_order_release);
                    m_radioStateValue.store(static_cast<int>(state), std::memory_order_release);
                    if (state == RadioState::MOX || state == RadioState::TUNE)
                        m_txDebugPrimed = false;
                    if (state == RadioState::RX)
                        clearTxIqRing();
                }, Qt::DirectConnection);

        qDebug() << "[SoapySDR] init() complete — signals connected";

    } catch (const std::exception &ex) {
        qCritical() << "SoapySDRDataSource init error:" << ex.what();
    }

    publishWidebandFrequencyRange();
}

void SoapySDRDataSource::clearTxIqRing() {
    QMutexLocker lock(&m_txIqMutex);
    m_txIqRing.clear();
}

void SoapySDRDataSource::configureTxSampleRate() {
    if (!m_device || !m_txCapable)
        return;

    const int savedRxRate = m_rfSampleRate;
    try {
        m_txSampleRate = kTxIqSampleRate;
        m_device->setSampleRate(SOAPY_SDR_TX, 0, m_txSampleRate);
        const double rxAfter = m_device->getSampleRate(SOAPY_SDR_RX, 0);
        if (std::abs(rxAfter - savedRxRate) > std::max(1.0, savedRxRate * 0.01)) {
            // Lime and similar devices couple RX/TX rates — keep RF rate and upsample WDSP IQ.
            m_device->setSampleRate(SOAPY_SDR_RX, 0, savedRxRate);
            m_txSampleRate = savedRxRate;
            m_device->setSampleRate(SOAPY_SDR_TX, 0, m_txSampleRate);
            qDebug() << "SoapySDR TX: RX/TX rates coupled; using RF" << m_txSampleRate
                     << "Hz with WDSP upsample from" << kTxIqSampleRate << "Hz";
        } else {
            qDebug() << "SoapySDR TX: independent TX rate" << m_txSampleRate << "Hz";
        }
    } catch (const std::exception &e) {
        m_txSampleRate = savedRxRate;
        try {
            m_device->setSampleRate(SOAPY_SDR_TX, 0, m_txSampleRate);
        } catch (...) {}
        qWarning() << "SoapySDR TX: configureTxSampleRate fallback to RF rate:" << e.what();
    }
}

void SoapySDRDataSource::drainSoapyTxIqQueue() {
    while (!io->soapy_tx_iq_queue.isEmpty()) {
        const QVector<float> block = io->soapy_tx_iq_queue.dequeue();
        QMutexLocker lock(&m_txIqMutex);

        if (m_txResampler) {
            unsigned int num_written;
            // Up-sample from DSP rate (48k) to RF TX rate.
            resamp_crcf_execute_block(m_txResampler, (liquid_float_complex*)block.data(), block.size() / 2, m_txResampOut, &num_written);
            float* outPtr = (float*)m_txResampOut;
            for (unsigned int i = 0; i < num_written * 2; ++i) {
                m_txIqRing.append(outPtr[i]);
            }
        } else {
            m_txIqRing += block;
        }

        // Keep ~1 s of TX IQ at the hardware TX rate to avoid excessive memory use.
        const int maxFloats = m_txSampleRate * 2;
        if (m_txIqRing.size() > maxFloats)
            m_txIqRing.remove(0, m_txIqRing.size() - maxFloats);
    }
}

bool SoapySDRDataSource::fillTxBufferFromRing(float *txBuff, int numComplexSamples) {
    const int needFloats = numComplexSamples * 2;
    QMutexLocker lock(&m_txIqMutex);

    if (m_txIqRing.size() < needFloats)
        return false;

    for (int i = 0; i < needFloats; ++i)
        txBuff[i] = m_txIqRing.at(i);
    
    m_txIqRing.remove(0, needFloats);
    return true;
}

void SoapySDRDataSource::stop() {
    m_stopped = true;
    clearTxIqRing();
    if (m_device && m_rxStream) {
        m_device->deactivateStream(m_rxStream);
        m_device->closeStream(m_rxStream);
        m_rxStream = nullptr;
    }
    if (m_device && m_txStream) {
        m_device->deactivateStream(m_txStream);
        m_device->closeStream(m_txStream);
        m_txStream = nullptr;
    }
    if (m_device) {
        SoapySDR::Device::unmake(m_device);
        m_device = nullptr;
    }
    emit widebandSpectrumReset();
}

void SoapySDRDataSource::runStream() {
    if (!m_device || !m_rxStream) return;

    const size_t numSamples = 1024; // Match cudaSDR's BUFFER_SIZE
    std::vector<float> buff(numSamples * 2); // complex samples
    std::vector<float> txBuff(numSamples * 2, 0.0f);
    void *buffs[] = {buff.data()};
    void *txBuffs[] = {txBuff.data()};

    // Output buffering: keep fixed 1024-sample blocks for the downstream path.
    std::vector<float> outBuff(numSamples * 2, 0.0f);
    int outFill = 0;

    m_stopped = false;
    uint32_t packetCount = 0;
    m_lastKnownVfo = set->getVfoFrequency(0); // baseline so first poll doesn't retune
    QElapsedTimer txDebugTimer;
    txDebugTimer.start();
    QElapsedTimer txFreqSyncTimer;
    txFreqSyncTimer.start();

    while (!m_stopped) {
        // Poll Settings for VFO changes — reliable fallback for when the signal
        // connection from init() didn't fire (e.g. init() threw before connect()).
        {
            qint64 polledVfo = set->getVfoFrequency(0);
            if (polledVfo != m_lastKnownVfo) {
                m_lastKnownVfo = polledVfo;
                qint64 clamped = polledVfo;
                if (m_minFrequency > 0 && clamped < m_minFrequency) clamped = m_minFrequency;
                if (m_maxFrequency > 0 && clamped > m_maxFrequency) clamped = m_maxFrequency;
                m_pendingFreq.store(static_cast<double>(clamped), std::memory_order_relaxed);
                m_freqPending.store(true, std::memory_order_release);
            }
        }

        // Apply pending sample-rate change (requires stream restart)
        if (m_sampleRatePending.load(std::memory_order_acquire)) {
            m_sampleRatePending.store(false, std::memory_order_relaxed);
            const int newDspRate = m_pendingDspSampleRate.load(std::memory_order_relaxed);
            const int newRfRate = m_pendingRfSampleRate.load(std::memory_order_relaxed);
            const int oldDspRate = m_sampleRate;
            const int oldRfRate = m_rfSampleRate;
            const int oldDecimRatio = m_decimRatio;
            try {
                if (m_rxStream) {
                    m_device->deactivateStream(m_rxStream);
                    m_device->closeStream(m_rxStream);
                    m_rxStream = nullptr;
                }
                m_device->setSampleRate(SOAPY_SDR_RX, 0, newRfRate);
                m_rfSampleRate = newRfRate;
                m_sampleRate = newDspRate;
                syncRfRateFromHardware(m_sampleRate);
                publishWidebandFrequencyRange();

                setupResamplers(m_rfSampleRate, m_sampleRate, m_txSampleRate, kTxIqSampleRate);

                {
                    QMutexLocker lock(&io->mutex);
                    io->soapyInputSampleRate = m_sampleRate;
                }
                applyBandwidthForRfRate(m_rfSampleRate);
                if (m_rxResampler) resamp_crcf_reset(m_rxResampler);
                if (m_txResampler) resamp_crcf_reset(m_txResampler);
                outFill = 0;
                if (!restartRxStream()) {
                    qWarning() << "SoapySDRDataSource: stream restart failed after sample rate change";
                    m_stopped = true;
                    break;
                }
                if (m_txCapable) {
                    configureTxSampleRate();
                    restartTxStream();
                }
                qDebug() << "SoapySDRDataSource: Sample rate applied:" << m_rfSampleRate
                         << "Hz (WDSP input-rate conversion to DSP" << m_sampleRate << "Hz)";
            } catch (const std::exception &e) {
                qWarning() << "SoapySDRDataSource: setSampleRate apply failed:" << e.what()
                           << "requested DSP" << newDspRate << "RF" << newRfRate
                           << "- restoring previous DSP" << oldDspRate << "RF" << oldRfRate;

                // Keep streaming at the previous known-good rate instead of stopping.
                try {
                    m_decimRatio = oldDecimRatio;
                    m_sampleRate = oldDspRate;
                    m_rfSampleRate = oldRfRate;
                    m_device->setSampleRate(SOAPY_SDR_RX, 0, oldRfRate);
                    syncRfRateFromHardware(m_sampleRate);
                    setupResamplers(m_rfSampleRate, m_sampleRate, m_txSampleRate, kTxIqSampleRate);
                    if (m_rxResampler) resamp_crcf_reset(m_rxResampler);
                    if (m_txResampler) resamp_crcf_reset(m_txResampler);
                    {
                        QMutexLocker lock(&io->mutex);
                        io->soapyInputSampleRate = m_sampleRate;
                    }
                    applyBandwidthForRfRate(m_rfSampleRate);
                    publishWidebandFrequencyRange();
                    outFill = 0;
                    if (!restartRxStream()) {
                        qWarning() << "SoapySDRDataSource: restore restart failed";
                        m_stopped = true;
                        break;
                    }
                    if (m_txCapable) {
                        configureTxSampleRate();
                        restartTxStream();
                    }
                } catch (const std::exception &restoreErr) {
                    qWarning() << "SoapySDRDataSource: restore failed:" << restoreErr.what();
                    m_stopped = true;
                    break;
                }
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
            publishWidebandSpectrum(buff.data(), ret);

            if (m_rxResampler && m_rxResampIn && m_rxResampOut) {
                unsigned int num_written;
                // Read 1024 complex samples into the resampler. 
                // We cast the interleaved float buffer to liquid_float_complex.
                resamp_crcf_execute_block(m_rxResampler, (liquid_float_complex*)buff.data(), ret, m_rxResampOut, &num_written);
                
                if (num_written > 0) {
                    float* outPtr = (float*)m_rxResampOut;
                    for (unsigned int i = 0; i < num_written; ++i) {
                        outBuff[outFill * 2]     = outPtr[i * 2];
                        outBuff[outFill * 2 + 1] = outPtr[i * 2 + 1];

                        if (++outFill == static_cast<int>(numSamples)) {
                            // Half-duplex: do not feed RX WDSP while MOX (TX IQ uses timer).
                            const bool halfDuplexTx =
                                m_txActive.load(std::memory_order_acquire)
                                && !set->getTxFullDuplex();
                            if (!halfDuplexTx) {
                                QVector<float> out(numSamples * 2);
                                std::copy(outBuff.begin(), outBuff.end(), out.begin());
                                io->soapy_iq_queue.enqueue(out);
                                emit readydata();
                            }
                            outFill = 0;
                        }
                    }
                }
            }
        } else if (ret < 0) {
            const int err = ret;
            if (++m_streamTimeouts >= kStreamTimeoutsBeforeRestart) {
                qWarning() << "SoapySDRDataSource: readStream"
                           << SoapySDR::errToStr(err) << "(" << err << ") — restarting stream";
                m_streamTimeouts = 0;
                if (restartRxStream()) {
                    if (m_rxResampler) resamp_crcf_reset(m_rxResampler);
                    outFill = 0;
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
                if (m_txCapable) {
                    m_device->setFrequency(SOAPY_SDR_TX, 0, freq);
                    m_lastTxSetFrequency = static_cast<qint64>(freq);
                }
                publishWidebandFrequencyRange();
            } catch (const std::exception& e) {
                qWarning() << "SoapySDRDataSource: setFrequency failed:" << e.what();
            }
        }

        // TX after RX so MOX never stalls the receive stream or DSP path.
        if (m_txCapable && m_txActive.load(std::memory_order_acquire)) {
            if (!m_txStream && !restartTxStream()) {
                m_txErrorCount++;
            } else if (m_txStream) {
                if (txFreqSyncTimer.elapsed() > 200) {
                    try {
                        qint64 targetTxHz = static_cast<qint64>(std::llround(m_pendingFreq.load(std::memory_order_relaxed)));
                        const int rx = set->getCurrentReceiver();
                        if (targetTxHz <= 0)
                            targetTxHz = set->getVfoFrequency(rx);
                        if (targetTxHz <= 0)
                            targetTxHz = set->getCtrFrequency(rx);
                        if (targetTxHz > 0 && std::llabs(targetTxHz - m_lastTxSetFrequency) > 1) {
                            m_device->setFrequency(SOAPY_SDR_TX, 0, static_cast<double>(targetTxHz));
                            m_lastTxSetFrequency = targetTxHz;
                        }
                    } catch (const std::exception& e) {
                        ++m_txErrorCount;
                        qWarning() << "SoapySDR TX frequency sync failed:" << e.what();
                    }
                    txFreqSyncTimer.restart();
                }

                const RadioState currentState = static_cast<RadioState>(m_radioStateValue.load(std::memory_order_acquire));
                const int txQueueDepth = io->soapy_tx_iq_queue.count();
                bool txReady = false;
                if (currentState == RadioState::TUNE) {
                    const float phaseStep = 2.0f * static_cast<float>(M_PI) * 1000.0f
                                          / static_cast<float>(qMax(1, m_txSampleRate));
                    const std::complex<float> w(std::cos(phaseStep), std::sin(phaseStep));
                    std::complex<float> ph = m_txTonePhase;
                    const float amp = tuneToneAmplitudeFromDrive(set->getDriveLevel());
                    for (size_t i = 0; i < numSamples; ++i) {
                        txBuff[2 * i] = amp * ph.real();
                        txBuff[2 * i + 1] = amp * ph.imag();
                        ph *= w;
                    }
                    m_txTonePhase = ph;
                    txReady = true;
                } else {
                    drainSoapyTxIqQueue();
                    txReady = fillTxBufferFromRing(txBuff.data(), static_cast<int>(numSamples));
                }

                int txRingFloats = 0;
                {
                    QMutexLocker lock(&m_txIqMutex);
                    txRingFloats = m_txIqRing.size();
                }

                if (txDebugTimer.elapsed() > 1000 || !m_txDebugPrimed) {
                    RadioState rs = static_cast<RadioState>(m_radioStateValue.load(std::memory_order_acquire));
                    qDebug() << "SoapySDR TX debug: state =" << static_cast<int>(rs)
                             << "txReady =" << txReady
                             << "txQueue =" << txQueueDepth
                             << "txRingFloats =" << txRingFloats
                             << "samples =" << static_cast<int>(numSamples)
                             << "txRate =" << m_txSampleRate;
                    if (txDebugTimer.elapsed() > 1000)
                        txDebugTimer.restart();
                    m_txDebugPrimed = true;
                }

                if (txReady) {
                    int txFlags = 0;
                    long long txTimeNs = 0;
                    const int txRet = m_device->writeStream(m_txStream, txBuffs,
                                                            static_cast<int>(numSamples),
                                                            txFlags, txTimeNs, 100000);
                    if (txRet < 0) {
                        if (txRet == SOAPY_SDR_UNDERFLOW) {
                            ++m_txUnderrunCount;
                        } else {
                            ++m_txErrorCount;
                        }
                    }
                    if (!m_txLogTimer.isValid())
                        m_txLogTimer.start();
                    if (m_txLogTimer.elapsed() > 5000 && (m_txUnderrunCount > 0 || m_txErrorCount > 0)) {
                        qWarning() << "SoapySDRDataSource TX stats: underruns =" << m_txUnderrunCount
                                   << "errors =" << m_txErrorCount;
                        m_txUnderrunCount = 0;
                        m_txErrorCount = 0;
                        m_txLogTimer.restart();
                    }
                } else if (currentState != RadioState::TUNE) {
                    ++m_txUnderrunCount;
                }
            }
        }

        // Close TX stream as soon as we leave TX — stops LimeSDR carrier emission.
        if (m_txCapable && m_txStream && !m_txActive.load(std::memory_order_acquire)) {
            try {
                m_device->deactivateStream(m_txStream);
                m_device->closeStream(m_txStream);
            } catch (...) {}
            m_txStream = nullptr;
            qDebug() << "SoapySDRDataSource: TX stream closed (RX mode)";
        }
    }
}

// Runs on the UI thread (Qt::DirectConnection) — must only write atomics.
void SoapySDRDataSource::setSampleRate(int value) {
    const RfRatePlan plan = chooseRfSampleRate(value);
    m_minSampleRate = plan.effectiveMinHz;
    m_pendingDspSampleRate.store(value, std::memory_order_relaxed);
    m_pendingDecimRatio.store(plan.decimRatio, std::memory_order_relaxed);
    m_pendingRfSampleRate.store(plan.rfSampleRate, std::memory_order_relaxed);
    m_sampleRatePending.store(true, std::memory_order_release);
    qDebug() << "SoapySDRDataSource: DSP rate" << value << "Hz queued (RF"
             << plan.rfSampleRate << "Hz, WDSP converts RF->DSP)";
}

// Runs on the UI thread (Qt::DirectConnection) — must only write atomics.
void SoapySDRDataSource::setFrequency(int rx, qint64 frequency) {
    Q_UNUSED(rx);
    qint64 clamped = frequency;
    if (m_minFrequency > 0 && clamped < m_minFrequency) clamped = m_minFrequency;
    if (m_maxFrequency > 0 && clamped > m_maxFrequency) clamped = m_maxFrequency;
    m_pendingFreq.store(static_cast<double>(clamped), std::memory_order_relaxed);
    m_freqPending.store(true, std::memory_order_release);
}

void SoapySDRDataSource::setupResamplers(int rxRfRate, int rxDspRate, int txRfRate, int txDspRate) {
    // RX Resampler (RF -> DSP)
    if (m_rxResampler) { resamp_crcf_destroy(m_rxResampler); m_rxResampler = nullptr; }
    if (m_rxResampIn) { delete[] (float*)m_rxResampIn; m_rxResampIn = nullptr; }
    if (m_rxResampOut) { delete[] (float*)m_rxResampOut; m_rxResampOut = nullptr; }

    if (rxRfRate > 0 && rxDspRate > 0 && std::abs(rxRfRate - rxDspRate) > 1) {
        const float ratio = static_cast<float>(rxDspRate) / static_cast<float>(rxRfRate);
        unsigned int m = 13;
        float as = 60.0f;
        float fc = 0.40f * ratio; // cutoff normalized to input rate
        m_rxResampler = resamp_crcf_create(ratio, m, fc, as, 32);
        m_rxResampIn = (liquid_float_complex*)new float[1024 * 2];
        m_rxResampOut = (liquid_float_complex*)new float[2048 * 2];
        qDebug() << "SoapySDRDataSource: RX resampler configured for ratio" << ratio
                 << "(" << rxRfRate << "->" << rxDspRate << "Hz)";
    }

    // TX Resampler (DSP -> RF)
    if (m_txResampler) { resamp_crcf_destroy(m_txResampler); m_txResampler = nullptr; }
    if (m_txResampIn) { delete[] (float*)m_txResampIn; m_txResampIn = nullptr; }
    if (m_txResampOut) { delete[] (float*)m_txResampOut; m_txResampOut = nullptr; }

    if (txRfRate > 0 && txDspRate > 0 && std::abs(txRfRate - txDspRate) > 1) {
        const float ratio = static_cast<float>(txRfRate) / static_cast<float>(txDspRate);
        unsigned int m = 13;
        float as = 60.0f;
        float fc = 0.40f; // cutoff normalized to input rate
        m_txResampler = resamp_crcf_create(ratio, m, fc, as, 32);
        m_txResampIn = (liquid_float_complex*)new float[1024 * 2];
        const int maxOut = static_cast<int>(std::ceil(1024 * ratio)) + 64;
        m_txResampOut = (liquid_float_complex*)new float[maxOut * 2];
        qDebug() << "SoapySDRDataSource: TX resampler configured for ratio" << ratio
                 << "(" << txDspRate << "->" << txRfRate << "Hz)";
    }
}

#endif // HAVE_SOAPYSDR
