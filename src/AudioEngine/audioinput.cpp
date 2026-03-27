#include "audioinput.h"
#include <QDebug>
#include <QtEndian>

AudioInput::AudioInput(QObject *parent)
    : QObject(parent)
    , m_audioSource(nullptr)
    , m_audioInputDevice(nullptr)
    , m_running(false)
    , m_sampleRate(48000)  // Default sample rate
    , m_bufferSize(1024)   // Default buffer size
    , set(Settings::instance())
{
    setupAudioSource();
}

AudioInput::~AudioInput()
{
    Stop();
    if (m_audioSource) {
        delete m_audioSource;
    }
}

void AudioInput::setupAudioSource()
{
    // Configure audio format
    m_format.setSampleRate(m_sampleRate);
    m_format.setChannelCount(1);          // Mono
    m_format.setSampleFormat(QAudioFormat::Int16); // 16-bit PCM

    // Get default input device
    QAudioDevice inputDevice = QMediaDevices::defaultAudioInput();
    if (!inputDevice.isFormatSupported(m_format)) {
        qWarning() << "Default format not supported, trying to use the nearest";
        m_format = inputDevice.preferredFormat();
    }

    // Create the audio source
    m_audioSource = new QAudioSource(inputDevice, m_format);
    m_audioSource->setBufferSize(4 * m_bufferSize); // Larger buffer to prevent underruns
}

void AudioInput::Start()
{
    if (m_running)
        return;

    m_mutex.lock();
    if (m_audioSource) {
        // Clear any existing queue
        m_faudioInQueue.clear();

        // Start the audio input
        m_audioInputDevice = m_audioSource->start();
        if (m_audioInputDevice) {
            connect(m_audioInputDevice, &QIODevice::readyRead,
                    this, &AudioInput::handleReadyRead);
            m_running = true;
            qDebug() << "Audio input started";
        } else {
            qWarning() << "Could not start audio input";
        }
    }
    m_mutex.unlock();
}

void AudioInput::Stop()
{
    m_mutex.lock();
    if (m_running && m_audioSource) {
        disconnect(m_audioInputDevice, &QIODevice::readyRead,
                   this, &AudioInput::handleReadyRead);
        m_audioSource->stop();
        m_audioInputDevice = nullptr;
        m_running = false;
        qDebug() << "Audio input stopped";
    }
    m_mutex.unlock();
}

void AudioInput::handleReadyRead()
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

void AudioInput::processAudioData(const QByteArray &data)
{
    // Process the incoming audio data
    const qint16 *ptr = reinterpret_cast<const qint16 *>(data.constData());
    int numSamples = data.size() / sizeof(qint16);

    // Create a new AUDIOBUF with the sample data
    AUDIOBUF audioBuf;
    audioBuf.resize(numSamples);

    // Convert the PCM samples to float in the range [-1.0, 1.0]
    for (int i = 0; i < numSamples; ++i) {
        audioBuf[i] = static_cast<double>(ptr[i]) / 32768.0;
    }

    // Add to the queue
    m_faudioInQueue.enqueue(audioBuf);

    // Emit signal to notify that data is ready
    emit tx_mic_data_ready();
}
