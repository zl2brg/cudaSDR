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
    void encodeCCBytes(unsigned char* buffer, THPSDRParameter* io, int& sendState) override;

    QByteArray formatStartStop(char value) override;
    QByteArray formatInitFrame(int rx, THPSDRParameter* io) override;
    QByteArray formatOutputPacket(const QByteArray& audioData, uint32_t& sequence) override;

    int getPayloadSize() override { return 1024; }
    int getHeaderSize() override { return 4; } // Protocol 2 sequence is 4 bytes
    QList<quint16> getRequiredPorts() override { 
        return { 1024, 1025, 1026, 1027, 1028, 1029, 1035 }; 
    }

private:
    uint32_t m_lastSequence;
    int m_rxSamples;
};

#endif // CPROTOCOL2_H
