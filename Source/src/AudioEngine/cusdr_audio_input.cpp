//
// Created by Simon Eatough, ZL2BRG on 14/09/21.
//

#include <QCoreApplication>
#include <QMetaObject>
#include <QSet>
#include <QDebug>
#include <QThread>
#include <QFile>
#include <QElapsedTimer>
#include <QtEndian>
#define LOG_AUDIO_INPUT

#include "cusdr_audio_input.h"


PAudioInput::PAudioInput(QObject *parent) 
    : QObject(parent)
    , set(Settings::instance())
    , m_audioSource(nullptr)
    , m_audioInputDevice(nullptr)
    , m_running(false)
    , m_sampleRate(48000)
    , m_bufferSize(AUDIO_FRAMESIZE)
    , m_deviceIndex(0)
{
    CHECKED_CONNECT(set,
                    SIGNAL(micInputChanged(int)),
                    this,
                    SLOT(MicInputChanged(int)));

    m_deviceIndex = set->getMicInputDev();
    
    audioinputBuffer.resize(AUDIO_FRAMESIZE);
    audioinputBuffer.fill(0.0);
    
    Setup();
    
    AUDIO_INPUT_DEBUG << "Audio Buffer Size: " << audioinputBuffer.size();
}

PAudioInput::~PAudioInput()
{
    Stop();
    if (m_audioSource) {
        delete m_audioSource;
        m_audioSource = nullptr;
    }
}

void PAudioInput::Setup() {
    m_mutex.lock();
    
    // Clean up existing audio source
    if (m_audioSource) {
        if (m_running) {
            m_audioSource->stop();
        }
        delete m_audioSource;
        m_audioSource = nullptr;
    }
    
    // Configure audio format
    m_format.setSampleRate(m_sampleRate);
    m_format.setChannelCount(1);  // Mono
    m_format.setSampleFormat(QAudioFormat::Int16); // 16-bit PCM

    // Get audio input device
    QAudioDevice inputDevice;
    if (m_deviceIndex == 0) {
        // Index 0 is HPSDR local mic input - use default device
        inputDevice = QMediaDevices::defaultAudioInput();
        AUDIO_INPUT_DEBUG << "Using default audio input device for HPSDR";
    } else {
        // Use specific device (adjust index since 0 is HPSDR)
        QList<QAudioDevice> devices = QMediaDevices::audioInputs();
        int actualIndex = m_deviceIndex - 1;
        if (actualIndex >= 0 && actualIndex < devices.size()) {
            inputDevice = devices[actualIndex];
            AUDIO_INPUT_DEBUG << "Using audio input device:" << inputDevice.description();
        } else {
            inputDevice = QMediaDevices::defaultAudioInput();
            AUDIO_INPUT_DEBUG << "Device index out of range, using default";
        }
    }
    
    // Check if format is supported
    if (!inputDevice.isFormatSupported(m_format)) {
        AUDIO_INPUT_DEBUG << "Format not supported, using nearest format";
        m_format = inputDevice.preferredFormat();
    }

    // Create the audio source
    m_audioSource = new QAudioSource(inputDevice, m_format, this);
    m_audioSource->setBufferSize(4 * m_bufferSize); // Larger buffer to prevent underruns
    
    AUDIO_INPUT_DEBUG << "Audio input setup complete";
    
    m_mutex.unlock();
}

void PAudioInput::MicInputChanged(int value) {
    /* Index 0 is HPSDR local mic input */
    AUDIO_INPUT_DEBUG << "Mic Input Changed to:" << value;
    
    m_deviceIndex = value;
    
    Stop();
    Setup();
    
    if (m_deviceIndex > 0) {
        Start();
        AUDIO_INPUT_DEBUG << "External mic mode started";
    } else {
        AUDIO_INPUT_DEBUG << "Local HPSDR Mic Mode Selected";
    }
}

void PAudioInput::Stop() {
    m_mutex.lock();
    if (m_running && m_audioSource) {
        if (m_audioInputDevice) {
            disconnect(m_audioInputDevice, &QIODevice::readyRead,
                      this, &PAudioInput::handleReadyRead);
        }
        m_audioSource->stop();
        m_audioInputDevice = nullptr;
        m_running = false;
        AUDIO_INPUT_DEBUG << "Audio input stopped";
    }
    m_mutex.unlock();
}

bool PAudioInput::Start() {
    if (m_running)
        return true;

    m_mutex.lock();
    if (m_audioSource) {
        // Start the audio input
        m_audioInputDevice = m_audioSource->start();
        if (m_audioInputDevice) {
            connect(m_audioInputDevice, &QIODevice::readyRead,
                    this, &PAudioInput::handleReadyRead);
            m_running = true;
            AUDIO_INPUT_DEBUG << "Audio input started";
        } else {
            AUDIO_INPUT_DEBUG << "Could not start audio input";
            m_mutex.unlock();
            return false;
        }
    }
    m_mutex.unlock();
    return true;
}

void PAudioInput::handleReadyRead()
{
    m_mutex.lock();
    if (m_running && m_audioInputDevice) {
        // Read all available data
        QByteArray buffer = m_audioInputDevice->readAll();
        if (!buffer.isEmpty()) {
            processAudioData(buffer);
        }
    }
    m_mutex.unlock();
}

void PAudioInput::processAudioData(const QByteArray &data)
{
    // Process the incoming audio data
    const qint16 *ptr = reinterpret_cast<const qint16 *>(data.constData());
    int numSamples = data.size() / sizeof(qint16);
    if (numSamples > 0)
    {
    // Resize buffer to hold all samples
    audioinputBuffer.resize(numSamples);
    qDebug() << "Processing" << numSamples << "audio samples";
    // Convert all PCM samples to double in the range [-1.0, 1.0]
    for (int i = 0; i < numSamples; ++i) {
        audioinputBuffer[i] = static_cast<double>(ptr[i]) / 32768.0;
    }

    // Add entire buffer to the queue
    m_faudioInQueue.enqueue(audioinputBuffer);

    // Emit signal to notify that data is ready
    emit tx_mic_data_ready();
    }
}

