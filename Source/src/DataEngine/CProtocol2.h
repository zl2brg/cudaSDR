#ifndef CPROTOCOL2_H
#define CPROTOCOL2_H

#include "IHPSDRProtocol.h"
#include <QtEndian>

class CProtocol2 : public IHPSDRProtocol {
public:
    CProtocol2();
    ~CProtocol2() override;

    bool isPacketValid(const unsigned char* data, int len) override;
    uint32_t getSequence(const unsigned char* data) override;
    int getPacketType(const unsigned char* data) override;

    void processInputBuffer(const QByteArray& buffer, DataEngine* de) override;
    void decodeCCBytes(const QByteArray& buffer, THPSDRParameter* io) override;
    void encodeCCBytes(unsigned char* buffer, THPSDRParameter* io, int& sendState, quint16& port) override;

    QByteArray formatStartStop(char value, quint16& port) override;
    QByteArray formatInitFrame(int rx, THPSDRParameter* io, quint16& port) override;
    QByteArray formatOutputPacket(const QByteArray& audioData, uint32_t& sequence) override;

    int getPayloadSize() override { return 1428; } // 1444-byte DDC packet minus 16-byte header
    // Protocol 2 DDC data packet header (per spec v4.3 p.51):
    //   Bytes  0- 3: Sequence number       (4 bytes)
    //   Bytes  4-11: VITA-49 Time Stamp     (8 bytes)
    //   Bytes 12-13: Bits per sample        (2 bytes)
    //   Bytes 14-15: I&Q Samples per frame  (2 bytes)
    //   Byte  16+  : I&Q sample data
    // Total header = 16 bytes; IQ data = 238 × 6 = 1428 bytes.
    int getHeaderSize() override { return 16; }
    // TODO(P2-MULTI-RX): getRequiredPorts() returns a fixed list with only
    // port 1035 (DDC0 source port).  When a device reports num_DDCs > 1 in
    // its discovery reply, additional DDC ports must be added here:
    //   DDC0 → port 1035, DDC1 → 1036, ..., DDC(N-1) → 1034+N
    // This requires the DDC count from discovery to be stored in the protocol
    // object (or passed from DataEngine after parsing the discovery reply) so
    // that DataIO can open the correct set of receive sockets.
    QList<quint16> getRequiredPorts() override { 
        return { 1024, 1025, 1026, 1027, 1028, 1029, 1035 }; 
    }

private:
    uint32_t m_lastSequence;
    int m_rxSamples;
    QMap<quint16, uint32_t> m_sequences;
    // Stored by isPacketValid() and read by getPacketType() to discriminate
    // between DDC-data packets (large) and High-Priority-Status packets (small).
    mutable int m_lastPacketLen;
};

#endif // CPROTOCOL2_H
