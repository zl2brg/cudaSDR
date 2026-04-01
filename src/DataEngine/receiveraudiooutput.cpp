#include "receiveraudiooutput.h"
#include <QDebug>

ReceiverAudioOutput::ReceiverAudioOutput(QObject *parent)
    : QObject(parent)
{
    setSampleRate(m_sampleRate);
}

ReceiverAudioOutput::~ReceiverAudioOutput()
{
    stop();
}

void ReceiverAudioOutput::setSampleRate(int rate)
{
    m_sampleRate = rate;
    m_format.setSampleRate(m_sampleRate);
    m_format.setChannelCount(2); // Stereo
    m_format.setSampleFormat(QAudioFormat::Float); // Use Float for SDR output

    QAudioDevice outputDevice = QMediaDevices::defaultAudioOutput();
    if (!outputDevice.isFormatSupported(m_format)) {
        qWarning() << "Default format not supported, trying preferred format";
        m_format = outputDevice.preferredFormat();
    }
    if (m_audioSink) {
        stop();
    }
    m_audioSink = new QAudioSink(outputDevice, m_format, this);
}

void ReceiverAudioOutput::start()
{
    if (!m_audioSink) setSampleRate(m_sampleRate);
    // Pre-allocate a larger internal buffer (8 DSP buffers = ~170ms at 48kHz)
    // to reduce partial-write frequency before the first call
    m_audioSink->setBufferSize(8 * 8192);
    m_device = m_audioSink->start();
    m_pending.clear();
}

void ReceiverAudioOutput::stop()
{
    if (m_audioSink) {
        m_audioSink->stop();
        delete m_audioSink;
        m_audioSink = nullptr;
        m_device = nullptr;
    }
}

void ReceiverAudioOutput::writeAudio(const QVector<float>& audioBuffer)
{
    QMutexLocker locker(&m_mutex);
    if (!m_device) return;

    // Append new samples to any unwritten bytes from the previous call
    const char* src = reinterpret_cast<const char*>(audioBuffer.constData());
    m_pending.append(src, audioBuffer.size() * (int)sizeof(float));

    qint64 written = m_device->write(m_pending);
    if (written > 0)
        m_pending.remove(0, (int)written);

    // Safety cap: if the sink is stalled, drop oldest data rather than
    // growing unbounded (4 DSP buffers ≈ 85ms)
    constexpr int MAX_PENDING = 4 * 8192;
    if (m_pending.size() > MAX_PENDING) {
        qWarning() << "Audio: pending overflow, dropping"
                   << (m_pending.size() - MAX_PENDING) << "bytes";
        m_pending.remove(0, m_pending.size() - MAX_PENDING);
    }
}
