#include "RttyAutoClassifier.h"
#include <algorithm>
#include <numeric>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace {

static void fftRadix2(std::vector<std::complex<float>> &data)
{
    const size_t n = data.size();
    size_t j = 0;
    for (size_t i = 0; i < n; ++i) {
        if (i < j) {
            std::swap(data[i], data[j]);
        }
        size_t bit = n >> 1;
        while (bit > 0 && (j & bit)) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
    }

    for (size_t len = 2; len <= n; len <<= 1) {
        const float angle = -2.0f * static_cast<float>(M_PI) / static_cast<float>(len);
        const std::complex<float> wlen(std::cos(angle), std::sin(angle));
        for (size_t i = 0; i < n; i += len) {
            std::complex<float> w(1.0f, 0.0f);
            for (size_t k = 0; k < len / 2; ++k) {
                const std::complex<float> u = data[i + k];
                const std::complex<float> v = data[i + k + len / 2] * w;
                data[i + k] = u + v;
                data[i + k + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
}

constexpr float CANDIDATE_BAUD_RATES[4] = {45.4545f, 50.0f, 75.0f, 100.0f};

} // anonymous namespace

RttyAutoClassifier::RttyAutoClassifier(int rxId, QObject *parent)
    : QObject(parent)
    , m_rxId(rxId)
{
    m_audioRing.resize(FFT_SIZE, 0.0f);
    m_fftWindow.resize(FFT_SIZE);
    for (int i = 0; i < FFT_SIZE; ++i) {
        m_fftWindow[i] = 0.5f * (1.0f - std::cos(2.0f * static_cast<float>(M_PI) * i / static_cast<float>(FFT_SIZE - 1)));
    }
    m_powerSpectrum.resize(FFT_SIZE / 2 + 1, 0.0f);
    m_smoothedSpectrum.resize(FFT_SIZE / 2 + 1, 0.0f);
    reset();
}

void RttyAutoClassifier::setEnabled(bool enable)
{
    m_enabled = enable;
    if (!m_enabled) {
        reset();
    }
}

void RttyAutoClassifier::reset()
{
    std::fill(m_audioRing.begin(), m_audioRing.end(), 0.0f);
    m_audioRingPos = 0;
    std::fill(m_powerSpectrum.begin(), m_powerSpectrum.end(), 0.0f);
    std::fill(m_smoothedSpectrum.begin(), m_smoothedSpectrum.end(), 0.0f);
    m_spectrumFrames = 0;

    m_candidateShift = 0.0f;
    m_candidateCenter = 0.0f;
    m_shiftCandidateCount = 0;
    m_shiftLocked = false;
    m_baudLocked = false;

    std::fill(std::begin(m_baudScores), std::end(m_baudScores), 0.0f);
    m_transitionCount = 0;

    m_stopBitLlrSum = 0.0f;
    m_polarityFrameCount = 0;
    m_polarityCooldown = 0;
}

void RttyAutoClassifier::feedAudio(const float *samples, int count, int sampleRate)
{
    if (!m_enabled || !samples || count <= 0 || sampleRate <= 0) {
        return;
    }

    for (int i = 0; i < count; ++i) {
        m_audioRing[m_audioRingPos] = samples[i];
        m_audioRingPos = (m_audioRingPos + 1) % FFT_SIZE;
        if (m_audioRingPos == 0) {
            processFftBuffer();
        }
    }
}

void RttyAutoClassifier::processFftBuffer()
{
    std::vector<std::complex<float>> fftBuffer(FFT_SIZE);
    for (int i = 0; i < FFT_SIZE; ++i) {
        fftBuffer[i] = std::complex<float>(m_audioRing[i] * m_fftWindow[i], 0.0f);
    }

    fftRadix2(fftBuffer);

    const int halfSize = FFT_SIZE / 2;
    for (int k = 0; k <= halfSize; ++k) {
        const float pwr = std::norm(fftBuffer[k]);
        m_powerSpectrum[k] = pwr;
        if (m_spectrumFrames == 0) {
            m_smoothedSpectrum[k] = pwr;
        } else {
            m_smoothedSpectrum[k] = 0.80f * m_smoothedSpectrum[k] + 0.20f * pwr;
        }
    }
    ++m_spectrumFrames;

    // Search passband 500 Hz to 2800 Hz
    constexpr float binHz = 48000.0f / static_cast<float>(FFT_SIZE); // ~23.4375 Hz
    const int kMin = std::max(2, static_cast<int>(500.0f / binHz));
    const int kMax = std::min(halfSize - 2, static_cast<int>(2800.0f / binHz));

    // Calculate median noise floor in search range
    std::vector<float> floorVals;
    floorVals.reserve(kMax - kMin + 1);
    for (int k = kMin; k <= kMax; ++k) {
        floorVals.push_back(m_smoothedSpectrum[k]);
    }
    std::sort(floorVals.begin(), floorVals.end());
    const float medianNoise = floorVals.empty() ? 1e-6f : floorVals[floorVals.size() / 2];

    // Find local maxima
    struct Peak {
        int bin = 0;
        float power = 0.0f;
    };
    std::vector<Peak> peaks;

    for (int k = kMin + 1; k < kMax; ++k) {
        const float p = m_smoothedSpectrum[k];
        if (p > m_smoothedSpectrum[k - 1] && p > m_smoothedSpectrum[k + 1]) {
            if (p > medianNoise * 3.5f) { // ~5.4 dB above median floor
                peaks.push_back({k, p});
            }
        }
    }

    if (peaks.size() < 2) {
        m_shiftCandidateCount = 0;
        return;
    }

    std::sort(peaks.begin(), peaks.end(), [](const Peak &a, const Peak &b) {
        return a.power > b.power;
    });

    const Peak &p1 = peaks[0];
    int p2Idx = -1;
    for (size_t i = 1; i < peaks.size(); ++i) {
        if (std::abs(peaks[i].bin - p1.bin) >= 4) { // At least ~94 Hz apart
            p2Idx = static_cast<int>(i);
            break;
        }
    }

    if (p2Idx < 0) {
        m_shiftCandidateCount = 0;
        return;
    }

    const Peak &p2 = peaks[p2Idx];

    auto refineFreq = [this, binHz](int k) -> float {
        const float y0 = m_smoothedSpectrum[k - 1];
        const float y1 = m_smoothedSpectrum[k];
        const float y2 = m_smoothedSpectrum[k + 1];
        const float denom = y0 - 2.0f * y1 + y2;
        float delta = 0.0f;
        if (std::abs(denom) > 1e-12f) {
            delta = 0.5f * (y0 - y2) / denom;
            delta = std::clamp(delta, -0.5f, 0.5f);
        }
        return (static_cast<float>(k) + delta) * binHz;
    };

    float f1 = refineFreq(p1.bin);
    float f2 = refineFreq(p2.bin);
    if (f1 > f2) {
        std::swap(f1, f2);
    }

    const float rawShift = f2 - f1;
    const float center = (f1 + f2) * 0.5f;

    // Classify shift against standard standards
    float standardShift = 0.0f;
    if (rawShift >= 140.0f && rawShift <= 185.0f) {
        standardShift = 170.0f;
    } else if (rawShift > 185.0f && rawShift <= 250.0f) {
        standardShift = 200.0f;
    } else if (rawShift >= 380.0f && rawShift <= 437.0f) {
        standardShift = 425.0f;
    } else if (rawShift > 437.0f && rawShift <= 500.0f) {
        standardShift = 450.0f;
    } else if (rawShift >= 780.0f && rawShift <= 920.0f) {
        standardShift = 850.0f;
    }

    if (standardShift > 0.0f) {
        if (standardShift == m_candidateShift && std::abs(center - m_candidateCenter) < 40.0f) {
            m_shiftCandidateCount++;
            m_candidateCenter = 0.8f * m_candidateCenter + 0.2f * center;
            if (m_shiftCandidateCount >= 3) {
                m_lockedShiftHz = standardShift;
                m_lockedCenterFreqHz = m_candidateCenter;
                m_shiftLocked = true;
                emit shiftDetected(m_lockedShiftHz, m_lockedCenterFreqHz);
                // Weather/nav shifts are 50 baud. Don't wait for interval scoring
                // (1.5-stop looks like 100 baud) while the demod still runs 45.45.
                if (!m_baudLocked && (standardShift == 425.0f || standardShift == 450.0f)) {
                    m_lockedBaudRate = 50.0f;
                    m_baudLocked = true;
                    emit baudRateDetected(m_lockedBaudRate);
                }
            }
        } else {
            m_candidateShift = standardShift;
            m_candidateCenter = center;
            m_shiftCandidateCount = 1;
        }
    } else {
        m_shiftCandidateCount = 0;
    }
}

void RttyAutoClassifier::feedTransition(int intervalSamples, float sampleRate)
{
    if (!m_enabled || intervalSamples < 12 || intervalSamples > 250) {
        return;
    }

    // Integer bit runs plus the 1.5-unit stop used by commercial/weather RTTY.
    // A 50-baud 1.5-stop is 60 samples @ 2 kHz — also exactly 3 bits at 100 baud —
    // so integer-only scoring locks 45.45 (default) or 100 instead of 50.
    static const float kMultiples[] = {1.0f, 1.5f, 2.0f, 2.5f, 3.0f, 4.0f, 5.0f, 6.0f};
    for (int i = 0; i < 4; ++i) {
        const float T = sampleRate / CANDIDATE_BAUD_RATES[i];
        float bestErr = 1.0f;
        float bestK = 1.0f;
        for (float k : kMultiples) {
            const float expected = k * T;
            const float err = std::abs(static_cast<float>(intervalSamples) - expected) / T;
            if (err < bestErr) {
                bestErr = err;
                bestK = k;
            }
        }
        if (bestErr < 0.08f) {
            float weight = 1.0f / bestK;
            if (bestK == 1.5f)
                weight = 0.85f;
            else if (bestK >= 3.0f)
                weight *= 0.45f;
            m_baudScores[i] += (1.0f - bestErr / 0.08f) * weight;
        }
    }

    ++m_transitionCount;
    if (m_transitionCount >= 20) {
        analyzeBaudScores();
        m_transitionCount = 0;
    }
}

void RttyAutoClassifier::analyzeBaudScores()
{
    int bestIdx = 0;
    float bestScore = m_baudScores[0];
    float secondScore = 0.0f;

    for (int i = 1; i < 4; ++i) {
        if (m_baudScores[i] > bestScore) {
            secondScore = bestScore;
            bestScore = m_baudScores[i];
            bestIdx = i;
        } else if (m_baudScores[i] > secondScore) {
            secondScore = m_baudScores[i];
        }
    }

    if (bestScore > 5.0f && bestScore > 1.20f * secondScore) {
        m_lockedBaudRate = CANDIDATE_BAUD_RATES[bestIdx];
        m_baudLocked = true;
        emit baudRateDetected(m_lockedBaudRate);

        // Soft score decay to enable re-tracking if signal changes
        for (int i = 0; i < 4; ++i) {
            m_baudScores[i] *= 0.4f;
        }
    }
}

void RttyAutoClassifier::feedFramingResult(float stopBitLlr, float confidence)
{
    Q_UNUSED(confidence);
    if (!m_enabled) {
        return;
    }

    if (m_polarityCooldown > 0) {
        --m_polarityCooldown;
        return;
    }

    m_stopBitLlrSum += stopBitLlr;
    ++m_polarityFrameCount;

    // Require at least 15 consecutive frames of strongly inverted stop bits (< -2.5)
    // to prevent spurious flips from baud drift or occasional framing slips
    if (m_polarityFrameCount >= 15) {
        const float avgLlr = m_stopBitLlrSum / static_cast<float>(m_polarityFrameCount);
        m_stopBitLlrSum = 0.0f;
        m_polarityFrameCount = 0;

        // If stop bits are consistently negative (Space instead of Mark), polarity is inverted
        if (avgLlr < -2.5f) {
            m_polarityCooldown = 40; // 40-frame lockout to prevent rapid oscillation
            emit polarityInversionSuggested();
        }
    }
}
