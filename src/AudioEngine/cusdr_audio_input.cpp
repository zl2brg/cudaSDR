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

namespace {
int findAudioInputByDescription(const QList<QAudioDevice> &devices, const QString &name)
{
    for (int i = 0; i < devices.size(); ++i) {
        if (devices.at(i).description() == name)
            return i;
    }
    return -1;
}
}


TransmitAudioInput::TransmitAudioInput(QObject *parent) 
    : QObject(parent)
    , set(Settings::instance())
    , m_audioSource(nullptr)
    , m_audioInputDevice(nullptr)
    , m_running(false)
    , m_sampleRate(48000)
    , m_bufferSize(AUDIO_FRAMESIZE)
    , m_deviceIndex(0)
    , m_digitalDeviceIndex(0)
    , m_isDigitalMode(false)
{
    CHECKED_CONNECT(set,
                    SIGNAL(micInputChanged(int)),
                    this,
                    SLOT(MicInputChanged(int)));

    CHECKED_CONNECT(set,
                    SIGNAL(digitalAudioInputChanged(int)),
                    this,
                    SLOT(DigitalAudioInputChanged(int)));

    connect(set, &Settings::dspModeChanged,
            this, &TransmitAudioInput::dspModeChanged);

    m_deviceIndex = set->getMicInputDev();
    m_digitalDeviceIndex = set->getDigitalAudioInputDev();

    // Resolve persisted source names against currently available devices.
    // If the saved device no longer exists, fall back to the Qt default device.
    const QList<QAudioDevice> devices = availableAudioInputDevices();
    const QAudioDevice defaultDevice = QMediaDevices::defaultAudioInput();
    const QString defaultName = defaultDevice.description();

    const QString savedMicName = set->getMicInputSourceName();
    if (savedMicName == "hpsdr-local") {
        m_deviceIndex = 0;
    } else {
        int micPos = findAudioInputByDescription(devices, savedMicName);
        if (micPos < 0 && !defaultName.isEmpty())
            micPos = findAudioInputByDescription(devices, defaultName);
        if (micPos >= 0)
            m_deviceIndex = micPos + 1;
    }

    const QString savedDigitalName = set->getDigitalInputSourceName();
    if (savedDigitalName == "none") {
        m_digitalDeviceIndex = 0;
    } else {
        int digPos = findAudioInputByDescription(devices, savedDigitalName);
        if (digPos < 0 && !defaultName.isEmpty())
            digPos = findAudioInputByDescription(devices, defaultName);
        if (digPos >= 0)
            m_digitalDeviceIndex = digPos + 1;
        else
            m_digitalDeviceIndex = 0;
    }
    
    audioinputBuffer.resize(AUDIO_FRAMESIZE);
    audioinputBuffer.fill(0.0);
    
    Setup();
    
    AUDIO_INPUT_DEBUG << "Audio Buffer Size: " << audioinputBuffer.size();
}

TransmitAudioInput::~TransmitAudioInput()
{
    Stop();
    if (m_audioSource) {
        delete m_audioSource;
        m_audioSource = nullptr;
    }
}

QList<QAudioDevice> TransmitAudioInput::availableAudioInputDevices()
{
    return QMediaDevices::audioInputs();
}

QList<QAudioDevice> TransmitAudioInput::getAudioInputDevices() const
{
    return availableAudioInputDevices();
}

void TransmitAudioInput::Setup() {
    m_mutex.lock();

    AUDIO_INPUT_DEBUG << "Setup: mode=" << (m_isDigitalMode ? "digital" : "analog")
                      << " micIndex=" << m_deviceIndex
                      << " digitalIndex=" << m_digitalDeviceIndex;
    
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
    QList<QAudioDevice> devices = getAudioInputDevices();

    if (m_isDigitalMode) {
        // m_digitalDeviceIndex: 0 = None (no audio), >0 = device (1-based offset)
        if (m_digitalDeviceIndex > 0) {
            int actualIndex = m_digitalDeviceIndex - 1;
            if (actualIndex >= 0 && actualIndex < devices.size()) {
                inputDevice = devices[actualIndex];
                AUDIO_INPUT_DEBUG << "Digital mode: using audio input:" << inputDevice.description();
            } else {
                inputDevice = QMediaDevices::defaultAudioInput();
                AUDIO_INPUT_DEBUG << "Digital mode: device index out of range, using default:" << inputDevice.description();
            }
        } else {
            AUDIO_INPUT_DEBUG << "Digital mode: no digital audio device configured";
            m_mutex.unlock();
            return; // "None" selected — no audio source needed
        }
    } else if (m_deviceIndex == 0) {
        // Index 0 is HPSDR/local mic path. Do not open a host Qt audio input.
        AUDIO_INPUT_DEBUG << "Current TX audio input device: HPSDR local mic (no host Qt input device)";
        m_mutex.unlock();
        return;
    } else {
        // Use specific device (adjust index since 0 is HPSDR)
        int actualIndex = m_deviceIndex - 1;
        if (actualIndex >= 0 && actualIndex < devices.size()) {
            inputDevice = devices[actualIndex];
            AUDIO_INPUT_DEBUG << "Using audio input device:" << inputDevice.description();
        } else {
            inputDevice = QMediaDevices::defaultAudioInput();
            AUDIO_INPUT_DEBUG << "Device index out of range, using default";
        }
    }

    AUDIO_INPUT_DEBUG << "Current TX audio input device:" << inputDevice.description();
    
    // Check if format is supported
    if (!inputDevice.isFormatSupported(m_format)) {
        AUDIO_INPUT_DEBUG << "Format not supported, using nearest format";
        m_format = inputDevice.preferredFormat();
    }

    // Create the audio source
    AUDIO_INPUT_DEBUG << "Opening audio input device:" << inputDevice.description();
    m_audioSource = new QAudioSource(inputDevice, m_format, this);
    m_audioSource->setBufferSize(4 * m_bufferSize); // Larger buffer to prevent underruns
    
    AUDIO_INPUT_DEBUG << "Audio input setup complete";
    
    m_mutex.unlock();
}

void TransmitAudioInput::MicInputChanged(int value) {
    /* Index 0 is HPSDR local mic input */
    AUDIO_INPUT_DEBUG << "Mic Input Changed to:" << value;

    m_deviceIndex = value;

    if (!m_isDigitalMode) {
        stopHardware();
        Setup();
        // Mirror original behaviour: always capture from an external device
        if (m_deviceIndex > 0)
            Start();
        else
            AUDIO_INPUT_DEBUG << "Local HPSDR Mic Mode selected";
    }
}

void TransmitAudioInput::DigitalAudioInputChanged(int index) {
    AUDIO_INPUT_DEBUG << "Digital audio input changed to:" << index;
    m_digitalDeviceIndex = index;

    if (m_isDigitalMode) {
        stopHardware();
        Setup();
        if (m_digitalDeviceIndex > 0)
            Start();
    }
}

void TransmitAudioInput::dspModeChanged(QObject *sender, int rx, DSPMode mode) {
    Q_UNUSED(sender)
    Q_UNUSED(rx)

    bool digital = (mode == DIGL || mode == DIGU);
    if (digital == m_isDigitalMode)
        return;

    m_isDigitalMode = digital;
    AUDIO_INPUT_DEBUG << "DSP mode changed; digital mode:" << m_isDigitalMode;

    stopHardware();
    Setup();
    // Start if the active mode has a valid device
    bool shouldStart = m_isDigitalMode ? (m_digitalDeviceIndex > 0) : (m_deviceIndex > 0);
    if (shouldStart)
        Start();
}

void TransmitAudioInput::stopHardware() {
    m_mutex.lock();
    if (m_running && m_audioSource) {
        if (m_audioInputDevice) {
            disconnect(m_audioInputDevice, &QIODevice::readyRead,
                      this, &TransmitAudioInput::handleReadyRead);
        }
        m_audioSource->stop();
        m_audioInputDevice = nullptr;
        m_running = false;
        AUDIO_INPUT_DEBUG << "Audio input hardware stopped (internal)";
    }
    m_mutex.unlock();
}

void TransmitAudioInput::Stop() {
    stopHardware();
    AUDIO_INPUT_DEBUG << "Audio input stopped";
}

bool TransmitAudioInput::Start() {
    if (m_running)
        return true;

    m_mutex.lock();
    if (m_audioSource) {
        // Start the audio input
        m_audioInputDevice = m_audioSource->start();
        if (m_audioInputDevice) {
            connect(m_audioInputDevice, &QIODevice::readyRead,
                    this, &TransmitAudioInput::handleReadyRead);
            m_running = true;
            AUDIO_INPUT_DEBUG << "Audio input started";
            
            // Check if there is already data available
            if (m_audioInputDevice->bytesAvailable() > 0) {
                handleReadyRead();
            }
        } else {
            AUDIO_INPUT_DEBUG << "Could not start audio input";
            m_mutex.unlock();
            return false;
        }
    }
    m_mutex.unlock();
    return true;
}

void TransmitAudioInput::handleReadyRead()
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

void TransmitAudioInput::processAudioData(const QByteArray &data)
{
    // Process the incoming audio data (assuming 16-bit signed PCM)
    const qint16 *ptr = reinterpret_cast<const qint16 *>(data.constData());
    int numSamples = data.size() / sizeof(qint16);
    
    for (int i = 0; i < numSamples; ++i) {
        m_residualBuffer.append(static_cast<float>(ptr[i]) / 32768.0f);
        
        if (m_residualBuffer.size() >= DSP_SAMPLE_SIZE) {
            AUDIOBUF chunk;
            chunk.resize(DSP_SAMPLE_SIZE);
            double sumSq = 0;
            for (int s = 0; s < DSP_SAMPLE_SIZE; s++) {
                float val = m_residualBuffer.at(s);
                chunk[s] = static_cast<double>(val);
                sumSq += (val * val);
            }
            m_residualBuffer.remove(0, DSP_SAMPLE_SIZE);
            
            static int blockCount = 0;
            if (++blockCount % 100 == 0) {
                double rms = sqrt(sumSq / DSP_SAMPLE_SIZE);
                float peak = 0.0f;
                for (int s = 0; s < DSP_SAMPLE_SIZE; ++s) {
                    float a = std::fabs(static_cast<float>(chunk[s]));
                    if (a > peak)
                        peak = a;
                }
                double rmsDb = 20.0 * std::log10(std::max(rms, 1.0e-9));
                double peakDb = 20.0 * std::log10(std::max(static_cast<double>(peak), 1.0e-9));
                double headroomDb = -peakDb;
                AUDIO_INPUT_DEBUG << "Mic input: " << DSP_SAMPLE_SIZE
                                  << " samples, RMS=" << rms
                                  << " (" << rmsDb << " dBFS), peak=" << peak
                                  << " (" << peakDb << " dBFS), headroom=" << headroomDb << " dB";
            }
            
            m_faudioInQueue.enqueue(chunk);
            emit tx_mic_data_ready();
        }
    }
}

