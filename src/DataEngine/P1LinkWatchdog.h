/**
 * @file  P1LinkWatchdog.h
 * @brief Protocol 1 EP6 stall detection and start-command retry policy.
 *
 * Protocol 1 host packets are paced by received EP6 IQ, so if EP6 stops the
 * host stops sending too. A radio whose watchdog then drops the stream never
 * comes back on its own; this decides when DataIO re-sends the start command.
 */

#ifndef P1_LINK_WATCHDOG_H
#define P1_LINK_WATCHDOG_H

#include <QtGlobal>

class P1LinkWatchdog {
public:
    struct PacketEvent {
        bool   first = false;
        bool   resumed = false;
        qint64 gapMs = 0;
        int    restarts = 0;
    };

    explicit P1LinkWatchdog(qint64 stallMs = 300, qint64 retryMs = 1000)
        : m_stallMs(stallMs), m_retryMs(retryMs) {}

    /** Call when the start command has just been sent. */
    void reset(qint64 nowMs) {
        m_lastPacketMs = nowMs;
        m_lastRestartMs = nowMs;
        m_streaming = false;
        m_stalled = false;
        m_restarts = 0;
    }

    PacketEvent onIqPacket(qint64 nowMs) {
        PacketEvent ev;
        ev.gapMs = nowMs - m_lastPacketMs;
        ev.first = !m_streaming;
        ev.resumed = m_streaming && m_stalled;
        ev.restarts = m_restarts;
        m_streaming = true;
        m_stalled = false;
        m_restarts = 0;
        m_lastPacketMs = nowMs;
        return ev;
    }

    bool isSilent(qint64 nowMs) const { return nowMs - m_lastPacketMs >= m_stallMs; }

    /** True when the caller should re-send the start command now. */
    bool shouldResendStart(qint64 nowMs) {
        if (!isSilent(nowMs))
            return false;
        m_stalled = true;
        if (nowMs - m_lastRestartMs < m_retryMs)
            return false;
        m_lastRestartMs = nowMs;
        ++m_restarts;
        return true;
    }

    bool   streaming() const { return m_streaming; }
    bool   stalled() const { return m_stalled; }
    int    restarts() const { return m_restarts; }
    qint64 silenceMs(qint64 nowMs) const { return nowMs - m_lastPacketMs; }

private:
    qint64 m_stallMs;
    qint64 m_retryMs;
    qint64 m_lastPacketMs = 0;
    qint64 m_lastRestartMs = 0;
    bool   m_streaming = false;
    bool   m_stalled = false;
    int    m_restarts = 0;
};

#endif // P1_LINK_WATCHDOG_H
