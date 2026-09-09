//
// Created by simon on 14/09/21.
//

#include <QObject>
#include <QList>
#include <QtMultimedia/QAudioSource>
#include <QtMultimedia/QAudioDevice>
#include <QtMultimedia/QMediaDevices>
#include <QBuffer>
#include <QVector>
#include <QMutex>
#include <QIODevice>

#include "cusdr_settings.h"
#include "Util/SpscRingBuffer.h"
#include <vector>

#ifndef CUDASDR_CUSDR_AUDIO_INPUT_H
#define CUDASDR_CUSDR_AUDIO_INPUT_H


#define AUDIO_FRAMESIZE  1024
#define AUDIO_IN_PACKET_SIZE 4096//2048

#define LOG_AUDIO_INPUT

#ifdef LOG_AUDIO_INPUT
#   define AUDIO_INPUT_DEBUG qDebug().nospace() << "AudioInput::\t"
#else
#   define AUDIO_INPUT_DEBUG nullDebug()
#endif


typedef QVector<double> AUDIOBUF;

Q_DECLARE_METATYPE (AUDIOBUF)

class TransmitAudioInput : public QObject {
Q_OBJECT
public:
    TransmitAudioInput(QObject *parent = nullptr);
    ~TransmitAudioInput();
    static QList<QAudioDevice> availableAudioInputDevices();
    void Setup();
    void Stop();
    bool Start();
    QList<QAudioDevice> getAudioInputDevices() const;
    void clearTxQueues();
    
public:
    QStringList paDeviceList;
    AUDIOBUF  audioinputBuffer;

    // Lock-free single-producer single-consumer circular buffers for mic audio
    SpscRingBuffer<float> m_faudioRing{16384};
    SpscRingBuffer<float> m_netAudioRing{16384};

    void pushMicAudio(const float* samples, size_t count);
    void pushNetAudio(const float* samples, size_t count);
    size_t readMicAudio(float* dest, size_t count);
    size_t readNetAudio(float* dest, size_t count);
    bool readMicAudioBlock(float* dest, size_t count);
    bool readNetAudioBlock(float* dest, size_t count);
    size_t micAudioAvailable() const { return m_faudioRing.availableRead() + m_micFetchResidual.size(); }
    size_t netAudioAvailable() const { return m_netAudioRing.availableRead() + m_netFetchResidual.size(); }
    bool hasPendingNetAudio() const { return netAudioAvailable() > 0; }
    bool hasPendingMicAudio() const { return micAudioAvailable() > 0; }

signals:
    void tx_mic_data_ready();

private:
    void setupAudioSource();
    void processAudioData(const QByteArray &data);
    void stopHardware(); // stop device without clearing m_txActive
    bool isTransmitting() const;
    bool shouldCaptureWhileTx() const;
    
private slots:
    void MicInputChanged(int source);
    void DigitalAudioInputChanged(int index);
    void dspModeChanged(int rx, DSPMode mode);
    void handleReadyRead();

private:
    Settings*           set;
    QAudioSource*       m_audioSource;
    QIODevice*          m_audioInputDevice;
    QAudioFormat        m_format;
    QMutex              m_mutex;
    bool                m_running;
    QVector<float>      m_residualBuffer;
    std::vector<float>  m_micFetchResidual;
    std::vector<float>  m_netFetchResidual;
    int                 m_sampleRate;
    int                 m_bufferSize;
    int                 m_deviceIndex;
    int                 m_digitalDeviceIndex;
    bool                m_isDigitalMode;
};

#endif //CUDASDR_CUSDR_AUDIO_INPUT_H

