#include "SpectralPainter.h"

#include <QImage>
#include <QDebug>
#include <cmath>
#include <algorithm>
#include <array>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace {

void sanitizePaintRange(float &lowHz, float &highHz)
{
    if (highHz < lowHz)
        std::swap(lowHz, highHz);
    lowHz = qBound(50.0f, lowHz, 6000.0f);
    highHz = qBound(lowHz + 200.0f, highHz, 8000.0f);
}

// Same canvas as the square, which painted cleanly. Each callsign letter is
// drawn this large, one after another — packing 6 letters into 64 columns
// made ~10 px glyphs that read as noise.
constexpr int kPaintW = 64;
constexpr int kPaintH = 48;
constexpr int kGlyphN = 8;

using Glyph8 = std::array<quint8, kGlyphN>;

Glyph8 glyph8(QChar ch)
{
    switch (ch.toUpper().unicode()) {
    case ' ': return {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    case '0': return {{0x3C, 0x66, 0xC3, 0xC3, 0xC3, 0x66, 0x3C, 0x00}};
    case '1': return {{0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00}};
    case '2': return {{0x7E, 0xC3, 0x03, 0x1E, 0x78, 0xC0, 0xFF, 0x00}};
    case '3': return {{0x7E, 0xC3, 0x03, 0x1E, 0x03, 0xC3, 0x7E, 0x00}};
    case '4': return {{0x0E, 0x1E, 0x36, 0x66, 0xFF, 0x06, 0x06, 0x00}};
    case '5': return {{0xFF, 0xC0, 0xFE, 0x03, 0x03, 0xC3, 0x7E, 0x00}};
    case '6': return {{0x3E, 0x60, 0xC0, 0xFE, 0xC3, 0xC3, 0x7E, 0x00}};
    case '7': return {{0xFF, 0x03, 0x06, 0x0C, 0x18, 0x18, 0x18, 0x00}};
    case '8': return {{0x7E, 0xC3, 0xC3, 0x7E, 0xC3, 0xC3, 0x7E, 0x00}};
    case '9': return {{0x7E, 0xC3, 0xC3, 0x7F, 0x03, 0x06, 0x7C, 0x00}};
    case 'A': return {{0x18, 0x3C, 0x66, 0xC3, 0xFF, 0xC3, 0xC3, 0x00}};
    case 'B': return {{0xFE, 0xC3, 0xC3, 0xFE, 0xC3, 0xC3, 0xFE, 0x00}};
    case 'C': return {{0x3E, 0x63, 0xC0, 0xC0, 0xC0, 0x63, 0x3E, 0x00}};
    case 'D': return {{0xFC, 0xC6, 0xC3, 0xC3, 0xC3, 0xC6, 0xFC, 0x00}};
    case 'E': return {{0xFF, 0xC0, 0xC0, 0xFC, 0xC0, 0xC0, 0xFF, 0x00}};
    case 'F': return {{0xFF, 0xC0, 0xC0, 0xFC, 0xC0, 0xC0, 0xC0, 0x00}};
    case 'G': return {{0x3E, 0x63, 0xC0, 0xCF, 0xC3, 0x63, 0x3F, 0x00}};
    case 'H': return {{0xC3, 0xC3, 0xC3, 0xFF, 0xC3, 0xC3, 0xC3, 0x00}};
    case 'I': return {{0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00}};
    case 'J': return {{0x03, 0x03, 0x03, 0x03, 0xC3, 0xC3, 0x7E, 0x00}};
    case 'K': return {{0xC3, 0xC6, 0xCC, 0xF8, 0xCC, 0xC6, 0xC3, 0x00}};
    case 'L': return {{0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xFF, 0x00}};
    case 'M': return {{0xC3, 0xE7, 0xFF, 0xDB, 0xC3, 0xC3, 0xC3, 0x00}};
    case 'N': return {{0xC3, 0xE3, 0xF3, 0xDB, 0xCF, 0xC7, 0xC3, 0x00}};
    case 'O': return {{0x3C, 0x66, 0xC3, 0xC3, 0xC3, 0x66, 0x3C, 0x00}};
    case 'P': return {{0xFE, 0xC3, 0xC3, 0xFE, 0xC0, 0xC0, 0xC0, 0x00}};
    case 'Q': return {{0x3C, 0x66, 0xC3, 0xC3, 0xDB, 0x66, 0x3D, 0x00}};
    case 'R': return {{0xFE, 0xC3, 0xC3, 0xFE, 0xCC, 0xC6, 0xC3, 0x00}};
    case 'S': return {{0x7E, 0xC3, 0xC0, 0x7E, 0x03, 0xC3, 0x7E, 0x00}};
    case 'T': return {{0xFF, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}};
    case 'U': return {{0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0x7E, 0x00}};
    case 'V': return {{0xC3, 0xC3, 0xC3, 0xC3, 0x66, 0x3C, 0x18, 0x00}};
    case 'W': return {{0xC3, 0xC3, 0xC3, 0xDB, 0xFF, 0xE7, 0xC3, 0x00}};
    case 'X': return {{0xC3, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0xC3, 0x00}};
    case 'Y': return {{0xC3, 0xC3, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x00}};
    case 'Z': return {{0xFF, 0x06, 0x0C, 0x18, 0x30, 0x60, 0xFF, 0x00}};
    case '/': return {{0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0x00}};
    case '-': return {{0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00}};
    case '+': return {{0x18, 0x18, 0x18, 0xFF, 0x18, 0x18, 0x18, 0x00}};
    case '.': return {{0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00}};
    default:  return {{0xFF, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xFF, 0x00}};
    }
}

QImage rasterizeBigGlyph(QChar ch)
{
    QImage img(kPaintW, kPaintH, QImage::Format_Grayscale8);
    img.fill(0);
    if (ch == QLatin1Char(' '))
        return img;

    const int scale = qMin(kPaintW / kGlyphN, kPaintH / kGlyphN);
    const int usedW = kGlyphN * scale;
    const int usedH = kGlyphN * scale;
    const int destX = (kPaintW - usedW) / 2;
    const int destY = (kPaintH - usedH) / 2;
    const Glyph8 g = glyph8(ch);

    for (int gy = 0; gy < kGlyphN; ++gy) {
        const quint8 row = g[static_cast<size_t>(gy)];
        for (int gx = 0; gx < kGlyphN; ++gx) {
            if ((row & (1 << (7 - gx))) == 0)
                continue;
            for (int oy = 0; oy < scale; ++oy) {
                uchar *line = img.scanLine(destY + gy * scale + oy);
                for (int ox = 0; ox < scale; ++ox)
                    line[destX + gx * scale + ox] = 255;
            }
        }
    }
    return img;
}

QImage rasterizeSquare()
{
    QImage img(kPaintW, kPaintH, QImage::Format_Grayscale8);
    img.fill(0);
    constexpr int t = 6;
    for (int y = 0; y < kPaintH; ++y) {
        uchar *line = img.scanLine(y);
        for (int x = 0; x < kPaintW; ++x) {
            if (y < t || y >= kPaintH - t || x < t || x >= kPaintW - t)
                line[x] = 255;
        }
    }
    return img;
}

QImage rasterizeCircle()
{
    constexpr int t = 5;
    const double cx = (kPaintW - 1) / 2.0;
    const double cy = (kPaintH - 1) / 2.0;
    const double rx = std::max(1.0, (kPaintW - 1) / 2.0 - 1.0);
    const double ry = std::max(1.0, (kPaintH - 1) / 2.0 - 1.0);
    const double inner = 1.0 - (static_cast<double>(t) / std::min(rx, ry));
    QImage img(kPaintW, kPaintH, QImage::Format_Grayscale8);
    img.fill(0);
    for (int y = 0; y < kPaintH; ++y) {
        uchar *line = img.scanLine(y);
        for (int x = 0; x < kPaintW; ++x) {
            const double nx = (x - cx) / rx;
            const double ny = (y - cy) / ry;
            const double d = std::sqrt(nx * nx + ny * ny);
            if (d <= 1.0 && d >= inner)
                line[x] = 255;
        }
    }
    return img;
}

QImage rasterizeBanner(const QString &text)
{
    const QString key = text.trimmed().toUpper();
    if (key == QLatin1String("SQUARE"))
        return rasterizeSquare();
    if (key == QLatin1String("CIRCLE"))
        return rasterizeCircle();
    return rasterizeBigGlyph(text.isEmpty() ? QLatin1Char(' ') : text.at(0));
}

bool isShapeKeyword(const QString &text)
{
    const QString key = text.trimmed().toUpper();
    return key == QLatin1String("SQUARE") || key == QLatin1String("CIRCLE");
}

} // namespace


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
    sanitizePaintRange(lowHz, highHz);
    QMutexLocker locker(&m_mutex);
    m_lowHz = lowHz;
    m_highHz = highHz;
}

void SpectralPainter::setDurationMs(int ms)
{
    QMutexLocker locker(&m_mutex);
    m_durationMs = qBound(500, ms, 15000);
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
    {
        QMutexLocker locker(&m_mutex);
        sampleRate = m_sampleRate;
        lowHz = m_lowHz;
        highHz = m_highHz;
        durationMs = m_durationMs;
        invertFreq = invertFrequency || m_invertFrequency;
        invertTm = m_invertTime;
    }

    if (isShapeKeyword(targetText)) {
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

    constexpr int numBands = kPaintW;
    constexpr int charHeight = kPaintH;

    std::vector<QImage> charImages;
    charImages.reserve(numChars);
    for (int i = 0; i < numChars; ++i)
        charImages.push_back(rasterizeBigGlyph(text.at(i)));

    // durationMs is the hold time for one letter so a callsign stays as tall as "A".
    const int totalMs = qMax(1, durationMs) * numChars;
    const double durationSec = static_cast<double>(totalMs) / 1000.0;
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
    // Blank gap so successive letters do not smear together on the waterfall.
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
            const int y = qBound(0, static_cast<int>(std::lround(v)), charHeight - 1);
            const uchar *line = charImages[cIdx].constScanLine(y);

            for (int x = 0; x < numBands; ++x) {
                if (line[x] < 128)
                    continue;
                sampleRe += std::cos(phi[x]);
                sampleIm += std::sin(phi[x]);
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
    const QImage img = rasterizeBanner(text);
    if (img.isNull() || img.width() <= 0 || img.height() <= 0)
        return {};

    const int numBands = img.width();
    const int imgHeight = img.height();

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
        const int y = qBound(0, static_cast<int>(std::lround(v)), imgHeight - 1);
        const uchar *line = img.constScanLine(y);

        double sampleRe = 0.0;
        double sampleIm = 0.0;
        for (int x = 0; x < numBands; ++x) {
            if (line[x] >= 128) {
                sampleRe += std::cos(phi[x]);
                sampleIm += std::sin(phi[x]);
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
