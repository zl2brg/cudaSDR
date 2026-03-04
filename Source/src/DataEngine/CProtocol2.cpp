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

void CProtocol2::encodeCCBytes(unsigned char* buffer, THPSDRParameter* io, int& sendState, quint16& port) {
    Settings* set = Settings::instance();
    io->mutex.lock();
    memset(buffer, 0, 64);
    
    // Protocol 2 commands are 60 bytes.
    // 0-3: Sequence number
    // 4: Packet type or first data byte
    
    switch (sendState) {
        case 0: // General Packet (Port 1024)
            port = 1024;
            {
                uint32_t seq = qToBigEndian(m_sequences[1024]++);
                memcpy(buffer, &seq, 4);
                buffer[4] = 0x00; // Command - General Packet to SDR
                
                // Set Ports (Big Endian)
                quint16 ddcSpecPort = qToBigEndian((quint16)1025);
                memcpy(&buffer[5], &ddcSpecPort, 2);
                
                quint16 txSpecPort = qToBigEndian((quint16)1026);
                memcpy(&buffer[7], &txSpecPort, 2);
                
                quint16 hpPCPort = qToBigEndian((quint16)1027);
                memcpy(&buffer[9], &hpPCPort, 2);
                
                quint16 hpRadioPort = qToBigEndian((quint16)1027);
                memcpy(&buffer[11], &hpRadioPort, 2);
                
                quint16 ddcAudioPort = qToBigEndian((quint16)1028);
                memcpy(&buffer[13], &ddcAudioPort, 2);
                
                quint16 ducIQPort = qToBigEndian((quint16)1029);
                memcpy(&buffer[15], &ducIQPort, 2);
                
                quint16 ddc0Port = qToBigEndian((quint16)1035);
                memcpy(&buffer[17], &ddc0Port, 2);
                
                // Hardware counts
                buffer[19] = (unsigned char)io->receivers; 
                buffer[20] = 1; // Num ADCs (assume 1 for now)
                buffer[21] = 1; // Num DUCs
                buffer[22] = 1; // Num DACs
                
                // Other global settings (Byte 37+)
                buffer[37] = 0x00; // Time stamping off, VNA off, etc.
                buffer[38] = 0x00; // Hardware reset off
                buffer[39] = 0x00; // Big Endian data format (Bit 0=0)
            }
            sendState = 1;
            break;
            
        case 1: // DDC Specific Packet (Port 1025)
            port = 1025;
            {
                uint32_t seq = qToBigEndian(m_sequences[1025]++);
                memcpy(buffer, &seq, 4);
                
                // For now, configure DDC 0
                buffer[4] = 1; // Number of ADCs
                buffer[5] = 0x00; // Bit 0: Dither ADC0, Bit 1: Random ADC0
                
                // DDC 0 configuration (Starts at Byte 16)
                buffer[16] = 0x00; // ADC selection for DDC 0 (ADC 0)
                
                // Sampling Rate (Bytes 18-19)
                uint16_t rate = 0;
                switch (io->samplerate) {
                    case 48000: rate = 48; break;
                    case 96000: rate = 96; break;
                    case 192000: rate = 192; break;
                    case 384000: rate = 384; break;
                    default: rate = 48; break;
                }
                uint16_t rateBE = qToBigEndian(rate);
                memcpy(&buffer[18], &rateBE, 2);
                
                buffer[22] = 24; // Sample Size (24 bits)
            }
            sendState = 2;
            break;
            
        case 2: // Transmitter Specific Packet (Port 1026)
            port = 1026;
            {
                uint32_t seq = qToBigEndian(m_sequences[1026]++);
                memcpy(buffer, &seq, 4);
                buffer[4] = 1; // Number of DACs
                
                // DUC 0 settings
                buffer[5] = 0x00; 
                if (set->isInternalCw()) buffer[5] |= 0x02; // CW bit
                if (set->getCwKeyerMode() > 0) buffer[5] |= 0x08; // Iambic bit (rough mapping)
                
                buffer[6] = (unsigned char)set->getCwSidetoneVolume();
                
                uint16_t sideToneFreq = qToBigEndian((uint16_t)set->getCwSidetoneFreq());
                memcpy(&buffer[7], &sideToneFreq, 2);
                
                buffer[9] = (unsigned char)set->getCwKeyerSpeed();
                buffer[10] = (unsigned char)set->getCwKeyerWeight();
                
                uint16_t hangDelay = qToBigEndian((uint16_t)set->getCwHangTime());
                memcpy(&buffer[11], &hangDelay, 2);
            }
            sendState = 3;
            break;
            
        case 3: // High Priority Data Packet (Port 1027)
        default:
            port = 1027;
            {
                uint32_t seq = qToBigEndian(m_sequences[1027]++);
                memcpy(buffer, &seq, 4);
                buffer[4] = 0x01; // Run bit
                if (io->ccTx.mox || io->ccTx.ptt) {
                    buffer[4] |= 0x02; // PTT0
                }
                
                // Frequency for DDC 0 (Byte 9-12)
                uint32_t freq = qToBigEndian((uint32_t)set->getCtrFrequencies().at(0));
                memcpy(&buffer[9], &freq, 4);
            }
            sendState = 0;
            break;
    }
    
    io->mutex.unlock();
}

QByteArray CProtocol2::formatStartStop(char value, quint16& port) {
    // Protocol 2 High Priority Data Packet (Port 1027)
    port = 1027;
    // 0-3: Sequence (0)
    // 4: Bits - run, PTT(n) -> Bit 0 is run
    QByteArray commandDatagram;
    commandDatagram.resize(60); 
    commandDatagram.fill(0);
    commandDatagram[4] = value ? 0x01 : 0x00;
    return commandDatagram;
}

QByteArray CProtocol2::formatInitFrame(int rx, THPSDRParameter* io, quint16& port) {
    // Protocol 2 uses multiple specific packets.
    // For now, return a generic initialization or use "General Packet".
    port = 1024;
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
