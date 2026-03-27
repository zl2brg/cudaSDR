#include "receiveraudiooutput.h"
#include <QDebug>
#include <QtEndian>

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
    m_format.setChannelCount(2); // Mono
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
    m_device = m_audioSink->start();
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

    // Convert QVector<float> to QByteArray for QIODevice
    QByteArray pcm;
    pcm.resize(audioBuffer.size() * sizeof(float));
    memcpy(pcm.data(), audioBuffer.data(), pcm.size());

    qint64 written = m_device->write(pcm);
    if (written != pcm.size()) {
        qWarning() << "Audio output underrun: tried to write" << pcm.size() << "bytes, wrote" << written;
    }
}
