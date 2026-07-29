#ifndef IHPSDRPROTOCOL_H
#define IHPSDRPROTOCOL_H

#include <QObject>
#include <QByteArray>
#include <QHostAddress>

class DataEngine;
class RadioModel;

class IHPSDRProtocol {
public:
    virtual ~IHPSDRProtocol() {}

    // Packet validation and identification
    virtual bool isPacketValid(const unsigned char* data, int len) = 0;
    virtual uint32_t getSequence(const unsigned char* data) = 0;
    virtual int getPacketType(const unsigned char* data) = 0;

    // Data processing
    virtual void processInputBuffer(const QByteArray& buffer, DataEngine* de, quint16 sourcePort) = 0;
    virtual void decodeCCBytes(const QByteArray& buffer, DataEngine* de) = 0;
    virtual void encodeCCBytes(unsigned char* buffer, DataEngine* de, RadioModel* radioModel, int& sendState, quint16& port) = 0;
    virtual void encodeCCBytes(unsigned char* buffer, DataEngine* de, int& sendState, quint16& port) {
        encodeCCBytes(buffer, de, nullptr, sendState, port);
    }

    // Command formatting
    virtual QByteArray formatStartStop(char value, quint16& port) = 0;
    virtual QByteArray formatInitFrame(int rx, DataEngine* de, RadioModel* radioModel, quint16& port) = 0;
    virtual QByteArray formatInitFrame(int rx, DataEngine* de, quint16& port) {
        return formatInitFrame(rx, de, nullptr, port);
    }
    virtual QByteArray formatOutputPacket(const QByteArray& audioData, uint32_t& sequence) = 0;

    // Hardware specific
    virtual int getPayloadSize() = 0;
    virtual int getHeaderSize() = 0;
    virtual QList<quint16> getRequiredPorts() = 0;
};

#endif // IHPSDRPROTOCOL_H
