#include "audiooutputmanager.h"
#include <QMediaDevices>
#include <QDebug>
#include <cstring>
#include <algorithm>

AudioOutputManager::AudioOutputManager(QObject* parent)
    : QObject(parent)
{
    refreshDeviceList();
}

QVector<QAudioDevice> AudioOutputManager::availableDevices() const
{
    return QMediaDevices::audioOutputs();
}

void AudioOutputManager::refreshDeviceList()
{
    emit deviceListChanged();
}

void AudioOutputManager::assignReceiverToDevice(const QString& receiverId, const QAudioDevice& device)
{
    QMutexLocker locker(&m_mutex);

    if (m_receivers.contains(receiverId)) {
        auto& ra = m_receivers[receiverId];
        if (ra.sink) {
            ra.sink->stop();
            delete ra.sink;
            ra.sink = nullptr;
            ra.io = nullptr;
        }
    }

    QAudioFormat fmt;
    if (m_receivers.contains(receiverId) && m_receivers[receiverId].format.isValid())
        fmt = m_receivers[receiverId].format;
    else
        fmt = device.preferredFormat();

    auto* sink = new QAudioSink(device, fmt, this);
    QIODevice* io = sink->start();

    m_receivers[receiverId] = { device, fmt, sink, io, nullptr, QByteArray() };
}

void AudioOutputManager::removeReceiver(const QString& receiverId)
{
    QMutexLocker locker(&m_mutex);
    if (m_receivers.contains(receiverId)) {
        auto& ra = m_receivers[receiverId];
        if (ra.sink) {
            ra.sink->stop();
            delete ra.sink;
        }
        m_receivers.remove(receiverId);
    }
}

void AudioOutputManager::setReceiverFormat(const QString& receiverId, const QAudioFormat& format)
{
    QMutexLocker locker(&m_mutex);
    if (!m_receivers.contains(receiverId))
        return;
    m_receivers[receiverId].format = format;
    assignReceiverToDevice(receiverId, m_receivers[receiverId].device);
}

void AudioOutputManager::setReceiverDspCallback(const QString& receiverId, DspCallback cb)
{
    QMutexLocker locker(&m_mutex);
    if (m_receivers.contains(receiverId))
        m_receivers[receiverId].dsp = cb;
}

void AudioOutputManager::setMixDspCallback(DspCallback cb)
{
    QMutexLocker locker(&m_mutex);
    m_mixDsp = cb;
}

void AudioOutputManager::writeAudio(const QString& receiverId, const float* interleavedLR, int frames)
{
    QMutexLocker locker(&m_mutex);

    if (m_mixAll && m_mixSink && m_mixIO) {
        // Collect per-receiver samples for mixing
        // For simplicity, we'll just sum all writeAudio() calls this period (could be more sophisticated in practice)
        m_mixBuf.resize(frames * 2 * sizeof(float));
        float* mix = reinterpret_cast<float*>(m_mixBuf.data());
        std::memcpy(mix, interleavedLR, frames * 2 * sizeof(float));
        // Apply per-receiver DSP if set
        if (m_receivers.contains(receiverId) && m_receivers[receiverId].dsp)
            m_receivers[receiverId].dsp(mix, frames);
        // Could sum other receivers here if needed
        // Apply global DSP
        if (m_mixDsp)
            m_mixDsp(mix, frames);
        // Convert to int16
        QByteArray pcm(frames * 2 * sizeof(qint16), 0);
        floatToInt16(mix, reinterpret_cast<qint16*>(pcm.data()), frames);
        m_mixIO->write(pcm);
        return;
    }

    // Not mixing: Per-receiver output
    if (!m_receivers.contains(receiverId))
        return;

    auto& ra = m_receivers[receiverId];
    float* data = const_cast<float*>(interleavedLR);
    // DSP
    if (ra.dsp)
        ra.dsp(data, frames);
    // Convert to int16 PCM
    ra.buf.resize(frames * 2 * sizeof(qint16));
    floatToInt16(data, reinterpret_cast<qint16*>(ra.buf.data()), frames);
    if (ra.io)
        ra.io->write(ra.buf);
}

void AudioOutputManager::setMixAllToOneDevice(bool enabled, const QAudioDevice& device)
{
    QMutexLocker locker(&m_mutex);
    m_mixAll = enabled;

    if (m_mixSink) {
        m_mixSink->stop();
        delete m_mixSink;
        m_mixSink = nullptr;
        m_mixIO = nullptr;
    }
    if (enabled) {
        m_mixDevice = device;
        QAudioFormat fmt = device.preferredFormat();
        m_mixSink = new QAudioSink(device, fmt, this);
        m_mixIO = m_mixSink->start();
        m_mixBuf.clear();
    }
}

void AudioOutputManager::closeAllSinks()
{
    QMutexLocker locker(&m_mutex);
    for (auto& ra : m_receivers) {
        if (ra.sink) {
            ra.sink->stop();
            delete ra.sink;
            ra.sink = nullptr;
            ra.io = nullptr;
        }
    }
    m_receivers.clear();
    if (m_mixSink) {
        m_mixSink->stop();
        delete m_mixSink;
        m_mixSink = nullptr;
        m_mixIO = nullptr;
    }
}

void AudioOutputManager::floatToInt16(const float* in, qint16* out, int frames)
{
    for (int i = 0; i < frames * 2; ++i) {
        float v = std::clamp(in[i], -1.0f, 1.0f);
        out[i] = static_cast<qint16>(v * 32767.0f);
    }
}
