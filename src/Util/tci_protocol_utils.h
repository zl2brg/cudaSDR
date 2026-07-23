#ifndef TCI_PROTOCOL_UTILS_H
#define TCI_PROTOCOL_UTILS_H

#include "cusdr_hamDatabase.h"
#include "Settings/SettingsTypes.h"

#include <QByteArray>
#include <QVector>
#include <QStringList>
#include <QtEndian>
#include <QtGlobal>
#include <cstring>

namespace TciProtocol {

constexpr int kStreamHeaderBytes = 64;
constexpr uint32_t kIqStreamType = 0;
constexpr uint32_t kRxAudioStreamType = 1;
constexpr uint32_t kTxAudioStreamType = 2;
constexpr uint32_t kTxChronoStreamType = 3;
constexpr double kRxAudioRateHz = 48000.0;
// ExpertSDR / WSJT-X TX_CHRONO block: 2048 float samples = 1024 stereo frames.
constexpr int kTxChronoSamples = 2048;
constexpr int kTxChronoStereoFrames = kTxChronoSamples / 2;
constexpr qint64 kVfoMinHz = 135700;
constexpr qint64 kVfoMaxHz = 61868000;
constexpr int kDspSampleSize = 1024; // matches DSP_SAMPLE_SIZE in cusdr_settings.h
constexpr int kStreamFormatInt16 = 0;
constexpr int kStreamFormatFloat32 = 3;

struct StreamHeader {
    quint32 receiver = 0;
    quint32 sampleRate = 0;
    quint32 format = 0;
    quint32 length = 0;
    quint32 streamType = 0;
    quint32 channels = 0;
};

inline bool isValidVfoHz(qint64 freq)
{
    return freq >= kVfoMinHz && freq <= kVfoMaxHz;
}

inline QString tciMessage(const QString &name, const QStringList &args = {})
{
    // ExpertSDR / WSJT-X parsers match command names case-sensitively on the
    // lowercase forms (e.g. "start", "protocol"). Always emit lowercase.
    const QString cmd = name.toLower();
    if (args.isEmpty())
        return cmd + ';';
    return cmd + ':' + args.join(',') + ';';
}

inline bool parseBoolArg(const QString &value)
{
    const QString v = value.trimmed().toLower();
    return v == QLatin1String("1")
        || v == QLatin1String("true")
        || v == QLatin1String("on")
        || v == QLatin1String("tx");
}

inline QString dspModeToTci(DSPMode mode)
{
    switch (mode) {
        case LSB:  return QStringLiteral("lsb");
        case USB:  return QStringLiteral("usb");
        case DSB:  return QStringLiteral("dsb");
        case CWL:  return QStringLiteral("cwl");
        case CWU:  return QStringLiteral("cw");
        case FMN:  return QStringLiteral("fm");
        case AM:   return QStringLiteral("am");
        case DIGU: return QStringLiteral("digu");
        case DIGL: return QStringLiteral("digl");
        case SAM:  return QStringLiteral("sam");
        case FDV:  return QStringLiteral("fdv");
        default:   return QStringLiteral("usb");
    }
}

inline DSPMode tciModeToDsp(const QString &mode)
{
    const QString m = mode.trimmed().toUpper();
    if (m == QLatin1String("LSB"))    return LSB;
    if (m == QLatin1String("USB"))    return USB;
    if (m == QLatin1String("DSB"))    return DSB;
    if (m == QLatin1String("CWL"))    return CWL;
    if (m == QLatin1String("CW"))     return CWU;
    if (m == QLatin1String("CWU"))    return CWU;
    if (m == QLatin1String("FM"))     return FMN;
    if (m == QLatin1String("NFM"))    return FMN;
    if (m == QLatin1String("FMN"))    return FMN;
    if (m == QLatin1String("AM"))     return AM;
    if (m == QLatin1String("AMS"))    return SAM;
    if (m == QLatin1String("SAM"))    return SAM;
    if (m == QLatin1String("DIGU"))   return DIGU;
    if (m == QLatin1String("DIGL"))   return DIGL;
    if (m == QLatin1String("FDV"))    return FDV;
    if (m == QLatin1String("FREEDV")) return FDV;
    return USB;
}

inline bool isTrxActive(RadioState state)
{
    return state == RadioState::MOX || state == RadioState::TUNE || state == RadioState::DUPLEX;
}

inline bool acceptsTxAudio(RadioState state)
{
    return state == RadioState::MOX || state == RadioState::TUNE;
}

inline int effectiveIqSampleRate(int actualRate, int audioRate = static_cast<int>(kRxAudioRateHz),
                               int rateOverride = 0)
{
    if (rateOverride > 0)
        return rateOverride;

    int rate = actualRate > 0 ? actualRate : audioRate;
    while (rate <= audioRate)
        rate *= 2;
    return rate;
}

inline int parseAudioFormat(const QString &value)
{
    const QString fmt = value.trimmed().toLower();
    if (fmt == QLatin1String("float32"))
        return 3;
    if (fmt == QLatin1String("int32"))
        return 2;
    if (fmt == QLatin1String("int24"))
        return 1;
    if (fmt == QLatin1String("int16"))
        return 0;
    return 3;
}

inline bool parseStreamHeader(const QByteArray &message, StreamHeader &out)
{
    if (message.size() < kStreamHeaderBytes)
        return false;

    const uchar *hdr = reinterpret_cast<const uchar *>(message.constData());
    out.receiver = qFromLittleEndian<quint32>(hdr + 0);
    out.sampleRate = qFromLittleEndian<quint32>(hdr + 4);
    out.format = qFromLittleEndian<quint32>(hdr + 8);
    out.length = qFromLittleEndian<quint32>(hdr + 20);
    out.streamType = qFromLittleEndian<quint32>(hdr + 24);
    out.channels = qFromLittleEndian<quint32>(hdr + 28);
    return true;
}

inline QByteArray buildStreamHeader(const StreamHeader &hdr)
{
    QByteArray frame(kStreamHeaderBytes, 0);
    auto writeU32 = [&frame](int offset, quint32 value) {
        qToLittleEndian(value, reinterpret_cast<uchar *>(frame.data() + offset));
    };

    writeU32(0, hdr.receiver);
    writeU32(4, hdr.sampleRate);
    writeU32(8, hdr.format);
    writeU32(12, 0);
    writeU32(16, 0);
    writeU32(20, hdr.length);
    writeU32(24, hdr.streamType);
    writeU32(28, hdr.channels);
    return frame;
}

struct EncodedStreamPayload {
    QByteArray data;
    int elementCount = 0;
    bool ok = false;
};

inline QByteArray buildStreamFrame(const StreamHeader &hdr, const QByteArray &payload)
{
    QByteArray frame = buildStreamHeader(hdr);
    frame.append(payload);
    return frame;
}

inline EncodedStreamPayload encodeRxAudioPayload(const float *stereoInterleaved, int stereoFloatCount,
                                                 int format, int channels)
{
    EncodedStreamPayload out;
    if (!stereoInterleaved || stereoFloatCount < 2)
        return out;

    const int frames = stereoFloatCount / 2;
    if (format == kStreamFormatFloat32) {
        if (channels == 1) {
            out.elementCount = frames;
            out.data.resize(out.elementCount * static_cast<int>(sizeof(float)));
            float *dst = reinterpret_cast<float *>(out.data.data());
            for (int i = 0; i < frames; ++i)
                dst[i] = stereoInterleaved[i * 2];
        } else {
            out.elementCount = stereoFloatCount;
            out.data.resize(out.elementCount * static_cast<int>(sizeof(float)));
            std::memcpy(out.data.data(), stereoInterleaved,
                        static_cast<size_t>(out.elementCount) * sizeof(float));
        }
        out.ok = true;
    } else if (format == kStreamFormatInt16) {
        if (channels == 1) {
            out.elementCount = frames;
            out.data.resize(out.elementCount * static_cast<int>(sizeof(qint16)));
            auto *encoded = reinterpret_cast<qint16 *>(out.data.data());
            for (int i = 0; i < frames; ++i) {
                const float clamped = qBound(-1.0f, stereoInterleaved[i * 2], 1.0f);
                encoded[i] = static_cast<qint16>(clamped * 32767.0f);
            }
        } else {
            out.elementCount = stereoFloatCount;
            out.data.resize(out.elementCount * static_cast<int>(sizeof(qint16)));
            auto *encoded = reinterpret_cast<qint16 *>(out.data.data());
            for (int i = 0; i < out.elementCount; ++i) {
                const float clamped = qBound(-1.0f, stereoInterleaved[i], 1.0f);
                encoded[i] = static_cast<qint16>(clamped * 32767.0f);
            }
        }
        out.ok = true;
    }

    return out;
}

inline QByteArray buildRxAudioFrame(int rx, int sampleRate, int format, int channels,
                                    const float *stereoInterleaved, int stereoFloatCount)
{
    const EncodedStreamPayload payload =
        encodeRxAudioPayload(stereoInterleaved, stereoFloatCount, format, channels);
    if (!payload.ok)
        return {};

    StreamHeader hdr;
    hdr.receiver = static_cast<quint32>(rx);
    hdr.sampleRate = static_cast<quint32>(sampleRate);
    hdr.format = static_cast<quint32>(format);
    hdr.length = static_cast<quint32>(payload.elementCount);
    hdr.streamType = kRxAudioStreamType;
    hdr.channels = static_cast<quint32>(channels);
    return buildStreamFrame(hdr, payload.data);
}

inline EncodedStreamPayload encodeIqPayload(const float *iqInterleaved, int iqFloatCount, int format)
{
    EncodedStreamPayload out;
    if (!iqInterleaved || iqFloatCount < 2)
        return out;

    if (format == kStreamFormatFloat32) {
        out.elementCount = iqFloatCount;
        out.data.resize(out.elementCount * static_cast<int>(sizeof(float)));
        std::memcpy(out.data.data(), iqInterleaved,
                    static_cast<size_t>(out.elementCount) * sizeof(float));
        out.ok = true;
    } else if (format == kStreamFormatInt16) {
        out.elementCount = iqFloatCount;
        out.data.resize(out.elementCount * static_cast<int>(sizeof(qint16)));
        auto *encoded = reinterpret_cast<qint16 *>(out.data.data());
        for (int i = 0; i < out.elementCount; ++i) {
            const float clamped = qBound(-1.0f, iqInterleaved[i], 1.0f);
            encoded[i] = static_cast<qint16>(clamped * 32767.0f);
        }
        out.ok = true;
    }

    return out;
}

inline QByteArray buildIqFrame(int rx, int sampleRate, int format, int channels,
                               const float *iqInterleaved, int iqFloatCount)
{
    const EncodedStreamPayload payload = encodeIqPayload(iqInterleaved, iqFloatCount, format);
    if (!payload.ok)
        return {};

    StreamHeader hdr;
    hdr.receiver = static_cast<quint32>(rx);
    hdr.sampleRate = static_cast<quint32>(sampleRate);
    hdr.format = static_cast<quint32>(format);
    hdr.length = static_cast<quint32>(payload.elementCount);
    hdr.streamType = kIqStreamType;
    hdr.channels = static_cast<quint32>(channels);
    return buildStreamFrame(hdr, payload.data);
}

inline QByteArray buildTxAudioFrame(const float *monoSamples, int sampleCount, int sampleRate = 48000,
                                    int receiver = 0)
{
    StreamHeader hdr;
    hdr.receiver = static_cast<quint32>(receiver);
    hdr.sampleRate = static_cast<quint32>(sampleRate);
    hdr.format = 3;
    hdr.length = static_cast<quint32>(sampleCount);
    hdr.streamType = kTxAudioStreamType;
    hdr.channels = 1;

    QByteArray frame = buildStreamHeader(hdr);
    frame.append(reinterpret_cast<const char *>(monoSamples),
                 sampleCount * static_cast<int>(sizeof(float)));
    return frame;
}

/** Header-only TX_CHRONO timing frame. WSJT-X only emits TX audio in response. */
inline QByteArray buildTxChronoFrame(int receiver = 0, int sampleRate = 48000,
                                     int lengthSamples = kTxChronoSamples)
{
    StreamHeader hdr;
    hdr.receiver = static_cast<quint32>(receiver);
    hdr.sampleRate = static_cast<quint32>(sampleRate);
    hdr.format = kStreamFormatFloat32;
    hdr.length = static_cast<quint32>(lengthSamples);
    hdr.streamType = kTxChronoStreamType;
    hdr.channels = 2;
    return buildStreamHeader(hdr);
}

inline bool decodeTxAudioMonoSamples(const QByteArray &payload, int format, int channels,
                                     QVector<double> &outSamples)
{
    if (channels < 1)
        channels = 1;

    const int payloadBytes = payload.size();
    if (format == 3) { // FLOAT32
        const int floatCount = payloadBytes / static_cast<int>(sizeof(float));
        const int frames = floatCount / channels;
        const float *f = reinterpret_cast<const float *>(payload.constData());
        outSamples.reserve(outSamples.size() + frames);
        for (int i = 0; i < frames; ++i)
            outSamples.append(static_cast<double>(f[i * channels]));
        return frames > 0;
    }

    if (format == 0) { // INT16
        const int intCount = payloadBytes / static_cast<int>(sizeof(qint16));
        const int frames = intCount / channels;
        const qint16 *s = reinterpret_cast<const qint16 *>(payload.constData());
        outSamples.reserve(outSamples.size() + frames);
        for (int i = 0; i < frames; ++i)
            outSamples.append(static_cast<double>(s[i * channels]) / 32768.0);
        return frames > 0;
    }

    return false;
}

/** True when float32 samples look like ExpertSDR duplicated stereo (L≈R).
 *  Treating that layout as mono doubles each sample and plays TX at half speed. */
inline bool isDuplicatedStereoFloat32(const float *samples, int floatCount)
{
    if (!samples || floatCount < 4 || (floatCount % 2) != 0)
        return false;

    const int pairsToCheck = qMin(floatCount / 2, 128);
    int duplicatedPairs = 0;
    for (int i = 0; i < pairsToCheck; ++i) {
        if (qAbs(samples[i * 2] - samples[i * 2 + 1]) < 1.0e-6f)
            ++duplicatedPairs;
    }
    return duplicatedPairs >= (pairsToCheck * 9) / 10;
}

struct TxAudioFloatLayout {
    int useFloats = 0;
    int channels = 1;
    bool ok = false;
};

/**
 * Resolve how many float32 values to decode and as mono vs stereo.
 *
 * WSJT-X geometric signature (deterministic — do not inspect sample values):
 *   format=float32, length>0, payload capacity >= length * sizeof(float) * 2
 * (e.g. length=2048 → 16384-byte payload). Only the first `length` floats are
 * live stereo L/R pairs; trailing capacity is unused. Always decode those as
 * channels=2 → length/2 mono (1024 for length=2048 = 1 DSP block). Sample
 * content (L≈R, silence) is unreliable on tones/ramps and must not gate this.
 *
 * Tight payloads (Android / other ExpertSDR apps: availFloats ≈ length) keep
 * the prior path with duplicated-stereo detection on the declared float count.
 */
inline TxAudioFloatLayout resolveTxAudioFloat32Layout(const float *samples, int availFloats,
                                                      int headerLength, int headerChannels)
{
    TxAudioFloatLayout out;
    if (!samples || availFloats <= 0)
        return out;

    const int length = headerLength > 0 ? headerLength : availFloats;

    // WSJT-X oversized capacity: geometry alone decides layout.
    if (headerLength > 0 && availFloats >= length * 2 && length >= 2 && (length % 2) == 0) {
        out.useFloats = length;
        out.channels = 2;
        out.ok = true;
        return out;
    }

    // Tight / non-oversized payload (Android TCI, etc.).
    int useFloats = availFloats;
    if (headerLength > 0)
        useFloats = qMin(length, availFloats);
    if (useFloats <= 0)
        return out;

    int channels = 1;
    if (headerChannels == 1) {
        channels = 1;
    } else if (isDuplicatedStereoFloat32(samples, useFloats)
               || headerChannels >= 2) {
        channels = 2;
    } else if ((useFloats % 2) == 0 && useFloats >= 4) {
        channels = 2;
    }

    out.useFloats = useFloats;
    out.channels = channels;
    out.ok = true;
    return out;
}

struct TxAudioChunkResult {
    QVector<QVector<double>> blocks;
    QVector<double> residual;
    int droppedBlocks = 0;
};

inline TxAudioChunkResult chunkTxAudioResidual(QVector<double> residual, int dspSampleSize,
                                             int maxQueueBlocks, int currentQueueCount)
{
    TxAudioChunkResult result;
    result.residual = std::move(residual);

    while (result.residual.size() >= dspSampleSize) {
        if (currentQueueCount + result.blocks.size() >= maxQueueBlocks) {
            result.droppedBlocks++;
            result.residual.remove(0, dspSampleSize);
            continue;
        }

        result.blocks.append(result.residual.mid(0, dspSampleSize));
        result.residual.remove(0, dspSampleSize);
    }

    return result;
}

} // namespace TciProtocol

#endif // TCI_PROTOCOL_UTILS_H
