#include "RttyDemodulator.h"
#include "RttyAutoClassifier.h"
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Baudot ITA2 / US-TTY lookup tables
static const char LTRS_TABLE[32] = {
    '\0', 'E',  '\n', 'A',  ' ',  'S',  'I',  'U',   // 0 - 7
    '\r', 'D',  'R',  'J',  'N',  'F',  'C',  'K',   // 8 - 15
    'T',  'Z',  'L',  'W',  'H',  'Y',  'P',  'Q',   // 16 - 23
    'O',  'B',  'G',  '\0', 'M',  'X',  'V',  '\0'   // 24 - 31 (27=FIGS, 31=LTRS)
};

static const char FIGS_TABLE[32] = {
    '\0', '3',  '\n', '-',  ' ',  '\'', '8',  '7',   // 0 - 7
    '\r', '$',  '4',  '\a', ',',  '!',  ':',  '(',   // 8 - 15
    '5',  '"',  ')',  '2',  '#',  '6',  '0',  '1',   // 16 - 23
    '9',  '?',  '&',  '\0', '.',  '/',  ';',  '\0'   // 24 - 31
};

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
            emit autoParametersDetected(m_shiftHz, m_centerFreqHz, m_baudRate);
        }
    });

    connect(m_classifier, &RttyAutoClassifier::baudRateDetected, this, [this](float baudRate) {
        if (m_autoDetect) {
            setBaudRate(baudRate);
            emit autoParametersDetected(m_shiftHz, m_centerFreqHz, m_baudRate);
        }
    });

    connect(m_classifier, &RttyAutoClassifier::polarityInversionSuggested, this, [this]() {
        if (m_autoDetect) {
            setReversePolarity(!m_reversePolarity);
            emit polarityInversionDetected(m_reversePolarity);
        }
    });

    initDecimator();
    initMatchedFilters();
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
        initMatchedFilters();
    }
}

void RttyDemodulator::setShiftHz(float shift) {
    if (shift >= 50.0f && shift <= 1000.0f) {
        m_shiftHz = shift;
        initMatchedFilters();
    }
}

void RttyDemodulator::setCenterFreqHz(float centerFreq) {
    if (centerFreq >= 300.0f && centerFreq <= 4000.0f) {
        m_centerFreqHz = centerFreq;
    }
}

void RttyDemodulator::setReversePolarity(bool reverse) {
    m_reversePolarity = reverse;
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

float RttyDemodulator::markFreqHz() const {
    const float offset = m_reversePolarity ? (+m_shiftHz * 0.5f) : (-m_shiftHz * 0.5f);
    return m_centerFreqHz + m_trackedOffsetHz + offset;
}

float RttyDemodulator::spaceFreqHz() const {
    const float offset = m_reversePolarity ? (-m_shiftHz * 0.5f) : (+m_shiftHz * 0.5f);
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

    m_dpllPhase = 0.0f;
    m_dpllFreq = m_baudRate / static_cast<float>(DECIM_RATE);
    m_lastDiscriminator = 0.0f;
    m_sampleCounter = 0;
    m_lastZeroCrossingSample = 0;

    if (m_classifier) {
        m_classifier->reset();
    }

    m_noiseVariance = 0.001f;
    m_markEnergySmooth = 0.0f;
    m_spaceEnergySmooth = 0.0f;
    m_currentSnrDb = 0.0f;
    m_locked = false;

    m_framerState = WaitStart;
    m_bitCount = 0;
    m_shiftReg = 0;
    m_figsMode = false;
    m_consecutiveIdleCount = 0;
}

void RttyDemodulator::initDecimator() {
    // 48-tap symmetric FIR low-pass filter designed for 48 kHz -> 2 kHz decimation
    // Cutoff frequency approx. 350 Hz (normalized fc = 350 / 48000 ~ 0.00729)
    // Provides > 40 dB rejection of frequencies beyond Nyquist of 2 kHz (1000 Hz)
    const int numTaps = 48;
    m_firCoeffs.resize(numTaps);
    m_firState.resize(numTaps);
    std::fill(m_firState.begin(), m_firState.end(), std::complex<float>(0.0f, 0.0f));

    const float fc = 350.0f / 48000.0f;
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
    m_symSamples = qMax(4, qRound(static_cast<float>(DECIM_RATE) / m_baudRate));
    m_cpxHistory.resize(m_symSamples);
    std::fill(m_cpxHistory.begin(), m_cpxHistory.end(), std::complex<float>(0.0f, 0.0f));
    m_historyIdx = 0;

    m_markRef.resize(m_symSamples);
    m_spaceRef.resize(m_symSamples);

    // In complex baseband centered at DC:
    // Mark tone is at -shift/2, Space tone is at +shift/2
    const float markFreq = -m_shiftHz * 0.5f;
    const float spaceFreq = +m_shiftHz * 0.5f;

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

void RttyDemodulator::processAudio(const float *samples, int count, int sampleRate) {
    if (!m_enabled || !samples || count <= 0)
        return;

    if (m_autoDetect && m_classifier) {
        m_classifier->feedAudio(samples, count, sampleRate);
    }

    // Direct Digital Downconversion (DDC) to DC + FIR Decimation
    const float ddcFreq = m_centerFreqHz + m_trackedOffsetHz;
    const float ddcStep = 2.0f * static_cast<float>(M_PI) * ddcFreq / static_cast<float>(sampleRate);

    const int numTaps = m_firCoeffs.size();

    for (int i = 0; i < count; ++i) {
        const float inSample = samples[i];

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

    // Smooth energy tracking for SNR & noise estimation
    m_markEnergySmooth = 0.95f * m_markEnergySmooth + 0.05f * markEnergy;
    m_spaceEnergySmooth = 0.95f * m_spaceEnergySmooth + 0.05f * spaceEnergy;

    // Noise floor estimate: minimum of smoothed tone energies
    const float minE = qMin(m_markEnergySmooth, m_spaceEnergySmooth);
    m_noiseVariance = 0.99f * m_noiseVariance + 0.01f * qMax(minE, 1e-6f);

    const float maxE = qMax(m_markEnergySmooth, m_spaceEnergySmooth);
    m_currentSnrDb = 10.0f * std::log10(qMax(maxE, 1e-6f) / m_noiseVariance);
    m_locked = (m_currentSnrDb > 4.0f);

    // Symbol Timing Recovery (2nd-Order DPLL)
    // Zero-crossing detector on discriminator diff
    const bool zeroCrossing = (diff > 0.0f) != (m_lastDiscriminator > 0.0f);
    m_lastDiscriminator = diff;

    if (zeroCrossing) {
        if (m_autoDetect && m_locked && m_classifier) {
            int interval = m_sampleCounter - m_lastZeroCrossingSample;
            m_classifier->feedTransition(interval, static_cast<float>(DECIM_RATE));
        }
        m_lastZeroCrossingSample = m_sampleCounter;
    }

    if (zeroCrossing && m_locked) {
        // Ideal zero crossing is at phase 0.0
        float phaseErr = m_dpllPhase;
        if (phaseErr > 0.5f) {
            phaseErr -= 1.0f;
        }

        // Loop gains
        const float Kp = 0.05f;
        const float Ki = 0.001f;

        m_dpllPhase -= Kp * phaseErr;
        m_dpllFreq -= Ki * phaseErr;

        // Clamp DPLL frequency to +/- 5% of nominal baud
        const float nomFreq = m_baudRate / static_cast<float>(DECIM_RATE);
        m_dpllFreq = qBound(nomFreq * 0.95f, m_dpllFreq, nomFreq * 1.05f);
    }

    // Advance DPLL phase
    const float oldPhase = m_dpllPhase;
    m_dpllPhase += m_dpllFreq;

    // Trigger symbol strobe at mid-symbol (phase crossing 0.5)
    if (oldPhase < 0.5f && m_dpllPhase >= 0.5f) {
        onSymbolStrobe();
    }

    // Wrap phase around [0.0, 1.0)
    if (m_dpllPhase >= 1.0f) {
        m_dpllPhase -= 1.0f;
    } else if (m_dpllPhase < 0.0f) {
        m_dpllPhase += 1.0f;
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

    // Calculate continuous Log-Likelihood Ratio (LLR)
    // Positive LLR = Mark (bit 1), Negative LLR = Space (bit 0)
    float diffE = markE - spaceE;
    if (m_reversePolarity) {
        diffE = -diffE;
    }

    const float llr = diffE / qMax(m_noiseVariance, 1e-6f);

    RttySoftSymbol sym;
    sym.llr = llr;
    sym.markMag = markMag;
    sym.spaceMag = spaceMag;
    sym.snrDb = m_currentSnrDb;
    sym.sampleIdx = m_sampleCounter;

    // 1. Emit soft symbol for Stage 2 Bayesian Trellis / HMM decoder
    emit symbolSampled(sym);

    // 2. Standalone Hard-Decision Baudot Slicer for Stage 1 validation
    const bool hardBit = (llr > 0.0f); // 1 = Mark, 0 = Space
    processBaudotBit(hardBit, llr);

    // Periodic diagnostics (every ~10 symbols)
    if ((m_sampleCounter % (m_symSamples * 10)) < m_symSamples) {
        emit toneStatusChanged(m_rxId, markFreqHz(), spaceFreqHz(), m_currentSnrDb, m_locked);
    }
}

void RttyDemodulator::processBaudotBit(bool bit, float llr) {
    Q_UNUSED(llr)

    switch (m_framerState) {
    case WaitStart:
        // Wait for Start bit (Space = 0)
        if (!bit) {
            m_framerState = DataBits;
            m_bitCount = 0;
            m_shiftReg = 0;
        }
        break;

    case DataBits:
        // Collect 5 data bits (LSB first)
        if (bit) {
            m_shiftReg |= (1 << m_bitCount);
        }
        ++m_bitCount;
        if (m_bitCount >= 5) {
            m_framerState = StopBit;
        }
        break;

    case StopBit:
        // Stop bit must be Mark (1)
        if (bit) {
            quint8 code = m_shiftReg & 0x1F;
            if (code == 0x1F) {
                // LTRS shift (11111)
                m_figsMode = false;
            } else if (code == 0x1B) {
                // FIGS shift (11011)
                m_figsMode = true;
            } else {
                const QChar ch = decodeBaudot(code, m_figsMode);
                if (!ch.isNull() && ch.toLatin1() != '\0') {
                    const QString str(ch);
                    m_recentText.append(str);
                    if (m_recentText.size() > 500) {
                        m_recentText = m_recentText.right(400);
                    }
                    emit characterDecoded(m_rxId, str);
                    emit textUpdated(m_rxId, m_recentText);
                }
            }
        }
        // Ready for next character
        m_framerState = WaitStart;
        break;
    }
}

QChar RttyDemodulator::decodeBaudot(quint8 code, bool figs) {
    code &= 0x1F; // 5-bit code
    if (code == 0x1F || code == 0x1B) {
        return QChar('\0');
    }
    const char c = figs ? FIGS_TABLE[code] : LTRS_TABLE[code];
    return (c != '\0') ? QChar(QLatin1Char(c)) : QChar('\0');
}
