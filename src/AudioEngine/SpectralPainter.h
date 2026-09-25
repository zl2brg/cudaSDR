#ifndef SPECTRALPAINTER_H
#define SPECTRALPAINTER_H

#include <QObject>
#include <QString>
#include <QMutex>
#include <vector>
#include <complex>
#include <atomic>
#include "QtWDSP/qtdsp_qComplex.h"

/**
 * @brief SpectralPainter synthesizes direct analytic complex baseband I/Q signals
 *        (or audio signals) that render readable text (such as operator callsign)
 *        on an SDR waterfall display.
 *
 * It rasterizes the specified text into a multi-tone additive signal
 * constrained within the passband (e.g. +600 Hz .. +2400 Hz for USB,
 * -2400 Hz .. -600 Hz for LSB) using Schroeder phase distribution to minimize
 * Peak-to-Average Power Ratio (PAPR).
 * Direct I/Q synthesis completely bypasses the voice audio pipeline (mic AGC,
 * compressor, TX EQ, TX bandpass filter, and Weaver SSB modulator), eliminating
 * intermodulation distortion and filter edge rolloff.
 * No border framing or bounding boxes are added, per user preference.
 */
class SpectralPainter : public QObject {
    Q_OBJECT

public:
    enum class PaintStyle {
        Ticker,
        Banner
    };
    Q_ENUM(PaintStyle)

    explicit SpectralPainter(QObject *parent = nullptr);
    ~SpectralPainter() override = default;

    PaintStyle paintStyle() const;
    void setPaintStyle(PaintStyle style);

    int sampleRate() const { return m_sampleRate; }
    void setSampleRate(int rate);

    float lowFrequencyHz() const { return m_lowHz; }
    float highFrequencyHz() const { return m_highHz; }
    void setFrequencyRange(float lowHz, float highHz);

    int durationMs() const { return m_durationMs; }
    void setDurationMs(int ms);

    QString customText() const;
    void setCustomText(const QString &text);

    bool invertFrequency() const;
    void setInvertFrequency(bool invert);

    bool invertTime() const;
    void setInvertTime(bool invert);

    /**
     * @brief Synthesizes direct complex baseband I/Q samples for the given text.
     * @param text Text to render. If empty, uses customText or callsignFallback.
     * @param callsignFallback Fallback callsign from settings if text is empty.
     * @param invertFrequency Invert frequency mapping (true for LSB mode: negative passband).
     * @return Vector of synthesized 48 kHz complex baseband I/Q samples (-1.0f .. +1.0f).
     */
    std::vector<std::complex<float>> synthesizeIq(const QString &text,
                                                  const QString &callsignFallback = QString(),
                                                  bool invertFrequency = false) const;

    /**
     * @brief Synthesizes audio samples for the given text.
     * @param text Text to render. If empty, uses customText or callsignFallback.
     * @param callsignFallback Fallback callsign from settings if text is empty.
     * @param invertFrequency Invert frequency mapping (useful for LSB mode).
     * @return Vector of synthesized 48 kHz mono float audio samples (-1.0f .. +1.0f).
     */
    std::vector<float> synthesize(const QString &text,
                                  const QString &callsignFallback = QString(),
                                  bool invertFrequency = false) const;

    /**
     * @brief Pre-renders the internal sample buffer using current settings.
     */
    void prepareBuffer(const QString &callsignFallback = QString(),
                       bool invertFrequency = false);

    /**
     * @brief Starts playback of the spectral paint stream.
     * @param isAutoTail True if triggered automatically at PTT release.
     * @param textOverride Optional text override (if empty, uses prepared text).
     * @param callsignFallback Optional callsign fallback if text is empty.
     * @param invertFrequency Invert frequency mapping (e.g. for LSB mode).
     * @return True if started successfully with non-empty buffer.
     */
    bool start(bool isAutoTail = false,
               const QString &textOverride = QString(),
               const QString &callsignFallback = QString(),
               bool invertFrequency = false);

    /**
     * @brief Aborts current playback immediately.
     */
    void stop();

    bool isActive() const { return m_active.load(std::memory_order_acquire); }
    bool isAutoTail() const { return m_isAutoTail.load(std::memory_order_acquire); }

    /**
     * @brief Reads next block of synthesized complex baseband I/Q samples into cpx buffer.
     * @param dest Buffer to copy samples into (cpx = {double re, double im}).
     * @param count Number of samples requested (e.g. DSP_SAMPLE_SIZE).
     * @return Number of samples actually read. If 0 or < count and end reached, active becomes false.
     */
    size_t readIqSamples(cpx *dest, size_t count);

    /**
     * @brief Reads next block of synthesized complex baseband I/Q samples into std::complex<float> buffer.
     * @param dest Buffer to copy samples into.
     * @param count Number of samples requested.
     * @return Number of samples actually read.
     */
    size_t readIqSamples(std::complex<float> *dest, size_t count);

    /**
     * @brief Reads next block of synthesized audio/in-phase samples into float buffer.
     * @param dest Buffer to copy samples into.
     * @param count Number of samples requested.
     * @return Number of samples actually read. If 0 or < count and end reached, active becomes false.
     */
    size_t readSamples(float *dest, size_t count);

signals:
    void paintStarted(bool isAutoTail);
    void paintFinished(bool wasAutoTail);

private:
    mutable QMutex m_mutex;
    int m_sampleRate = 48000;
    float m_lowHz = 600.0f;
    float m_highHz = 2400.0f;
    int m_durationMs = 2000;
    QString m_customText;
    PaintStyle m_paintStyle = PaintStyle::Banner;
    bool m_invertFrequency = false;
    bool m_invertTime = false;

    std::vector<std::complex<float>> synthesizeTickerIq(const QString &text, int sampleRate,
                                                        float lowHz, float highHz, int durationMs,
                                                        bool invertFreq, bool invertTm) const;
    std::vector<std::complex<float>> synthesizeBannerIq(const QString &text, int sampleRate,
                                                        float lowHz, float highHz, int durationMs,
                                                        bool invertFreq, bool invertTm) const;

    std::vector<std::complex<float>> m_iqSamples;
    size_t m_readIndex = 0;
    std::atomic<bool> m_active{false};
    std::atomic<bool> m_isAutoTail{false};
};

#endif // SPECTRALPAINTER_H
