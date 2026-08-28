#include "receiveraudiooutput.h"
#include "Util/AudioDeviceService.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QThread>

ReceiverAudioOutput::ReceiverAudioOutput(QObject *parent)
    : QObject(parent)
{
    m_reopenTimer.setParent(this);
    m_reopenTimer.setSingleShot(true);
    m_reopenTimer.setInterval(250);
    connect(&m_reopenTimer, &QTimer::timeout, this, &ReceiverAudioOutput::reopenOutput);

    // HDMI sleep removes the sink on the GUI/media thread; reopen on this object's thread.
    connect(AudioDeviceService::instance(), &AudioDeviceService::audioOutputsChanged,
            this, &ReceiverAudioOutput::onAudioOutputsChanged, Qt::QueuedConnection);

    setSampleRate(m_sampleRate);
}

ReceiverAudioOutput::~ReceiverAudioOutput()
{
    stop();
}

bool ReceiverAudioOutput::onOwnThread() const
{
    return QThread::currentThread() == thread();
}

void ReceiverAudioOutput::scheduleReopen()
{
    if (!onOwnThread()) {
        // QTimer::start() is only legal on the timer's own thread, and the audio
        // thread must not create or destroy the sink.
        if (!m_reopenPending) {
            m_reopenPending = true;
            QMetaObject::invokeMethod(this, &ReceiverAudioOutput::reopenOutput, Qt::QueuedConnection);
        }
        return;
    }
    m_reopenPending = true;
    if (!m_reopenTimer.isActive())
        m_reopenTimer.start();
}

void ReceiverAudioOutput::setSampleRate(int rate)
{
    if (!onOwnThread()) {
        QMetaObject::invokeMethod(this, [this, rate] { setSampleRate(rate); }, Qt::QueuedConnection);
        return;
    }

    QMutexLocker locker(&m_mutex);
    m_sampleRate = rate;
    m_format.setSampleRate(m_sampleRate);
    m_format.setChannelCount(2); // Stereo
    m_format.setSampleFormat(QAudioFormat::Float); // Use Float for SDR output

    const bool restart = m_wantRunning;
    stopLocked();
    openSinkLocked();
    if (restart)
        startLocked();
}

void ReceiverAudioOutput::start()
{
    if (!onOwnThread()) {
        QMetaObject::invokeMethod(this, &ReceiverAudioOutput::start, Qt::QueuedConnection);
        return;
    }

    QMutexLocker locker(&m_mutex);
    m_wantRunning = true;
    if (!m_audioSink)
        openSinkLocked();
    startLocked();
}

void ReceiverAudioOutput::stop()
{
    if (!onOwnThread()) {
        // Sink/notifiers live on this object's thread; never stop/delete cross-thread
        // (Qt FFmpeg/Pulse idle callbacks then hit a null endpoint → segfault).
        if (thread() && thread()->isRunning()) {
            QMetaObject::invokeMethod(this, [this] { stop(); }, Qt::QueuedConnection);
            return;
        }
        // Owning thread already gone — best-effort local teardown.
    }

    QMutexLocker locker(&m_mutex);
    m_wantRunning = false;
    m_reopenPending = false;
    if (onOwnThread())
        m_reopenTimer.stop();
    stopLocked();
}

void ReceiverAudioOutput::writeAudio(const QVector<float>& audioBuffer)
{
    // QAudioSink / Pulse require I/O on the sink's thread. DSP runs elsewhere.
    if (!onOwnThread()) {
        const QVector<float> copy = audioBuffer;
        QMetaObject::invokeMethod(this, [this, copy] { writeAudio(copy); }, Qt::QueuedConnection);
        return;
    }

    QMutexLocker locker(&m_mutex);
    if (!m_wantRunning)
        return;
    if (!isSinkHealthy()) {
        // Drop samples while the sink is gone (e.g. HDMI display sleep).
        // Teardown/reopen must happen on the sink's own thread, so only flag it here.
        if (!m_audioSink
            || m_audioSink->error() != QAudio::NoError
            || m_audioSink->state() == QAudio::StoppedState) {
            markSinkLostLocked("writeAudio unhealthy sink");
            scheduleReopen();
        }
        return;
    }

    static QElapsedTimer overflowLogTimer;
    static qint64 aggregatedDroppedBytes = 0;
    if (!overflowLogTimer.isValid()) {
        overflowLogTimer.start();
    }

    // Append new samples to any unwritten bytes from the previous call
    const char* src = reinterpret_cast<const char*>(audioBuffer.constData());
    m_pending.append(src, audioBuffer.size() * (int)sizeof(float));

    const qint64 written = m_device->write(m_pending);
    if (written < 0 || m_audioSink->error() != QAudio::NoError) {
        markSinkLostLocked("write failed");
        scheduleReopen();
        return;
    }
    if (written > 0)
        m_pending.remove(0, (int)written);

    // Safety cap: if the sink is stalled, drop oldest data rather than
    // growing unbounded. Increase to ~500ms at 48kHz stereo float
    // to accommodate high-rate batching bursts.
    constexpr int MAX_PENDING = 500 * (48000 * 2 * sizeof(float)) / 1000;
    if (m_pending.size() > MAX_PENDING) {
        const int dropped = m_pending.size() - MAX_PENDING;
        m_pending.remove(0, dropped);
        aggregatedDroppedBytes += dropped;

        // During fast UI-driven retunes, the DSP can briefly outrun wall-clock
        // playback. Aggregate drops and emit at most once per second.
        if (overflowLogTimer.elapsed() >= 1000) {
            qWarning() << "Audio: pending overflow, dropped"
                       << aggregatedDroppedBytes << "bytes in last"
                       << overflowLogTimer.elapsed() << "ms";
            aggregatedDroppedBytes = 0;
            overflowLogTimer.restart();
        }
    }
}

void ReceiverAudioOutput::onAudioOutputsChanged()
{
    QMutexLocker locker(&m_mutex);
    if (!m_wantRunning)
        return;

    // Debounce HDMI flicker / multi-fire hotplug notifications.
    scheduleReopen();
}

void ReceiverAudioOutput::onSinkStateChanged(QAudio::State state)
{
    QMutexLocker locker(&m_mutex);
    if (!m_wantRunning || !m_audioSink)
        return;

    if (state == QAudio::StoppedState && m_audioSink->error() != QAudio::NoError) {
        handleDeviceLostLocked("sink state Stopped with error");
        scheduleReopen();
    }
}

void ReceiverAudioOutput::reopenOutput()
{
    if (!onOwnThread()) {
        QMetaObject::invokeMethod(this, &ReceiverAudioOutput::reopenOutput, Qt::QueuedConnection);
        return;
    }

    QMutexLocker locker(&m_mutex);
    if (!m_wantRunning) {
        m_reopenPending = false;
        return;
    }
    m_reopenPending = false;

    qInfo() << "Audio: reopening output after device change; default ="
            << AudioDeviceService::instance()->defaultOutput().description();

    stopLocked();
    openSinkLocked();
    startLocked();
}

bool ReceiverAudioOutput::isSinkHealthy() const
{
    if (!m_audioSink || !m_device)
        return false;
    if (m_audioSink->error() != QAudio::NoError)
        return false;
    const QAudio::State st = m_audioSink->state();
    return st == QAudio::ActiveState || st == QAudio::IdleState;
}

void ReceiverAudioOutput::markSinkLostLocked(const char *reason)
{
    if (!m_device && m_reopenPending)
        return;
    qWarning() << "Audio: output device lost (" << reason << ") — muting until reopen";
    m_pending.clear();
    m_device = nullptr;
}

void ReceiverAudioOutput::handleDeviceLostLocked(const char *reason)
{
    qWarning() << "Audio: output device lost (" << reason << ") — muting until reopen";
    m_pending.clear();
    m_device = nullptr;
    if (m_audioSink) {
        // Disconnect before stop/delete so we don't recurse through stateChanged.
        disconnect(m_audioSink, nullptr, this, nullptr);
        m_audioSink->stop();
        m_audioSink->setParent(nullptr);
        m_audioSink->deleteLater();
        m_audioSink = nullptr;
    }
}

void ReceiverAudioOutput::openSinkLocked()
{
    QAudioDevice outputDevice = AudioDeviceService::instance()->defaultOutput();
    if (outputDevice.isNull()) {
        qWarning() << "Audio: no default output device available";
        return;
    }

    QAudioFormat format = m_format;
    if (!outputDevice.isFormatSupported(format)) {
        qWarning() << "Audio: preferred float format unsupported on"
                    << outputDevice.description() << "— using device preferred format";
        format = outputDevice.preferredFormat();
        // Keep our configured rate/channels when the preferred format allows.
        if (format.sampleRate() <= 0)
            format.setSampleRate(m_sampleRate);
        if (format.channelCount() <= 0)
            format.setChannelCount(2);
        m_format = format;
    }

    m_audioSink = new QAudioSink(outputDevice, m_format, this);
    // Queued: start()/stop() may emit while we hold m_mutex.
    connect(m_audioSink, &QAudioSink::stateChanged,
            this, &ReceiverAudioOutput::onSinkStateChanged, Qt::QueuedConnection);
}

void ReceiverAudioOutput::startLocked()
{
    if (!m_audioSink)
        openSinkLocked();
    if (!m_audioSink)
        return;

    // Pre-allocate a larger internal buffer (8 DSP buffers = ~170ms at 48kHz)
    m_audioSink->setBufferSize(8 * 8192);
    m_device = m_audioSink->start();
    m_pending.clear();

    if (!m_device || m_audioSink->error() != QAudio::NoError) {
        qWarning() << "Audio: failed to start sink on"
                    << AudioDeviceService::instance()->defaultOutput().description()
                    << "error" << m_audioSink->error();
        handleDeviceLostLocked("start failed");
    }
}

void ReceiverAudioOutput::stopLocked()
{
    m_pending.clear();
    m_device = nullptr;
    if (m_audioSink) {
        disconnect(m_audioSink, nullptr, this, nullptr);
        m_audioSink->stop();
        // Unparent + deleteLater: sync delete races platform stream idle callbacks
        // (QPlatformAudioEndpointBase::updateStreamIdle with this=nullptr).
        m_audioSink->setParent(nullptr);
        m_audioSink->deleteLater();
        m_audioSink = nullptr;
    }
}
