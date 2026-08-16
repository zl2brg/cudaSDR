#include "CwDecoder.h"

#include <QMap>
#include <algorithm>

CwDecoder::CwDecoder(int rxId, QObject *parent)
    : QObject(parent)
    , m_rxId(rxId)
    , m_trackedPitchHz(700.0f)
    , m_lastFilterPitchHz(700.0f)
{
    updateBiquadCoefficients();
}

void CwDecoder::setEnabled(bool enable)
{
    if (m_enabled == enable)
        return;
    m_enabled = enable;
    if (!m_enabled) {
        reset();
    }
}

void CwDecoder::setAutoTrackEnabled(bool enabled)
{
    m_autoTrack = enabled;
    if (!m_autoTrack) {
        m_trackedPitchHz = static_cast<float>(m_pitchHz);
        updateBiquadCoefficients();
    }
}

void CwDecoder::setPitch(int pitchHz)
{
    const int clamped = qBound(300, pitchHz, 2000);
    if (m_pitchHz == clamped)
        return;
    m_pitchHz = clamped;
    if (!m_autoTrack || std::abs(m_trackedPitchHz - static_cast<float>(m_pitchHz)) > 220.0f) {
        m_trackedPitchHz = static_cast<float>(m_pitchHz);
    }
    updateBiquadCoefficients();
}

void CwDecoder::clearText()
{
    m_recentText.clear();
    m_symbolAccumulator.clear();
    m_charPending = false;
    m_wordSpacePending = false;
    emit textUpdated(m_rxId, m_recentText);
}

void CwDecoder::reset()
{
    m_x1 = m_x2 = m_y1 = m_y2 = 0.0f;
    m_wbX1 = m_wbX2 = m_wbY1 = m_wbY2 = 0.0f;
    m_envelope = 0.0f;
    m_noiseLevel = 0.005f;
    m_peakLevel = 0.08f;
    m_toneActive = false;
    m_currentSnrDb = 0.0f;
    m_ditMs = 60.0f;
    m_currentWpm = 20;
    m_markDurationMs = 0.0f;
    m_spaceDurationMs = 0.0f;
    m_symbolAccumulator.clear();
    m_charPending = false;
    m_wordSpacePending = false;
    m_decimCounter = 0;
    m_pitchEstimCounter = 0;
    m_ringIdx = 0;
    std::fill(std::begin(m_ringBuffer), std::end(m_ringBuffer), 0.0f);
}

void CwDecoder::updateBiquadCoefficients()
{
    const float fs = 8000.0f;

    // 1. Narrow Detection Filter at m_trackedPitchHz (Q = 7.0, ~100 Hz BW)
    const float fNarrow = qBound(300.0f, m_trackedPitchHz, 1500.0f);
    m_lastFilterPitchHz = fNarrow;
    const float qNarrow = 7.0f;

    const float w0 = 2.0f * M_PI * fNarrow / fs;
    const float alpha = std::sin(w0) / (2.0f * qNarrow);
    const float cosW0 = std::cos(w0);

    const float a0 = 1.0f + alpha;
    m_b0 = alpha / a0;
    m_b1 = 0.0f;
    m_b2 = -alpha / a0;
    m_a1 = (-2.0f * cosW0) / a0;
    m_a2 = (1.0f - alpha) / a0;

    // 2. Wideband Pitch Tracker Filter at nominal m_pitchHz (Q = 1.6, ~440 Hz BW)
    const float fWide = static_cast<float>(m_pitchHz);
    const float qWide = 1.6f;
    const float w0Wb = 2.0f * M_PI * fWide / fs;
    const float alphaWb = std::sin(w0Wb) / (2.0f * qWide);
    const float cosW0Wb = std::cos(w0Wb);

    const float a0Wb = 1.0f + alphaWb;
    m_wbB0 = alphaWb / a0Wb;
    m_wbB1 = 0.0f;
    m_wbB2 = -alphaWb / a0Wb;
    m_wbA1 = (-2.0f * cosW0Wb) / a0Wb;
    m_wbA2 = (1.0f - alphaWb) / a0Wb;

    // Envelope low-pass filter coefficient for tau = 12ms at 8000Hz (dt = 0.125ms)
    m_envAlpha = 1.0f - std::exp(-0.125f / 12.0f);
}

void CwDecoder::estimateTonePitch()
{
    if (!m_autoTrack)
        return;

    // Compute autocorrelation on wideband ring buffer (length = 32, lags 7..22)
    // At 8000 Hz: lag 7 = 1142 Hz, lag 22 = 363 Hz
    float maxCorr = 0.0f;
    int bestLag = -1;
    float energy0 = 1e-6f;

    for (int i = 0; i < 32; ++i) {
        const float s0 = m_ringBuffer[(m_ringIdx - i) & 63];
        energy0 += s0 * s0;
    }

    if (energy0 < 0.00003f) {
        // Signal too weak in wideband passband: slowly drift back towards nominal
        if (std::abs(m_trackedPitchHz - static_cast<float>(m_pitchHz)) > 1.0f) {
            m_trackedPitchHz += 0.008f * (static_cast<float>(m_pitchHz) - m_trackedPitchHz);
            if (std::abs(m_trackedPitchHz - m_lastFilterPitchHz) >= 2.0f) {
                updateBiquadCoefficients();
                emit trackedPitchChanged(m_rxId, qRound(m_trackedPitchHz));
            }
        }
        return;
    }

    float corr[24] = {0.0f};
    for (int lag = 7; lag <= 22; ++lag) {
        float r = 0.0f;
        for (int i = 0; i < 32; ++i) {
            const float s0 = m_ringBuffer[(m_ringIdx - i) & 63];
            const float sLag = m_ringBuffer[(m_ringIdx - i - lag) & 63];
            r += s0 * sLag;
        }
        corr[lag] = r;
        if (r > maxCorr) {
            maxCorr = r;
            bestLag = lag;
        }
    }

    // Correlation threshold check (normalized autocorrelation > 0.40)
    if (bestLag >= 8 && bestLag <= 21 && (maxCorr / energy0) > 0.40f) {
        // 3-point parabolic interpolation for sub-sample lag
        const float alpha = corr[bestLag - 1];
        const float beta = corr[bestLag];
        const float gamma = corr[bestLag + 1];
        const float denom = 2.0f * (alpha - 2.0f * beta + gamma);
        float delta = 0.0f;
        if (std::abs(denom) > 1e-6f) {
            delta = (alpha - gamma) / denom;
            delta = qBound(-0.5f, delta, 0.5f);
        }

        const float refinedLag = static_cast<float>(bestLag) + delta;
        const float detectedFreq = 8000.0f / refinedLag;

        // Verify frequency is within ±220 Hz of nominal pitch
        const float minFreq = static_cast<float>(m_pitchHz) - 220.0f;
        const float maxFreq = static_cast<float>(m_pitchHz) + 220.0f;

        if (detectedFreq >= minFreq && detectedFreq <= maxFreq) {
            // Adaptive pitch smoothing
            m_trackedPitchHz += 0.15f * (detectedFreq - m_trackedPitchHz);

            if (std::abs(m_trackedPitchHz - m_lastFilterPitchHz) >= 2.0f) {
                updateBiquadCoefficients();
                emit trackedPitchChanged(m_rxId, qRound(m_trackedPitchHz));
            }
        }
    }
}

void CwDecoder::processAudio(const float *samples, int count, int sampleRate)
{
    if (!m_enabled || !samples || count <= 0)
        return;

    m_sampleRate = sampleRate > 0 ? sampleRate : 48000;
    m_decimFactor = std::max(1, m_sampleRate / 8000);
    const float dtMs = (1000.0f * static_cast<float>(m_decimFactor)) / static_cast<float>(m_sampleRate);

    for (int i = 0; i < count; ++i) {
        if (++m_decimCounter >= m_decimFactor) {
            m_decimCounter = 0;

            const float inSample = samples[i];

            // 1. Wideband filter & ring buffer for pitch estimation
            const float wbOut = m_wbB0 * inSample + m_wbB1 * m_wbX1 + m_wbB2 * m_wbX2 - m_wbA1 * m_wbY1 - m_wbA2 * m_wbY2;
            m_wbX2 = m_wbX1;
            m_wbX1 = inSample;
            m_wbY2 = m_wbY1;
            m_wbY1 = wbOut;

            m_ringIdx = (m_ringIdx + 1) & 63;
            m_ringBuffer[m_ringIdx] = wbOut;

            // Pitch estimation every 32 decimated samples (~4 ms)
            if (++m_pitchEstimCounter >= 32) {
                m_pitchEstimCounter = 0;
                estimateTonePitch();
            }

            // 2. Apply Narrow Biquad Bandpass Filter (centered on tracked pitch)
            const float outSample = m_b0 * inSample + m_b1 * m_x1 + m_b2 * m_x2 - m_a1 * m_y1 - m_a2 * m_y2;
            m_x2 = m_x1;
            m_x1 = inSample;
            m_y2 = m_y1;
            m_y1 = outSample;

            // 3. Full-wave rectify & low-pass smooth envelope
            const float rect = std::abs(outSample);
            m_envelope += m_envAlpha * (rect - m_envelope);

            // 4. Process envelope slicer & state machine
            processEnvelope(m_envelope, dtMs);
        }
    }
}

void CwDecoder::processEnvelope(float env, float dtMs)
{
    // Update noise floor and peak levels
    if (env > m_peakLevel) {
        m_peakLevel += 0.05f * (env - m_peakLevel);
    } else {
        m_peakLevel += 0.0002f * (env - m_peakLevel); // Slow decay
    }

    if (env < m_noiseLevel) {
        m_noiseLevel += 0.05f * (env - m_noiseLevel);
    } else {
        m_noiseLevel += 0.0005f * (env - m_noiseLevel); // Slow rise
    }

    // Minimum sensible floor bounds
    m_noiseLevel = std::max(0.0001f, m_noiseLevel);
    m_peakLevel = std::max(m_noiseLevel * 1.5f + 0.002f, m_peakLevel);

    const float dynamicRange = m_peakLevel - m_noiseLevel;
    const float snrLinear = m_peakLevel / m_noiseLevel;
    m_currentSnrDb = 20.0f * std::log10(std::max(1.0f, snrLinear));

    // Schmitt-trigger hysteresis thresholds with higher noise immunity
    const float highThreshold = m_noiseLevel + 0.45f * dynamicRange;
    const float lowThreshold = m_noiseLevel + 0.20f * dynamicRange;

    // Minimum SNR required to trigger (rejects noise spikes)
    const bool validSignal = (m_currentSnrDb >= m_minSnrDb);

    const bool prevActive = m_toneActive;
    if (!m_toneActive && env >= highThreshold && validSignal) {
        m_toneActive = true;
    } else if (m_toneActive && env <= lowThreshold) {
        m_toneActive = false;
    }

    if (m_toneActive != prevActive) {
        emit toneStatusChanged(m_rxId, m_toneActive, m_currentSnrDb);
    }

    // Timing state machine
    if (m_toneActive) {
        if (!prevActive) {
            // Space finished -> Mark started
            onSpaceCompleted(m_spaceDurationMs);
            m_spaceDurationMs = 0.0f;
            m_markDurationMs = 0.0f;
        }
        m_markDurationMs += dtMs;
    } else {
        if (prevActive) {
            // Mark finished -> Space started
            onMarkCompleted(m_markDurationMs);
            m_markDurationMs = 0.0f;
            m_spaceDurationMs = 0.0f;
        }
        m_spaceDurationMs += dtMs;

        // Check for character completion (inter-character space >= 1.8 * dit)
        if (m_charPending && m_spaceDurationMs >= (1.8f * m_ditMs)) {
            decodeCurrentSymbol();
            m_charPending = false;
        }

        // Check for word completion (inter-word space >= 4.5 * dit)
        if (m_wordSpacePending && m_spaceDurationMs >= (4.5f * m_ditMs)) {
            if (!m_recentText.isEmpty() && !m_recentText.endsWith(QLatin1Char(' '))) {
                m_recentText.append(QLatin1Char(' '));
                if (m_recentText.length() > 256)
                    m_recentText = m_recentText.right(256);
                emit characterDecoded(m_rxId, QStringLiteral(" "), m_lastReportedWpm);
                emit textUpdated(m_rxId, m_recentText);
            }
            m_wordSpacePending = false;
        }
    }
}

void CwDecoder::onMarkCompleted(float durationMs)
{
    // Glitch / Key click rejection (< 15 ms is likely a click or noise spike)
    if (durationMs < 15.0f || m_currentSnrDb < m_minSnrDb)
        return;

    // Dit vs Dah Decision Threshold: 1.8 * dit length
    const float ditThreshold = 1.8f * m_ditMs;

    if (durationMs < ditThreshold) {
        // DIT ('.')
        m_symbolAccumulator.append(QLatin1Char('.'));
        // Smooth dit length adaptation (15% weight)
        m_ditMs = 0.85f * m_ditMs + 0.15f * durationMs;
    } else {
        // DAH ('-')
        m_symbolAccumulator.append(QLatin1Char('-'));
        // Dah is 3 * dit length, adapt estimated dit
        const float estimatedDit = durationMs / 3.0f;
        m_ditMs = 0.85f * m_ditMs + 0.15f * estimatedDit;
    }

    // Clamp dit length to realistic speeds: 8 WPM (150 ms) to 50 WPM (24 ms)
    m_ditMs = qBound(24.0f, m_ditMs, 150.0f);
    m_currentWpm = qRound(1200.0f / m_ditMs);

    m_charPending = true;
    m_wordSpacePending = true;
}

void CwDecoder::onSpaceCompleted(float durationMs)
{
    Q_UNUSED(durationMs);
}

void CwDecoder::decodeCurrentSymbol()
{
    if (m_symbolAccumulator.isEmpty())
        return;

    const QString decodedChar = morseToChar(m_symbolAccumulator);
    m_symbolAccumulator.clear();

    if (!decodedChar.isEmpty()) {
        m_recentText.append(decodedChar);
        if (m_recentText.length() > 256)
            m_recentText = m_recentText.right(256);

        // Only update reported WPM when an actual valid Morse character is decoded
        if (std::abs(m_currentWpm - m_lastReportedWpm) >= 1) {
            m_lastReportedWpm = m_currentWpm;
            emit wpmChanged(m_rxId, m_lastReportedWpm);
        }

        emit characterDecoded(m_rxId, decodedChar, m_lastReportedWpm);
        emit textUpdated(m_rxId, m_recentText);
    }
}

QString CwDecoder::morseToChar(const QString &morse)
{
    static const QMap<QString, QString> morseTable = {
        // Letters
        {QStringLiteral(".-"), QStringLiteral("A")},
        {QStringLiteral("-..."), QStringLiteral("B")},
        {QStringLiteral("-.-."), QStringLiteral("C")},
        {QStringLiteral("-.."), QStringLiteral("D")},
        {QStringLiteral("."), QStringLiteral("E")},
        {QStringLiteral("..-."), QStringLiteral("F")},
        {QStringLiteral("--."), QStringLiteral("G")},
        {QStringLiteral("...."), QStringLiteral("H")},
        {QStringLiteral(".."), QStringLiteral("I")},
        {QStringLiteral(".---"), QStringLiteral("J")},
        {QStringLiteral("-.-"), QStringLiteral("K")},
        {QStringLiteral(".-.."), QStringLiteral("L")},
        {QStringLiteral("--"), QStringLiteral("M")},
        {QStringLiteral("-."), QStringLiteral("N")},
        {QStringLiteral("---"), QStringLiteral("O")},
        {QStringLiteral(".--."), QStringLiteral("P")},
        {QStringLiteral("--.-"), QStringLiteral("Q")},
        {QStringLiteral(".-."), QStringLiteral("R")},
        {QStringLiteral("..."), QStringLiteral("S")},
        {QStringLiteral("-"), QStringLiteral("T")},
        {QStringLiteral("..-"), QStringLiteral("U")},
        {QStringLiteral("...-"), QStringLiteral("V")},
        {QStringLiteral(".--"), QStringLiteral("W")},
        {QStringLiteral("-..-"), QStringLiteral("X")},
        {QStringLiteral("-.--"), QStringLiteral("Y")},
        {QStringLiteral("--.."), QStringLiteral("Z")},

        // Numerals
        {QStringLiteral("-----"), QStringLiteral("0")},
        {QStringLiteral(".----"), QStringLiteral("1")},
        {QStringLiteral("..---"), QStringLiteral("2")},
        {QStringLiteral("...--"), QStringLiteral("3")},
        {QStringLiteral("....-"), QStringLiteral("4")},
        {QStringLiteral("....."), QStringLiteral("5")},
        {QStringLiteral("-...."), QStringLiteral("6")},
        {QStringLiteral("--..."), QStringLiteral("7")},
        {QStringLiteral("---.."), QStringLiteral("8")},
        {QStringLiteral("----."), QStringLiteral("9")},

        // Punctuation
        {QStringLiteral(".-.-.-"), QStringLiteral(".")},
        {QStringLiteral("--..--"), QStringLiteral(",")},
        {QStringLiteral("..--.."), QStringLiteral("?")},
        {QStringLiteral(".----."), QStringLiteral("'")},
        {QStringLiteral("-.-.--"), QStringLiteral("!")},
        {QStringLiteral("-..-."), QStringLiteral("/")},
        {QStringLiteral("-.--."), QStringLiteral("(")},
        {QStringLiteral("-.--.-"), QStringLiteral(")")},
        {QStringLiteral(".-..."), QStringLiteral("<AS>")},
        {QStringLiteral("---..."), QStringLiteral(":")},
        {QStringLiteral("-.-.-."), QStringLiteral(";")},
        {QStringLiteral("-...-"), QStringLiteral("=")},
        {QStringLiteral(".-.-."), QStringLiteral("<AR>")},
        {QStringLiteral("-....-"), QStringLiteral("-")},
        {QStringLiteral("..--.-"), QStringLiteral("_")},
        {QStringLiteral(".-..-."), QStringLiteral("\"")},
        {QStringLiteral(".--.-."), QStringLiteral("@")},

        // Common Prosigns
        {QStringLiteral("...-.-"), QStringLiteral("<SK>")},
        {QStringLiteral("-.--."), QStringLiteral("<KN>")},
        {QStringLiteral("...---..."), QStringLiteral("<SOS>")},
        {QStringLiteral("-.-.-"), QStringLiteral("<CT>")},
        {QStringLiteral("........"), QStringLiteral("<HH>")}
    };

    return morseTable.value(morse, QStringLiteral("*")); // '*' represents unknown symbol
}
