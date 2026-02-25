/**
 * @file  Protocol2Handler.h
 * @brief HPSDR Protocol 2 hardware transport stub.
 *
 * Protocol 2 uses TCP for control and UDP EP6 streaming for IQ data,
 * replacing the UDP-only Protocol 1 (Metis/Hermes) wire format.
 *
 * This is a stub ready for implementation.  When complete it should:
 *  - Establish a TCP control connection to the board (port 1024).
 *  - Send HP/LP filter configuration via TCP commands.
 *  - Receive IQ data as EP6 UDP packets and place decoded frames on
 *    de->io.iq_queue (same contract as DataIO), emitting readydata().
 *  - Implement sendInitFrames() / sendCommand() using Protocol 2 commands.
 *
 * Reference: openHPSDR Ethernet Protocol v2.x specification.
 */

#ifndef PROTOCOL2HANDLER_H
#define PROTOCOL2HANDLER_H

#include "IHardwareIO.h"
#include "cusdr_settings.h"

class Protocol2Handler : public IHardwareIO {
    Q_OBJECT

public:
    explicit Protocol2Handler(THPSDRParameter* ioData, QObject* parent = nullptr);
    ~Protocol2Handler() override;

public slots:
    // IHardwareIO interface
    void initIO()                      override;
    void stop()                        override;
    void writeData()                   override;
    void sendAudio(unsigned char* buf) override;
    void sendInitFrames(int rx)        override;
    void sendCommand(char cmd)         override;
    void set_wbBuffers(int val)        override;

private:
    Settings*        set;
    THPSDRParameter* io;
    volatile bool    m_stopped;

    // TODO: TCP control socket
    // TODO: UDP EP6 receive socket
    // TODO: P2 sequence tracking

private slots:
    void setSampleRate(QObject* sender, int value);
};

#endif // PROTOCOL2HANDLER_H
