#include "RttyDemodulator.h"
#include "RttyAutoClassifier.h"
#include "RttyBaudot.h"
#include <QMetaType>
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

RttyDemodulator::RttyDemodulator(int rxId, QObject *parent)
    : QObject(parent)
    , m_rxId(rxId)
{
    m_classifier = new RttyAutoClassifier(m_rxId, this);
    m_classifier->setEnabled(m_autoDetect);

    connect(m_classifier, &RttyAutoClassifier::shiftDetected, this, [this](float shiftHz, float centerFreqHz) {
        if (m_autoDetect) {
            setShiftHz(shiftHz);
            setCenterFreqHz(centerFreqHz);
            if (RttyBaudot::isWeatherShiftHz(shiftHz) && qAbs(m_baudRate - 45.4545f) < 1.0f)
                setBaudRate(50.0f);
            emit autoParametersDetected(m_shiftHz, m_centerFreqHz, m_baudRate);
        }
    });

    connect(m_classifier, &RttyAutoClassifier::baudRateDetected, this, [this](float baudRate) {
        if (m_autoDetect) {
            // Weather/nav 425–450 Hz is 50 baud. Slipped crossings look like 45.45
            // and must not override a locked weather channel.
            if (RttyBaudot::isWeatherShiftHz(m_shiftHz) && qAbs(baudRate - 50.0f) > 1.0f)
                return;
            setBaudRate(baudRate);
            emit autoParametersDetected(m_shiftHz, m_centerFreqHz, m_baudRate);
        }
    });

    initDecimator();
    initMatchedFilters();
    m_scopeXs.reserve(SCOPE_BATCH);
    m_scopeYs.reserve(SCOPE_BATCH);
    qRegisterMetaType<QVector<float>>();
}

void RttyDemodulator::setEnabled(bool enable) {
    if (m_enabled != enable) {
        m_enabled = enable;
        if (!m_enabled) {
            reset();
        }
    }
}

void RttyDemodulator::setBaudRate(float baud) {
    if (baud > 10.0f && baud < 300.0f) {
        m_baudRate = baud;
        m_samplesPerBit = static_cast<float>(DECIM_RATE) / m_baudRate;
        initMatchedFilters();
        resetUart();
    }
}

void RttyDemodulator::setShiftHz(float shift) {
    if (shift >= 50.0f && shift <= 1000.0f) {
        m_shiftHz = shift;
        m_trackedOffsetHz = 0.0f;
        initMatchedFilters();
        resetUart();
    }
}

void RttyDemodulator::setCenterFreqHz(float centerFreq) {
    if (centerFreq >= 300.0f && centerFreq <= 4000.0f) {
        if (std::abs(m_centerFreqHz - centerFreq) > 1.0f) {
            m_trackedOffsetHz = 0.0f;
        }
        m_centerFreqHz = centerFreq;
    }
}

void RttyDemodulator::setReversePolarity(bool reverse) {
    if (m_reversePolarity == reverse)
        return;
    m_reversePolarity = reverse;
    resetUart();
}

void RttyDemodulator::setAfcEnabled(bool enabled) {
    m_afcEnabled = enabled;
    if (!m_afcEnabled) {
        m_trackedOffsetHz = 0.0f;
    }
}

void RttyDemodulator::setAutoDetectEnabled(bool enabled) {
    if (m_autoDetect != enabled) {
        m_autoDetect = enabled;
        if (m_classifier) {
            m_classifier->setEnabled(enabled);
        }
    }
}

void RttyDemodulator::setUsbMode(bool usb) {
    // USB/LSB is display-only. Mark/space live in the audio domain
    // (amateur high tones: Mark = center - shift/2) and must not flip.
    m_isUsbMode = usb;
}

bool RttyDemodulator::debugLoggingActive() const {
    return m_debugLogging || qEnvironmentVariableIsSet("CUDASDR_RTTY_DEBUG");
}

float RttyDemodulator::markFreqHz() const {
    const float offset = m_reversePolarity ? (m_shiftHz * 0.5f) : (-m_shiftHz * 0.5f);
    return m_centerFreqHz + m_trackedOffsetHz + offset;
}

float RttyDemodulator::spaceFreqHz() const {
    const float offset = m_reversePolarity ? (-m_shiftHz * 0.5f) : (m_shiftHz * 0.5f);
    return m_centerFreqHz + m_trackedOffsetHz + offset;
}

void RttyDemodulator::clearText() {
    m_recentText.clear();
    emit textUpdated(m_rxId, m_recentText);
}

void RttyDemodulator::reset() {
    m_ddcPhase = 0.0f;
    m_decimPhase = 0;
    std::fill(m_firState.begin(), m_firState.end(), std::complex<float>(0.0f, 0.0f));
    std::fill(m_cpxHistory.begin(), m_cpxHistory.end(), std::complex<float>(0.0f, 0.0f));
    m_historyIdx = 0;
    m_trackedOffsetHz = 0.0f;

    m_dpllPhase = 0.0f;
    m_dpllFreq = m_baudRate / static_cast<float>(DECIM_RATE);
    m_lastDiscriminator = 0.0f;
    m_lastShortDiff = 0.0f;
    m_sampleCounter = 0;
    m_lastZeroCrossingSample = 0;
    m_baudLastCrossingSample = 0;
    m_markDwellSamples = 0;
    m_asyncBitsLeft = 0;
    m_asyncCountdown = 0.0f;

    if (m_classifier) {
        m_classifier->reset();
    }

    m_noiseVariance = 0.001f;
    m_signalPower = 0.001f;
    m_markEnergySmooth = 0.0f;
    m_spaceEnergySmooth = 0.0f;
    m_currentSnrDb = 0.0f;
    m_locked = false;

    resetUart();

    m_scopeDecim = 0;
    m_scopeMarkBp.resetState();
    m_scopeSpaceBp.resetState();
    m_scopeXs.clear();
    m_scopeYs.clear();
    m_lastScopeXs.clear();
    m_lastScopeYs.clear();
    emit scopeFrameReady(m_rxId, m_lastScopeXs, m_lastScopeYs);
}

void RttyDemodulator::resetUart() {
    m_samplesPerBit = static_cast<float>(DECIM_RATE) / qMax(m_baudRate, 1.0f);
    m_uartState = UartIdle;
    m_uartCounter = 0.0f;
    m_heldBit = true;
    m_noiseFloor = 0.0f;
    m_prevDataBit = true;
    m_haveTransition = false;
    m_lastTransitionSample = 0;
    m_bitCount = 0;
    m_shiftReg = 0;
    m_figsMode = false;
}

void RttyDemodulator::initDecimator() {
    // 48-tap symmetric FIR low-pass filter designed for 48 kHz -> 2 kHz decimation.
    // Cutoff frequency 750 Hz (normalized fc = 750 / 48000 ~ 0.015625).
    // Passes all standard shifts (170, 200, 425, 450, 850 Hz) and keying sidebands
    // without passband attenuation, while providing anti-aliasing rejection.
    const int numTaps = 48;
    m_firCoeffs.resize(numTaps);
    m_firState.resize(numTaps);
    std::fill(m_firState.begin(), m_firState.end(), std::complex<float>(0.0f, 0.0f));

    const float fc = 750.0f / 48000.0f;
    float sum = 0.0f;
    for (int i = 0; i < numTaps; ++i) {
        const float n = static_cast<float>(i - (numTaps - 1) / 2.0f);
        float h = 0.0f;
        if (std::abs(n) < 1e-5f) {
            h = 2.0f * fc;
        } else {
            h = std::sin(2.0f * static_cast<float>(M_PI) * fc * n) / (static_cast<float>(M_PI) * n);
        }
        // Hamming window
        const float w = 0.54f - 0.46f * std::cos(2.0f * static_cast<float>(M_PI) * static_cast<float>(i) / static_cast<float>(numTaps - 1));
        m_firCoeffs[i] = h * w;
        sum += m_firCoeffs[i];
    }
    // Normalize filter gain to unity
    if (sum > 0.0f) {
        for (int i = 0; i < numTaps; ++i) {
            m_firCoeffs[i] /= sum;
        }
    }
}

void RttyDemodulator::initMatchedFilters() {
    // Number of samples per symbol at 2000 Hz complex sample rate
    // 2000 / 45.454545 = 44.0 samples
    m_samplesPerBit = static_cast<float>(DECIM_RATE) / qMax(m_baudRate, 1.0f);
    m_symSamples = qMax(4, qRound(m_samplesPerBit));
    m_cpxHistory.resize(m_symSamples);
    std::fill(m_cpxHistory.begin(), m_cpxHistory.end(), std::complex<float>(0.0f, 0.0f));
    m_historyIdx = 0;

    m_markRef.resize(m_symSamples);
    m_spaceRef.resize(m_symSamples);

    // Amateur high tones in the audio domain after WDSP, independent of USB/LSB:
    // Mark (idle/stop/bit 1) = center - shift/2 (2125 Hz), Space (start/bit 0) = center + shift/2 (2295 Hz).
    // Reverse polarity swaps the discriminator, not these filter tones.
    const float markFreq = -m_shiftHz * 0.5f;
    const float spaceFreq = m_shiftHz * 0.5f;

    const float markPhaseStep = 2.0f * static_cast<float>(M_PI) * markFreq / static_cast<float>(DECIM_RATE);
    const float spacePhaseStep = 2.0f * static_cast<float>(M_PI) * spaceFreq / static_cast<float>(DECIM_RATE);

    for (int k = 0; k < m_symSamples; ++k) {
        // Complex conjugate reference for matched filter correlation:
        // integrate x(k) * e^(-j 2pi f k)
        const float pm = markPhaseStep * static_cast<float>(k);
        const float ps = spacePhaseStep * static_cast<float>(k);
        m_markRef[k] = std::complex<float>(std::cos(pm), -std::sin(pm));
        m_spaceRef[k] = std::complex<float>(std::cos(ps), -std::sin(ps));
    }

    m_dpllFreq = m_baudRate / static_cast<float>(DECIM_RATE);
}

void RttyDemodulator::ScopeBiquad::setBandpass(float sampleRate, float f0, float q)
{
    const float w0 = 2.0f * static_cast<float>(M_PI) * f0 / qMax(sampleRate, 1.0f);
    const float cosW0 = std::cos(w0);
    const float sinW0 = std::sin(w0);
    const float alpha = sinW0 / (2.0f * qMax(q, 1.0f));
    const float a0 = 1.0f + alpha;
    b0 = alpha / a0;
    b1 = 0.0f;
    b2 = -alpha / a0;
    a1 = (-2.0f * cosW0) / a0;
    a2 = (1.0f - alpha) / a0;
    z1 = 0.0f;
    z2 = 0.0f;
}

float RttyDemodulator::ScopeBiquad::process(float x)
{
    const float y = b0 * x + z1;
    z1 = b1 * x - a1 * y + z2;
    z2 = b2 * x - a2 * y;
    return y;
}

void RttyDemodulator::ScopeBiquad::resetState()
{
    z1 = 0.0f;
    z2 = 0.0f;
}

void RttyDemodulator::updateScopeFilters(int sampleRate)
{
    const float markHz = qBound(200.0f, markFreqHz(), 4000.0f);
    const float spaceHz = qBound(200.0f, spaceFreqHz(), 4000.0f);
    if (sampleRate == m_scopeSampleRate
            && std::abs(markHz - m_scopeMarkHz) < 2.0f
            && std::abs(spaceHz - m_scopeSpaceHz) < 2.0f)
        return;

    m_scopeSampleRate = sampleRate;
    m_scopeMarkHz = markHz;
    m_scopeSpaceHz = spaceHz;
    const float sr = static_cast<float>(qMax(sampleRate, 1));
    m_scopeMarkBp.setBandpass(sr, markHz, qMax(markHz / SCOPE_BW_HZ, 1.0f));
    m_scopeSpaceBp.setBandpass(sr, spaceHz, qMax(spaceHz / SCOPE_BW_HZ, 1.0f));
}

void RttyDemodulator::processScopeSample(float sample)
{
    const float x = m_scopeMarkBp.process(sample);
    const float y = m_scopeSpaceBp.process(sample);
    ++m_scopeDecim;
    if (m_scopeDecim < SCOPE_DECIMATE)
        return;
    m_scopeDecim = 0;
    m_scopeXs.append(x);
    m_scopeYs.append(y);
    if (m_scopeXs.size() < SCOPE_BATCH)
        return;

    m_lastScopeXs = m_scopeXs;
    m_lastScopeYs = m_scopeYs;
    emit scopeFrameReady(m_rxId, m_scopeXs, m_scopeYs);
    m_scopeXs.clear();
    m_scopeYs.clear();
    m_scopeXs.reserve(SCOPE_BATCH);
    m_scopeYs.reserve(SCOPE_BATCH);
}

void RttyDemodulator::processAudio(const float *samples, int count, int sampleRate) {
    if (!m_enabled || !samples || count <= 0)
        return;

    if (m_autoDetect && m_classifier) {
        m_classifier->feedAudio(samples, count, sampleRate);
    }

    if (debugLoggingActive()) {
        static int audioLogCounter = 0;
        if (++audioLogCounter >= 50) {
            audioLogCounter = 0;
            float sumSq = 0.0f;
            for (int i = 0; i < count; ++i) {
                sumSq += samples[i] * samples[i];
            }
            const float rms = std::sqrt(sumSq / static_cast<float>(qMax(1, count)));
            if (rms < 1e-4f) {
                qWarning("[RTTY rx%d DEBUG] Audio input level extremely low (RMS=%.6f). Check slice volume / audio routing!", m_rxId, rms);
            } else if (rms > 0.95f) {
                qWarning("[RTTY rx%d DEBUG] Audio input is CLIPPING (RMS=%.3f)!", m_rxId, rms);
            } else {
                qDebug("[RTTY rx%d DEBUG] Audio OK: RMS=%.3f, SNR=%.1fdB, Baud=%.2f, Lock=%s, AFC=%+.1fHz, Sideband=%s, Polarity=%s",
                       m_rxId, rms, m_currentSnrDb, m_baudRate, m_locked ? "YES" : "NO", m_trackedOffsetHz,
                       m_isUsbMode ? "USB" : "LSB", m_reversePolarity ? "REV" : "NOR");
            }
        }
    }

    // Direct Digital Downconversion (DDC) to DC + FIR Decimation
    const float ddcFreq = m_centerFreqHz + m_trackedOffsetHz;
    const float ddcStep = 2.0f * static_cast<float>(M_PI) * ddcFreq / static_cast<float>(sampleRate);

    const int numTaps = m_firCoeffs.size();
    updateScopeFilters(sampleRate);

    for (int i = 0; i < count; ++i) {
        const float inSample = samples[i];
        processScopeSample(inSample);

        // 1. Complex heterodyne to DC: x(n) * e^(-j * phase)
        const float cosVal = std::cos(m_ddcPhase);
        const float sinVal = std::sin(m_ddcPhase);
        const std::complex<float> cpxSample(inSample * cosVal, -inSample * sinVal);

        m_ddcPhase += ddcStep;
        if (m_ddcPhase >= 2.0f * static_cast<float>(M_PI)) {
            m_ddcPhase -= 2.0f * static_cast<float>(M_PI);
        }

        // 2. Push to FIR delay line
        for (int k = numTaps - 1; k > 0; --k) {
            m_firState[k] = m_firState[k - 1];
        }
        m_firState[0] = cpxSample;

        // 3. Decimate by 24 (48 kHz -> 2 kHz)
        ++m_decimPhase;
        if (m_decimPhase >= DECIM_FACTOR) {
            m_decimPhase = 0;

            // Compute convolution sum for decimated sample
            std::complex<float> filtered(0.0f, 0.0f);
            for (int k = 0; k < numTaps; ++k) {
                filtered += m_firState[k] * m_firCoeffs[k];
            }

            processDecimatedComplexSample(filtered);
        }
    }
}

void RttyDemodulator::processDecimatedComplexSample(const std::complex<float> &cpxSample) {
    ++m_sampleCounter;

    // Store in circular history buffer
    m_cpxHistory[m_historyIdx] = cpxSample;
    m_historyIdx = (m_historyIdx + 1) % m_symSamples;

    // Correlate with Mark and Space matched filter references
    std::complex<float> corrMark(0.0f, 0.0f);
    std::complex<float> corrSpace(0.0f, 0.0f);

    int idx = m_historyIdx;
    for (int k = 0; k < m_symSamples; ++k) {
        const std::complex<float> s = m_cpxHistory[idx];
        corrMark += s * m_markRef[k];
        corrSpace += s * m_spaceRef[k];
        idx = (idx + 1 == m_symSamples) ? 0 : idx + 1;
    }

    const float markEnergy = std::norm(corrMark);   // |Mark|^2
    const float spaceEnergy = std::norm(corrSpace); // |Space|^2

    // Apply polarity
    float diff = m_reversePolarity ? (spaceEnergy - markEnergy) : (markEnergy - spaceEnergy);

    // Smooth energy tracking
    m_markEnergySmooth = 0.95f * m_markEnergySmooth + 0.05f * markEnergy;
    m_spaceEnergySmooth = 0.95f * m_spaceEnergySmooth + 0.05f * spaceEnergy;
    m_lastDiscriminator = diff;

    // Short-window discriminator (~6 ms). Independent of the configured baud, so
    // start-bit edges and auto-baud intervals are not smeared by a 44-sample MF.
    constexpr int kShortWin = 12;
    std::complex<float> shortMark(0.0f, 0.0f);
    std::complex<float> shortSpace(0.0f, 0.0f);
    if (m_symSamples >= kShortWin) {
        const float markPhaseStep = 2.0f * static_cast<float>(M_PI)
            * (-m_shiftHz * 0.5f) / static_cast<float>(DECIM_RATE);
        const float spacePhaseStep = 2.0f * static_cast<float>(M_PI)
            * (m_shiftHz * 0.5f) / static_cast<float>(DECIM_RATE);
        const int newest = (m_historyIdx + m_symSamples - 1) % m_symSamples;
        for (int i = 0; i < kShortWin; ++i) {
            const int h = (newest - (kShortWin - 1 - i) + m_symSamples) % m_symSamples;
            const float pm = markPhaseStep * static_cast<float>(i);
            const float ps = spacePhaseStep * static_cast<float>(i);
            const std::complex<float> s = m_cpxHistory[h];
            shortMark += s * std::complex<float>(std::cos(pm), -std::sin(pm));
            shortSpace += s * std::complex<float>(std::cos(ps), -std::sin(ps));
        }
    }
    float shortDiff = m_reversePolarity
        ? (std::norm(shortSpace) - std::norm(shortMark))
        : (std::norm(shortMark) - std::norm(shortSpace));
    const bool shortIsMark = (shortDiff > 0.0f);
    const bool shortWasMark = (m_lastShortDiff > 0.0f);
    const bool shortMarkToSpace = shortWasMark && !shortIsMark && (m_lastShortDiff != 0.0f);

    if (shortIsMark)
        ++m_markDwellSamples;

    if (shortIsMark != shortWasMark && m_lastShortDiff != 0.0f) {
        const int interval = m_sampleCounter - m_baudLastCrossingSample;
        if (m_autoDetect && m_classifier && m_currentSnrDb > 3.0f)
            m_classifier->feedTransition(interval, static_cast<float>(DECIM_RATE));
        m_baudLastCrossingSample = m_sampleCounter;
        m_lastZeroCrossingSample = m_sampleCounter;
    }
    m_lastShortDiff = shortDiff;

    // Hunt on the short edge. The 12-sample window flips ~one window after the
    // true Mark→Space, so wait a full symbol minus that window or the first
    // strobe lands in data0 and stop walks into the next start (~1 bit late).
    const float minStartDwell = static_cast<float>(m_symSamples) * 0.85f;
    const bool hunting = (m_asyncBitsLeft <= 0);
    if (hunting && shortMarkToSpace && m_markDwellSamples >= minStartDwell
        && (m_locked || (markEnergy + spaceEnergy) > 1e-6f)) {
        m_asyncBitsLeft = 7;
        m_asyncCountdown = static_cast<float>(m_symSamples) - static_cast<float>(kShortWin);
        m_dpllPhase = 0.0f;
    }

    if (shortMarkToSpace)
        m_markDwellSamples = 0;

    processUartSample(markEnergy, spaceEnergy);

    if (m_asyncBitsLeft > 0) {
        m_asyncCountdown -= 1.0f;
        if (m_asyncCountdown <= 0.0f) {
            onSymbolStrobe();
            m_asyncCountdown += static_cast<float>(m_symSamples);
        }
    }
}

void RttyDemodulator::onSymbolStrobe() {
    // Current correlation outputs at eye center
    std::complex<float> corrMark(0.0f, 0.0f);
    std::complex<float> corrSpace(0.0f, 0.0f);

    int idx = m_historyIdx;
    for (int k = 0; k < m_symSamples; ++k) {
        const std::complex<float> s = m_cpxHistory[idx];
        corrMark += s * m_markRef[k];
        corrSpace += s * m_spaceRef[k];
        idx = (idx + 1 == m_symSamples) ? 0 : idx + 1;
    }

    const float markMag = std::abs(corrMark);
    const float spaceMag = std::abs(corrSpace);
    const float markE = markMag * markMag;
    const float spaceE = spaceMag * spaceMag;

    // Strobe-based noise & signal estimation:
    // At mid-symbol, the inactive tone contains purely in-band noise without transition ISI.
    const float noiseSample = qMin(markE, spaceE);
    const float sigSample = qMax(markE, spaceE);
    m_noiseVariance = 0.95f * m_noiseVariance + 0.05f * qMax(noiseSample, 1e-6f);
    m_signalPower = 0.95f * m_signalPower + 0.05f * qMax(sigSample, 1e-6f);

    if (m_signalPower > m_noiseVariance * 1.05f) {
        m_currentSnrDb = 10.0f * std::log10((m_signalPower - m_noiseVariance) / m_noiseVariance);
    } else {
        m_currentSnrDb = 0.0f;
    }
    m_locked = (m_currentSnrDb > 0.5f);

    // Automatic Frequency Control (AFC)
    // Tracks transmitter frequency drift and operator tuning errors up to +/- 45 Hz
    if (m_afcEnabled && (m_locked || m_currentSnrDb > 0.0f)) {
        const bool markActive = (markE > spaceE * 1.5f);
        const bool spaceActive = (spaceE > markE * 1.5f);

        if (markActive || spaceActive) {
            const int half = m_symSamples / 2;
            std::complex<float> c1(0.0f, 0.0f);
            std::complex<float> c2(0.0f, 0.0f);
            const auto &ref = markActive ? m_markRef : m_spaceRef;

            int aIdx = m_historyIdx;
            for (int k = 0; k < half; ++k) {
                c1 += m_cpxHistory[aIdx] * ref[k];
                aIdx = (aIdx + 1 == m_symSamples) ? 0 : aIdx + 1;
            }
            for (int k = half; k < m_symSamples; ++k) {
                c2 += m_cpxHistory[aIdx] * ref[k];
                aIdx = (aIdx + 1 == m_symSamples) ? 0 : aIdx + 1;
            }

            const std::complex<float> prod = c2 * std::conj(c1);
            if (std::norm(prod) > 1e-6f) {
                const float dTheta = std::atan2(prod.imag(), prod.real());
                const float freqErr = dTheta * static_cast<float>(DECIM_RATE) / (static_cast<float>(M_PI) * static_cast<float>(m_symSamples));
                if (std::abs(freqErr) < 45.0f) {
                    m_trackedOffsetHz += 0.05f * freqErr;
                    m_trackedOffsetHz = qBound(-75.0f, m_trackedOffsetHz, 75.0f);
                }
            }
        }
    } else if (m_afcEnabled && m_currentSnrDb <= 0.0f) {
        // Slow decay back towards center when signal is completely submerged in noise
        m_trackedOffsetHz *= 0.9995f;
    }

    // Calculate continuous Log-Likelihood Ratio (LLR)
    // Positive LLR = Mark (bit 1), Negative LLR = Space (bit 0)
    float diffE = markE - spaceE;
    if (m_reversePolarity) {
        diffE = -diffE;
    }

    const float llr = diffE / qMax(m_noiseVariance, 1e-6f);

    if (m_asyncBitsLeft > 0)
        --m_asyncBitsLeft;

    RttySoftSymbol sym;
    sym.llr = llr;
    sym.markMag = markMag;
    sym.spaceMag = spaceMag;
    sym.snrDb = m_currentSnrDb;
    sym.sampleIdx = m_sampleCounter;

    if (debugLoggingActive()) {
        static int strobeLogCounter = 0;
        if (++strobeLogCounter >= 20) {
            strobeLogCounter = 0;
            const char *toneStr = (diffE > 0.0f) ? "MARK" : "SPACE";
            qDebug("[RTTY rx%d DEBUG] Strobe: MarkE=%.3f SpaceE=%.3f Tone=%s SNR=%.1fdB Lock=%d LLR=%+.1f AFC=%+.1fHz",
                   m_rxId, markE, spaceE, toneStr, m_currentSnrDb, m_locked, llr, m_trackedOffsetHz);
        }
    }

    emit symbolSampled(sym);

    if ((m_sampleCounter % (m_symSamples * 10)) < m_symSamples) {
        emit toneStatusChanged(m_rxId, markFreqHz(), spaceFreqHz(), m_currentSnrDb, m_locked);
    }
}

void RttyDemodulator::processUartSample(float markEnergy, float spaceEnergy)
{
    float markE = markEnergy;
    float spaceE = spaceEnergy;
    if (m_reversePolarity)
        qSwap(markE, spaceE);

    const float total = markE + spaceE + 1.0e-12f;
    const float disc = (markE - spaceE) / total;

    constexpr float kHyst = 0.06f;
    if (m_heldBit && disc < -kHyst)
        m_heldBit = false;
    else if (!m_heldBit && disc > kHyst)
        m_heldBit = true;
    const bool bit = m_heldBit;

    constexpr float kNoiseAlpha = 0.001f;
    constexpr float kGateSnr = 4.0f;
    if (m_noiseFloor < 1.0e-9f || total < m_noiseFloor * 2.0f)
        m_noiseFloor = m_noiseFloor * (1.0f - kNoiseAlpha) + total * kNoiseAlpha;
    const bool signalPresent = total > m_noiseFloor * kGateSnr;

    switch (m_uartState) {
    case UartIdle:
        if (!bit && signalPresent) {
            m_uartState = UartWaitStartEnd;
            m_uartCounter = m_samplesPerBit * 0.5f;
            m_haveTransition = false;
            m_prevDataBit = false;
        }
        break;

    case UartWaitStartEnd:
        m_uartCounter -= 1.0f;
        if (m_uartCounter <= 0.0f) {
            if (!bit) {
                m_uartState = UartData;
                m_shiftReg = 0;
                m_bitCount = 0;
                m_uartCounter = m_samplesPerBit;
            } else {
                m_uartState = UartIdle;
            }
        }
        break;

    case UartData:
        if (bit != m_prevDataBit) {
            if (m_haveTransition) {
                const float interval = static_cast<float>(
                    m_sampleCounter - m_lastTransitionSample);
                const float nBits = std::round(interval / m_samplesPerBit);
                if (nBits >= 1.0f && nBits <= 6.0f) {
                    const float observed = interval / nBits;
                    const float dev = std::abs(observed - m_samplesPerBit) / m_samplesPerBit;
                    if (dev < 0.05f)
                        m_samplesPerBit = m_samplesPerBit * 0.95f + observed * 0.05f;
                }
            }
            m_lastTransitionSample = m_sampleCounter;
            m_haveTransition = true;
            m_prevDataBit = bit;
        }
        m_uartCounter -= 1.0f;
        if (m_uartCounter <= 0.0f) {
            if (bit)
                m_shiftReg |= static_cast<quint8>(1 << m_bitCount);
            ++m_bitCount;
            m_uartCounter = m_samplesPerBit;
            if (m_bitCount >= 5)
                m_uartState = UartStop;
        }
        break;

    case UartStop:
        m_uartCounter -= 1.0f;
        if (m_uartCounter <= 0.0f) {
            m_uartState = UartIdle;
            if (bit) {
                acceptBaudot(m_shiftReg & 0x1F);
            } else if (debugLoggingActive()) {
                qDebug("[RTTY rx%d UART] drop stop=SPACE code=0x%02X",
                       m_rxId, m_shiftReg & 0x1F);
            }
        }
        break;
    }
}

void RttyDemodulator::acceptBaudot(quint8 code)
{
    code &= 0x1F;
    if (code == 0x1F) {
        m_figsMode = false;
        if (debugLoggingActive())
            qDebug("[RTTY rx%d UART] LTRS", m_rxId);
        return;
    }
    if (code == 0x1B) {
        m_figsMode = true;
        if (debugLoggingActive())
            qDebug("[RTTY rx%d UART] FIGS", m_rxId);
        return;
    }
    if (code == 0x00) {
        if (debugLoggingActive())
            qDebug("[RTTY rx%d UART] NUL figs=%d", m_rxId, m_figsMode ? 1 : 0);
        return;
    }
    if (m_usosEnabled && code == 0x04)
        m_figsMode = false;

    const QChar ch = decodeBaudot(code, m_figsMode);
    const char latin = ch.toLatin1();
    if (ch.isNull() || latin == '\0' || latin == '\a') {
        if (debugLoggingActive())
            qDebug("[RTTY rx%d UART] blank code=0x%02X figs=%d", m_rxId, code, m_figsMode ? 1 : 0);
        return;
    }

    const QString str(ch);
    m_recentText.append(str);
    constexpr int kRecentTextCap = 80 * 1000;
    constexpr int kRecentTextKeep = 80 * 900;
    if (m_recentText.size() > kRecentTextCap)
        m_recentText = m_recentText.right(kRecentTextKeep);
    emit characterDecoded(m_rxId, str);
    emit textUpdated(m_rxId, m_recentText);

    if (debugLoggingActive()) {
        qDebug("[RTTY rx%d UART] %s code=0x%02X figs=%d",
               m_rxId, qPrintable(str == QLatin1String("\r") ? QStringLiteral("CR")
                                 : str == QLatin1String("\n") ? QStringLiteral("LF")
                                 : str == QLatin1String(" ") ? QStringLiteral("SP")
                                 : str),
               code, m_figsMode ? 1 : 0);
    }
}

QChar RttyDemodulator::decodeBaudot(quint8 code, bool figs) {
    return RttyBaudot::decode(code, figs);
}
