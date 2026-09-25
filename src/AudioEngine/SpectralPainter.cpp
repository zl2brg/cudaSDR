#include "SpectralPainter.h"

#include <QImage>
#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include <QDebug>
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

SpectralPainter::SpectralPainter(QObject *parent)
    : QObject(parent)
{
}

void SpectralPainter::setSampleRate(int rate)
{
    QMutexLocker locker(&m_mutex);
    if (rate > 8000 && rate <= 192000) {
        m_sampleRate = rate;
    }
}

void SpectralPainter::setFrequencyRange(float lowHz, float highHz)
{
    QMutexLocker locker(&m_mutex);
    if (lowHz > 0.0f && highHz > lowHz) {
        m_lowHz = lowHz;
        m_highHz = highHz;
    }
}

void SpectralPainter::setDurationMs(int ms)
{
    QMutexLocker locker(&m_mutex);
    m_durationMs = qBound(500, ms, 10000);
}

QString SpectralPainter::customText() const
{
    QMutexLocker locker(&m_mutex);
    return m_customText;
}

void SpectralPainter::setCustomText(const QString &text)
{
    QMutexLocker locker(&m_mutex);
    m_customText = text.trimmed();
}

SpectralPainter::PaintStyle SpectralPainter::paintStyle() const
{
    QMutexLocker locker(&m_mutex);
    return m_paintStyle;
}

void SpectralPainter::setPaintStyle(PaintStyle style)
{
    QMutexLocker locker(&m_mutex);
    m_paintStyle = style;
}

bool SpectralPainter::invertFrequency() const
{
    QMutexLocker locker(&m_mutex);
    return m_invertFrequency;
}

void SpectralPainter::setInvertFrequency(bool invert)
{
    QMutexLocker locker(&m_mutex);
    m_invertFrequency = invert;
}

bool SpectralPainter::invertTime() const
{
    QMutexLocker locker(&m_mutex);
    return m_invertTime;
}

void SpectralPainter::setInvertTime(bool invert)
{
    QMutexLocker locker(&m_mutex);
    m_invertTime = invert;
}

std::vector<std::complex<float>> SpectralPainter::synthesizeIq(const QString &text,
                                                              const QString &callsignFallback,
                                                              bool invertFrequency) const
{
    QString targetText = text.trimmed();
    if (targetText.isEmpty()) {
        QMutexLocker locker(&m_mutex);
        targetText = m_customText;
    }
    if (targetText.isEmpty()) {
        targetText = callsignFallback.trimmed();
    }
    if (targetText.isEmpty()) {
        targetText = QStringLiteral("NOCALL");
    }

    int sampleRate;
    float lowHz;
    float highHz;
    int durationMs;
    bool invertFreq;
    bool invertTm;
    PaintStyle style;
    {
        QMutexLocker locker(&m_mutex);
        sampleRate = m_sampleRate;
        lowHz = m_lowHz;
        highHz = m_highHz;
        durationMs = m_durationMs;
        invertFreq = invertFrequency || m_invertFrequency;
        invertTm = m_invertTime;
        style = m_paintStyle;
    }

    if (style == PaintStyle::Banner) {
        return synthesizeBannerIq(targetText, sampleRate, lowHz, highHz, durationMs, invertFreq, invertTm);
    }
    return synthesizeTickerIq(targetText, sampleRate, lowHz, highHz, durationMs, invertFreq, invertTm);
}

std::vector<float> SpectralPainter::synthesize(const QString &text,
                                              const QString &callsignFallback,
                                              bool invertFrequency) const
{
    const std::vector<std::complex<float>> iq = synthesizeIq(text, callsignFallback, invertFrequency);
    if (iq.empty())
        return {};

    std::vector<float> output(iq.size());
    float maxAbs = 0.0f;
    for (size_t i = 0; i < iq.size(); ++i) {
        output[i] = iq[i].real();
        const float a = std::abs(output[i]);
        if (a > maxAbs)
            maxAbs = a;
    }
    if (maxAbs > 0.001f) {
        const float gain = 0.70f / maxAbs;
        for (float &s : output) {
            s *= gain;
        }
    }
    return output;
}

std::vector<std::complex<float>> SpectralPainter::synthesizeTickerIq(const QString &text, int sampleRate,
                                                                     float lowHz, float highHz, int durationMs,
                                                                     bool invertFreq, bool invertTm) const
{
    const int numChars = text.length();
    if (numChars == 0)
        return {};

    constexpr int numBands = 48;
    constexpr int charHeight = 32;

    // 1. Rasterize each character individually centered in [numBands x charHeight]
    QFont font;
    font.setFamily(QStringLiteral("Arial"));
    font.setStyleHint(QFont::SansSerif);
    font.setBold(true);

    const int padX = 3;
    const int padY = 2;
    const int availW = numBands - 2 * padX;
    const int availH = charHeight - 2 * padY;

    std::vector<QImage> charImages;
    charImages.reserve(numChars);

    for (int i = 0; i < numChars; ++i) {
        const QChar qc = text.at(i);
        QImage cImg(numBands, charHeight, QImage::Format_Grayscale8);
        cImg.fill(0);

        if (qc != QLatin1Char(' ')) {
            const QString chStr(qc);
            for (int px = 36; px >= 10; --px) {
                font.setPixelSize(px);
                QFontMetrics fm(font);
                if (fm.horizontalAdvance(chStr) <= availW && fm.height() <= availH) {
                    break;
                }
            }
            QFontMetrics fm(font);
            const int drawX = (numBands - fm.horizontalAdvance(chStr)) / 2;
            const int drawY = (charHeight - fm.height()) / 2 + fm.ascent();

            QPainter p(&cImg);
            p.setFont(font);
            p.setPen(Qt::white);
            p.drawText(drawX, drawY, chStr);
            p.end();
        }
        charImages.push_back(std::move(cImg));
    }

    // 2. Synthesize direct complex baseband I/Q stream across durationMs
    const double durationSec = static_cast<double>(durationMs) / 1000.0;
    const size_t totalSamples = static_cast<size_t>(std::round(durationSec * sampleRate));
    if (totalSamples == 0)
        return {};

    std::vector<std::complex<float>> output(totalSamples, std::complex<float>(0.0f, 0.0f));

    // Tone parameters per column x in [0, numBands - 1]
    std::vector<double> toneFreq(numBands);
    std::vector<double> deltaPhi(numBands);
    std::vector<double> phi(numBands);

    for (int x = 0; x < numBands; ++x) {
        const double frac = static_cast<double>(x) / static_cast<double>(numBands - 1);
        // Positive baseband for USB (+lowHz .. +highHz), negative baseband for LSB (-highHz .. -lowHz)
        const double f = invertFreq
            ? (-highHz + frac * (highHz - lowHz))
            : (lowHz + frac * (highHz - lowHz));

        toneFreq[x] = f;
        deltaPhi[x] = (2.0 * M_PI * f) / static_cast<double>(sampleRate);
        const double theta = (M_PI * x * x) / static_cast<double>(numBands);
        phi[x] = std::fmod(theta, 2.0 * M_PI);
    }

    const double samplesPerChar = static_cast<double>(totalSamples) / static_cast<double>(numChars);
    // 85% character glyph, 15% inter-character gap
    const double activeSamples = samplesPerChar * 0.85;

    for (size_t n = 0; n < totalSamples; ++n) {
        const int cIdx = qBound(0, static_cast<int>(std::floor(static_cast<double>(n) / samplesPerChar)), numChars - 1);
        const double charSample = static_cast<double>(n) - static_cast<double>(cIdx) * samplesPerChar;

        double sampleRe = 0.0;
        double sampleIm = 0.0;
        if (charSample < activeSamples) {
            const double progress = charSample / activeSamples;
            // Transmit bottom row first so it scrolls down the waterfall leaving letter upright
            const double v = invertTm
                ? (static_cast<double>(charHeight - 1) * progress)
                : (static_cast<double>(charHeight - 1) * (1.0 - progress));

            const int y0 = qBound(0, static_cast<int>(std::floor(v)), charHeight - 1);
            const int y1 = qBound(0, y0 + 1, charHeight - 1);
            const double tFrac = v - std::floor(v);

            const QImage &cImg = charImages[cIdx];
            const uchar *line0 = cImg.constScanLine(y0);
            const uchar *line1 = cImg.constScanLine(y1);

            for (int x = 0; x < numBands; ++x) {
                const double v0 = static_cast<double>(line0[x]) / 255.0;
                const double v1 = static_cast<double>(line1[x]) / 255.0;
                const double amp = (1.0 - tFrac) * v0 + tFrac * v1;

                if (amp > 0.04) {
                    sampleRe += amp * std::cos(phi[x]);
                    sampleIm += amp * std::sin(phi[x]);
                }
            }
        }

        for (int x = 0; x < numBands; ++x) {
            phi[x] += deltaPhi[x];
            if (phi[x] >= 2.0 * M_PI) {
                phi[x] -= 2.0 * M_PI;
            } else if (phi[x] < 0.0) {
                phi[x] += 2.0 * M_PI;
            }
        }

        output[n] = std::complex<float>(static_cast<float>(sampleRe), static_cast<float>(sampleIm));
    }

    // Peak normalize complex envelope to ~0.70 (-3 dBFS)
    float maxMag = 0.0f;
    for (const auto &s : output) {
        const float mag = std::abs(s);
        if (mag > maxMag)
            maxMag = mag;
    }
    if (maxMag > 0.001f) {
        const float gain = 0.70f / maxMag;
        for (auto &s : output) {
            s *= gain;
        }
    }

    // 10 ms raised-cosine edge taper
    const size_t taperLen = static_cast<size_t>(0.010 * sampleRate);
    if (taperLen > 0 && totalSamples > 2 * taperLen) {
        for (size_t n = 0; n < taperLen; ++n) {
            const float w = 0.5f * (1.0f - std::cos(static_cast<float>(M_PI * n / taperLen)));
            output[n] *= w;
            output[totalSamples - 1 - n] *= w;
        }
    }

    return output;
}

std::vector<std::complex<float>> SpectralPainter::synthesizeBannerIq(const QString &text, int sampleRate,
                                                                     float lowHz, float highHz, int durationMs,
                                                                     bool invertFreq, bool invertTm) const
{
    constexpr int numBands = 80;
    const int imgHeight = qBound(24, durationMs / 50, 64);

    // 1. Rasterize text onto a grayscale image without any border framing.
    // The image width (numBands columns) maps to frequency (lowHz to highHz).
    // The image height (imgHeight rows) maps to time over durationMs.
    QImage img(numBands, imgHeight, QImage::Format_Grayscale8);
    img.fill(0); // Pure black background

    QFont font;
    font.setFamily(QStringLiteral("Arial"));
    font.setStyleHint(QFont::SansSerif);
    font.setBold(true);

    const int padX = 4;
    const int padY = 2;
    const int availW = numBands - 2 * padX;
    const int availH = imgHeight - 2 * padY;

    for (int px = 32; px >= 8; --px) {
        font.setPixelSize(px);
        QFontMetrics fm(font);
        if (fm.horizontalAdvance(text) <= availW && fm.height() <= availH) {
            break;
        }
    }

    QFontMetrics fm(font);
    const int textW = fm.horizontalAdvance(text);
    const int textH = fm.height();
    const int drawX = (numBands - textW) / 2;
    const int drawY = (imgHeight - textH) / 2 + fm.ascent();

    {
        QPainter p(&img);
        p.setFont(font);
        p.setPen(Qt::white);
        p.drawText(drawX, drawY, text);
        p.end();
    }

    // 2. Additive multi-tone complex synthesis with Schroeder phase distribution
    const double durationSec = static_cast<double>(durationMs) / 1000.0;
    const size_t totalSamples = static_cast<size_t>(std::round(durationSec * sampleRate));
    if (totalSamples == 0)
        return {};

    std::vector<std::complex<float>> output(totalSamples, std::complex<float>(0.0f, 0.0f));

    // Tone parameters per column x in [0, numBands - 1]
    std::vector<double> toneFreq(numBands);
    std::vector<double> deltaPhi(numBands);
    std::vector<double> phi(numBands);

    for (int x = 0; x < numBands; ++x) {
        const double frac = static_cast<double>(x) / static_cast<double>(numBands - 1);
        // Positive baseband for USB (+lowHz .. +highHz), negative baseband for LSB (-highHz .. -lowHz)
        const double f = invertFreq
            ? (-highHz + frac * (highHz - lowHz))
            : (lowHz + frac * (highHz - lowHz));

        toneFreq[x] = f;
        deltaPhi[x] = (2.0 * M_PI * f) / static_cast<double>(sampleRate);
        const double theta = (M_PI * x * x) / static_cast<double>(numBands);
        phi[x] = std::fmod(theta, 2.0 * M_PI);
    }

    for (size_t n = 0; n < totalSamples; ++n) {
        const double progress = (totalSamples > 1)
            ? (static_cast<double>(n) / static_cast<double>(totalSamples - 1))
            : 0.0;

        const double v = invertTm
            ? (static_cast<double>(imgHeight - 1) * progress)
            : (static_cast<double>(imgHeight - 1) * (1.0 - progress));

        const int y0 = qBound(0, static_cast<int>(std::floor(v)), imgHeight - 1);
        const int y1 = qBound(0, y0 + 1, imgHeight - 1);
        const double tFrac = v - std::floor(v);

        const uchar *line0 = img.constScanLine(y0);
        const uchar *line1 = img.constScanLine(y1);

        double sampleRe = 0.0;
        double sampleIm = 0.0;
        for (int x = 0; x < numBands; ++x) {
            const double v0 = static_cast<double>(line0[x]) / 255.0;
            const double v1 = static_cast<double>(line1[x]) / 255.0;
            double amp = (1.0 - tFrac) * v0 + tFrac * v1;

            if (amp > 0.08) {
                // Solidify character strokes for sharp high-contrast waterfall appearance
                amp = qMin(1.0, amp * 1.25);
                sampleRe += amp * std::cos(phi[x]);
                sampleIm += amp * std::sin(phi[x]);
            }

            phi[x] += deltaPhi[x];
            if (phi[x] >= 2.0 * M_PI) {
                phi[x] -= 2.0 * M_PI;
            } else if (phi[x] < 0.0) {
                phi[x] += 2.0 * M_PI;
            }
        }
        output[n] = std::complex<float>(static_cast<float>(sampleRe), static_cast<float>(sampleIm));
    }

    // Peak normalize complex envelope to ~0.70 (-3 dBFS)
    float maxMag = 0.0f;
    for (const auto &s : output) {
        const float mag = std::abs(s);
        if (mag > maxMag)
            maxMag = mag;
    }

    if (maxMag > 0.001f) {
        const float gain = 0.70f / maxMag;
        for (auto &s : output) {
            s *= gain;
        }
    }

    const size_t taperLen = static_cast<size_t>(0.010 * sampleRate);
    if (taperLen > 0 && totalSamples > 2 * taperLen) {
        for (size_t n = 0; n < taperLen; ++n) {
            const float w = 0.5f * (1.0f - std::cos(static_cast<float>(M_PI * n / taperLen)));
            output[n] *= w;
            output[totalSamples - 1 - n] *= w;
        }
    }

    return output;
}

void SpectralPainter::prepareBuffer(const QString &callsignFallback, bool invertFrequency)
{
    std::vector<std::complex<float>> samples = synthesizeIq(QString(), callsignFallback, invertFrequency);
    QMutexLocker locker(&m_mutex);
    m_iqSamples = std::move(samples);
    m_readIndex = 0;
}

bool SpectralPainter::start(bool isAutoTail,
                            const QString &textOverride,
                            const QString &callsignFallback,
                            bool invertFrequency)
{
    std::vector<std::complex<float>> samples;
    const bool needSynthesize = !textOverride.isEmpty() ||
                                m_iqSamples.empty() ||
                                (invertFrequency != m_invertFrequency);
    if (needSynthesize) {
        samples = synthesizeIq(textOverride, callsignFallback, invertFrequency);
    }

    {
        QMutexLocker locker(&m_mutex);
        if (!samples.empty()) {
            m_iqSamples = std::move(samples);
        }
        if (m_iqSamples.empty()) {
            return false;
        }
        m_readIndex = 0;
        m_isAutoTail.store(isAutoTail, std::memory_order_release);
        m_active.store(true, std::memory_order_release);
    }

    emit paintStarted(isAutoTail);
    return true;
}

void SpectralPainter::stop()
{
    bool wasActive = false;
    bool wasAutoTail = false;
    {
        QMutexLocker locker(&m_mutex);
        wasActive = m_active.exchange(false, std::memory_order_acq_rel);
        wasAutoTail = m_isAutoTail.load(std::memory_order_acquire);
        m_readIndex = 0;
    }
    if (wasActive) {
        emit paintFinished(wasAutoTail);
    }
}

size_t SpectralPainter::readIqSamples(cpx *dest, size_t count)
{
    if (!m_active.load(std::memory_order_acquire) || !dest || count == 0)
        return 0;

    bool finished = false;
    bool wasAutoTail = false;
    size_t samplesRead = 0;

    {
        QMutexLocker locker(&m_mutex);
        if (!m_active.load(std::memory_order_relaxed))
            return 0;

        const size_t available = (m_readIndex < m_iqSamples.size()) ? (m_iqSamples.size() - m_readIndex) : 0;
        samplesRead = std::min(count, available);

        // WDSP and HPSDR convention for TX IQ buffers:
        // pbuff[0] (.re) is Q, pbuff[1] (.im) is I.
        // Mapping Q to .re and I to .im aligns direct I/Q with Spectrum0 analyzer
        // and hardware TX DAC paths, preventing horizontal mirroring and sideband inversion.
        for (size_t i = 0; i < samplesRead; ++i) {
            dest[i].re = static_cast<double>(m_iqSamples[m_readIndex + i].imag());
            dest[i].im = static_cast<double>(m_iqSamples[m_readIndex + i].real());
        }
        m_readIndex += samplesRead;

        if (m_readIndex >= m_iqSamples.size()) {
            m_active.store(false, std::memory_order_release);
            finished = true;
            wasAutoTail = m_isAutoTail.load(std::memory_order_relaxed);
        }
    }

    if (finished) {
        emit paintFinished(wasAutoTail);
    }

    return samplesRead;
}

size_t SpectralPainter::readIqSamples(std::complex<float> *dest, size_t count)
{
    if (!m_active.load(std::memory_order_acquire) || !dest || count == 0)
        return 0;

    bool finished = false;
    bool wasAutoTail = false;
    size_t samplesRead = 0;

    {
        QMutexLocker locker(&m_mutex);
        if (!m_active.load(std::memory_order_relaxed))
            return 0;

        const size_t available = (m_readIndex < m_iqSamples.size()) ? (m_iqSamples.size() - m_readIndex) : 0;
        samplesRead = std::min(count, available);

        if (samplesRead > 0) {
            std::copy(m_iqSamples.data() + m_readIndex,
                      m_iqSamples.data() + m_readIndex + samplesRead,
                      dest);
            m_readIndex += samplesRead;
        }

        if (m_readIndex >= m_iqSamples.size()) {
            m_active.store(false, std::memory_order_release);
            finished = true;
            wasAutoTail = m_isAutoTail.load(std::memory_order_relaxed);
        }
    }

    if (finished) {
        emit paintFinished(wasAutoTail);
    }

    return samplesRead;
}

size_t SpectralPainter::readSamples(float *dest, size_t count)
{
    if (!m_active.load(std::memory_order_acquire) || !dest || count == 0)
        return 0;

    bool finished = false;
    bool wasAutoTail = false;
    size_t samplesRead = 0;

    {
        QMutexLocker locker(&m_mutex);
        if (!m_active.load(std::memory_order_relaxed))
            return 0;

        const size_t available = (m_readIndex < m_iqSamples.size()) ? (m_iqSamples.size() - m_readIndex) : 0;
        samplesRead = std::min(count, available);

        for (size_t i = 0; i < samplesRead; ++i) {
            dest[i] = m_iqSamples[m_readIndex + i].real();
        }
        m_readIndex += samplesRead;

        if (m_readIndex >= m_iqSamples.size()) {
            m_active.store(false, std::memory_order_release);
            finished = true;
            wasAutoTail = m_isAutoTail.load(std::memory_order_relaxed);
        }
    }

    if (finished) {
        emit paintFinished(wasAutoTail);
    }

    return samplesRead;
}
