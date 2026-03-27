#pragma once

#include <QObject>
#include <QAudioSink>
#include <QAudioDevice>
#include <QAudioFormat>
#include <QMap>
#include <QMutex>
#include <QVector>
#include <QString>
#include <functional>

/**
 * AudioOutputManager (Stereo + DSP hook version)
 * - Manages stereo audio device output for multiple receivers.
 * - Allows assigning a DSP callback per receiver or for the mixed output.
 */
class AudioOutputManager : public QObject
{
    Q_OBJECT
public:
    explicit AudioOutputManager(QObject* parent = nullptr);

    using DspCallback = std::function<void(float* lr, int frames)>;

    // List all available output devices
    QVector<QAudioDevice> availableDevices() const;

    // Assign receiver (by ID) to an audio device (by QAudioDevice)
    void assignReceiverToDevice(const QString& receiverId, const QAudioDevice& device);

    // Remove receiver (mute)
    void removeReceiver(const QString& receiverId);

    // Set the audio format for a receiver (or default)
    void setReceiverFormat(const QString& receiverId, const QAudioFormat& format);

    // Set DSP callback for a receiver (stereo float, frames = # of samples per channel)
    void setReceiverDspCallback(const QString& receiverId, DspCallback cb);

    // Set DSP callback for the mixed output (all receivers mixed, then processed here)
    void setMixDspCallback(DspCallback cb);

    // Feed stereo float samples to a receiver's output (interleaved L/R)
    void writeAudio(const QString& receiverId, const float* interleavedLR, int frames);

    // Enable/disable mixing all receivers to one device
    void setMixAllToOneDevice(bool enabled, const QAudioDevice& device = QAudioDevice());

signals:
    void deviceListChanged();

private:
    struct ReceiverAudio {
        QAudioDevice device;
        QAudioFormat format;
        QAudioSink* sink = nullptr;
        QIODevice* io = nullptr;
        DspCallback dsp;
        QByteArray buf; // temp buffer for conversion
    };

    QMap<QString, ReceiverAudio> m_receivers;
    mutable QMutex m_mutex;

    // For "mix all" mode
    bool m_mixAll = true;
    QAudioDevice m_mixDevice;
    QAudioSink* m_mixSink = nullptr;
    QIODevice* m_mixIO = nullptr;
    DspCallback m_mixDsp;
    QByteArray m_mixBuf;

    void refreshDeviceList();
    void closeAllSinks();

    // Interleaved stereo float -> signed 16-bit PCM
    static void floatToInt16(const float* in, qint16* out, int frames);
};
