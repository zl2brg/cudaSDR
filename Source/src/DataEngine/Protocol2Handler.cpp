/**
 * @file  Protocol2Handler.cpp
 * @brief HPSDR Protocol 2 hardware transport — stub implementation.
 *
 * TODO: Implement Protocol 2 TCP/UDP control and EP6 IQ streaming.
 */

#include "Protocol2Handler.h"
#include <QDebug>

Protocol2Handler::Protocol2Handler(THPSDRParameter* ioData, QObject* parent)
    : IHardwareIO(parent)
    , set(Settings::instance())
    , io(ioData)
    , m_stopped(false)
{
    connect(set, &Settings::sampleRateChanged,
            this, &Protocol2Handler::setSampleRate);
}

Protocol2Handler::~Protocol2Handler()
{
    if (!m_stopped) stop();
}

void Protocol2Handler::initIO()
{
    // TODO: open TCP control socket to device (port 1024)
    // TODO: bind UDP EP6 receive socket
    // TODO: perform Protocol 2 handshake / board discovery
    qDebug() << "Protocol2Handler::initIO() — stub, not yet implemented";
    emit messageEvent("Protocol 2: initIO() stub — not implemented");
}

void Protocol2Handler::stop()
{
    m_stopped = true;
    // TODO: send Protocol 2 stop command
    // TODO: close sockets
    qDebug() << "Protocol2Handler::stop() — stub";
}

void Protocol2Handler::writeData()
{
    // TODO: send EP2 TX IQ + audio frame
}

void Protocol2Handler::sendAudio(unsigned char* buf)
{
    Q_UNUSED(buf)
    // TODO: forward audio channel data via Protocol 2 audio stream
}

void Protocol2Handler::sendInitFrames(int rx)
{
    Q_UNUSED(rx)
    // TODO: send Protocol 2 receiver frequency / config commands
}

void Protocol2Handler::sendCommand(char cmd)
{
    Q_UNUSED(cmd)
    // TODO: map generic start/stop cmd to Protocol 2 TCP control messages
}

void Protocol2Handler::set_wbBuffers(int val)
{
    Q_UNUSED(val)
    // TODO: configure wideband capture buffer count via Protocol 2
}

void Protocol2Handler::setSampleRate(QObject* sender, int value)
{
    Q_UNUSED(sender)
    Q_UNUSED(value)
    // TODO: update socket buffer and send rate-change command via Protocol 2
}
