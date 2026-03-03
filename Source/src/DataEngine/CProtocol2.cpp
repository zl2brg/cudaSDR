#include "CProtocol2.h"
#include "cusdr_dataEngine.h"

CProtocol2::CProtocol2() : m_lastSequence(0) {}

CProtocol2::~CProtocol2() {}

bool CProtocol2::isPacketValid(const unsigned char* data, int len) {
    // Protocol 2 packets start with a 4-byte sequence number.
    // Validation is harder without a fixed signature, but we can check lengths.
    // Often DDC packets are ~1444 bytes, but let's assume we use standard lengths.
    Q_UNUSED(data)
    return (len > 4); 
}

uint32_t CProtocol2::getSequence(const unsigned char* data) {
    uint32_t seq = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
    return seq;
}

int CProtocol2::getPacketType(const unsigned char* data) {
    // In Protocol 2, packet type depends on the port.
    // If we receive on port 1024, it's a Command Reply.
    // If we receive on port 1035, it's DDC data.
    // Since this method doesn't know the port, it's tricky.
    // Let's assume we use some heuristic or return a generic "P2" type.
    return 0x02; 
}

void CProtocol2::processInputBuffer(const QByteArray& buffer, DataEngine* de) {
    // Protocol 2 DDC Packet (Port 1035+)
    // 0-3: Sequence
    // 4-11: Time stamp
    // 12-1443: I&Q samples (238 or more 24-bit samples)
    
    int s = 12; // Start of I&Q data
    int samplesInPacket = (buffer.size() - 12 - 8) / 6; // Rough estimate if 24-bit
    
    for (int i = 0; i < samplesInPacket && s + 6 <= buffer.size(); i++) {
        int iSample = (int)((signed char)buffer.at(s++)) << 16;
        iSample |= (int)((unsigned char)buffer.at(s++)) << 8;
        iSample |= (int)((unsigned char)buffer.at(s++));
        
        int qSample = (int)((signed char)buffer.at(s++)) << 16;
        qSample |= (int)((unsigned char)buffer.at(s++)) << 8;
        qSample |= (int)((unsigned char)buffer.at(s++));
        
        double lsample = (double)iSample / 8388607.0;
        double rsample = (double)qSample / 8388607.0;
        
        // Feed to receiver 0 for now
        if (de->io.receivers > 0 && de->RX.at(0)->qtwdsp) {
            de->RX[0]->inBuf[m_rxSamples].re = lsample;
            de->RX[0]->inBuf[m_rxSamples].im = rsample;
        }
        
        m_rxSamples++;
        if (m_rxSamples == BUFFER_SIZE) {
            for (int r = 0; r < de->io.receivers; r++) {
                if (de->RX.at(r)->qtwdsp) {
                    QMetaObject::invokeMethod(de->RX.at(r), "dspProcessing", Qt::DirectConnection);
                }
            }
            m_rxSamples = 0;
        }
    }
}

void CProtocol2::decodeCCBytes(const QByteArray& buffer, THPSDRParameter* io) {
    // Protocol 2 High Priority Status Packet (Port 1025)
    // 0-3: Sequence
    // 4: Bits - PTT, Dot, Dash...
    if (buffer.size() < 5) return;
    
    io->ccRx.previous_dash = io->ccRx.dash;
    io->ccRx.previous_dot = io->ccRx.dot;
    
    io->ccRx.ptt = (buffer.at(4) & 0x01);
    io->ccRx.dot = (buffer.at(4) & 0x02);
    io->ccRx.dash = (buffer.at(4) & 0x04);
}

void CProtocol2::encodeCCBytes(unsigned char* buffer, THPSDRParameter* io, int& sendState) {
    // Protocol 2 High Priority Data Packet (Port 1027)
    // 0-3: Sequence (0)
    // 4: Bits - run, PTT(n)
    // Note: This 'buffer' was originally for the 1032-byte Protocol 1 packet.
    // For Protocol 2, we should probably send multiple packet types.
    // For now, let's just fill it with basic PTT state.
    
    io->mutex.lock();
    memset(buffer, 0, 64); // Protocol 2 commands are typically 60-64 bytes
    
    buffer[4] = 0x01; // Run bit
    if (io->ccTx.mox || io->ccTx.ptt) {
        buffer[4] |= 0x02; // PTT0
    }
    
    // sendState logic for other packet types could go here (DDC Specific etc.)
    sendState = (sendState + 1) % 8; 
    
    io->mutex.unlock();
}

QByteArray CProtocol2::formatStartStop(char value) {
    // Protocol 2 High Priority Data Packet (Port 1027)
    // 0-3: Sequence (0)
    // 4: Bits - run, PTT(n) -> Bit 0 is run
    QByteArray commandDatagram;
    commandDatagram.resize(60); 
    commandDatagram.fill(0);
    commandDatagram[4] = value ? 0x01 : 0x00;
    return commandDatagram;
}

QByteArray CProtocol2::formatInitFrame(int rx, THPSDRParameter* io) {
    // Protocol 2 uses multiple specific packets.
    // For now, return a generic initialization or use "General Packet".
    QByteArray initDatagram;
    initDatagram.resize(60);
    initDatagram.fill(0);
    // Byte 4 is 0x00 for General Packet to SDR
    return initDatagram;
}

QByteArray CProtocol2::formatOutputPacket(const QByteArray& audioData, uint32_t& sequence) {
    QByteArray outDatagram;
    uint32_t outseq = qFromBigEndian(sequence);
    outDatagram.resize(0);
    QByteArray seq(reinterpret_cast<const char*>(&outseq), sizeof(outseq));
    outDatagram += seq;
    outDatagram += audioData;
    sequence++;
    return outDatagram;
}
