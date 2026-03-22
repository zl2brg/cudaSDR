#include "CProtocol2.h"
#include "cusdr_dataEngine.h"
#include "cusdr_settings.h"

namespace {
// openHPSDR protocol v4.3 allows frequency or phase words; current FPGA builds
// generally expect phase words when Byte 37 bit[3] is set.
static quint32 hzToPhaseWord(quint32 frequencyHz) {
    static const double kDspClockHz = 122880000.0;
    const double scaled = (4294967296.0 * (double)frequencyHz) / kDspClockHz;
    if (scaled <= 0.0) return 0;
    if (scaled >= 4294967295.0) return 0xFFFFFFFFu;
    return (quint32)scaled;
}
}

CProtocol2::CProtocol2() : m_lastSequence(0), m_lastPacketLen(0) {
    memset(m_rxSamplesPerDDC, 0, sizeof(m_rxSamplesPerDDC));
}

CProtocol2::~CProtocol2() {}

QList<quint16> CProtocol2::getRequiredPorts() {
    QList<quint16> ports = { 1024, 1025, 1026, 1027, 1028, 1029 };
    int nRx = Settings::instance()->getNumberOfReceivers();
    for (int i = 0; i < nRx; i++)
        ports.append((quint16)(1035 + i));
    return ports;
}

bool CProtocol2::isPacketValid(const unsigned char* data, int len) {
    Q_UNUSED(data)
    // Store len so getPacketType() can use it to distinguish DDC data from
    // High-Priority-Status packets without needing an extra parameter.
    m_lastPacketLen = len;

    // Protocol 2 packets have very specific fixed lengths:
    //   60   : High Priority Status (Hardware -> Host)
    //   1028 : Wideband ADC data
    //   1444 : DDC IQ data
    return (len == 60 || len == 1028 || len == 1444);
}

uint32_t CProtocol2::getSequence(const unsigned char* data) {
    uint32_t seq = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
    return seq;
}

int CProtocol2::getPacketType(const unsigned char* data) {
    Q_UNUSED(data)
    // Protocol 2 uses port numbers (not an in-band type field) to distinguish
    // packet purposes.  Since this method has no port context, we discriminate
    // by packet size (stored during the prior isPacketValid() call):
    //   DDC IQ data packets  : sent on port 1035+ and are large (typically 1444 bytes)
    //   High-Priority Status : sent on port 1025 and are 60 bytes
    //
    // Return 0x06 for IQ data (matches the readDeviceData() check).
    // Return 0x05 for High Priority Status (PC <- SDR).
    // Return 0x04 for Wideband ADC data (default 1028 bytes).
    if (m_lastPacketLen == 1028) return 0x04;
    if (m_lastPacketLen > 1040) return 0x06;
    if (m_lastPacketLen == 60) return 0x05;
    return 0xFF;
}

void CProtocol2::processInputBuffer(const QByteArray& buffer, DataEngine* de) {
    if (buffer.isEmpty()) return;
    int rxIdx = (unsigned char)buffer.at(0);
    
    if (rxIdx < 0 || rxIdx >= de->io.receivers || rxIdx >= de->RX.size())
        return;

    Receiver* rx = de->RX.at(rxIdx);
    if (!rx || !rx->qtwdsp) return;

    int& rxSamples = m_rxSamplesPerDDC[rxIdx];

    int s = 1; // IQ payload starts after the 1-byte receiver tag
    int samplesInPacket = (buffer.size() - 1) / 6;

    // Instrumentation
    static int p2AccCount = 0;
    if (p2AccCount++ < 10) {
        qDebug() << "[P2-TRACE] processInputBuffer: rx=" << rxIdx 
                 << "samples=" << samplesInPacket 
                 << "currentAcc=" << rxSamples;
    }

    for (int i = 0; i < samplesInPacket && s + 6 <= buffer.size(); i++) {
        int iSample = (int)((signed char)buffer.at(s++)) << 16;
        iSample |= (int)((unsigned char)buffer.at(s++)) << 8;
        iSample |= (int)((unsigned char)buffer.at(s++));

        int qSample = (int)((signed char)buffer.at(s++)) << 16;
        qSample |= (int)((unsigned char)buffer.at(s++)) << 8;
        qSample |= (int)((unsigned char)buffer.at(s++));

        rx->inBuf[rxSamples].re = (double)iSample / 8388607.0;
        rx->inBuf[rxSamples].im = (double)qSample / 8388607.0;

        rxSamples++;
        if (rxSamples == BUFFER_SIZE) {
            rx->enqueueData();
            QMetaObject::invokeMethod(rx, "dspProcessing", Qt::BlockingQueuedConnection);
            rxSamples = 0;
        }
    }
}

void CProtocol2::decodeCCBytes(const QByteArray& buffer, THPSDRParameter* io) {
    // Protocol 2 High Priority Status Packet (default port 1025, hardware→host)
    // Per spec v4.3 p.45-48:
    if (buffer.size() < 60) return;

    Settings* s = Settings::instance();

    io->ccRx.previous_dash = io->ccRx.dash;
    io->ccRx.previous_dot  = io->ccRx.dot;

    // Byte 4: [0]=PTT, [1]=Dot, [2]=Dash, [4]=PLL locked, [5]=FIFO empty, [6]=FIFO full
    bool ptt = (buffer.at(4) & 0x01);
    if (ptt != io->ccRx.ptt) {
        io->ccRx.ptt = ptt;
        s->setRadioState(ptt ? RadioState::MOX : RadioState::RX);
    }

    io->ccRx.dot  = (buffer.at(4) & 0x02);
    io->ccRx.dash = (buffer.at(4) & 0x04);
    
    // Additional status in Byte 4
    // bool pllLocked = (buffer.at(4) & 0x10);
    // bool fifoEmpty = (buffer.at(4) & 0x20);
    // bool fifoFull  = (buffer.at(4) & 0x40);

    // Byte 5: ADC overload bitmask [0]=ADC0 … [7]=ADC7
    uint8_t adcOvld = (uint8_t)buffer.at(5);
    io->ccRx.mercury1_LT2208 = (adcOvld & 0x01);
    io->ccRx.mercury2_LT2208 = (adcOvld & 0x02);
    io->ccRx.mercury3_LT2208 = (adcOvld & 0x04);
    io->ccRx.mercury4_LT2208 = (adcOvld & 0x08);

    if (adcOvld != 0) {
        s->setADCOverflow(2);
    }

    // Bytes 14-15: Forward power Alex0 (16-bit BE)
    uint16_t fwdPwrRaw = qFromBigEndian<uint16_t>(reinterpret_cast<const uchar*>(buffer.constData() + 14));
    io->ccRx.ain1 = fwdPwrRaw;
    io->alexForwardVolts = (double)fwdPwrRaw * (3.3 / 4095.0); // Assume 12-bit ADC referenced to 3.3V

    // Bytes 22-23: Reverse power Alex0 (16-bit BE)
    uint16_t revPwrRaw = qFromBigEndian<uint16_t>(reinterpret_cast<const uchar*>(buffer.constData() + 22));
    io->ccRx.ain2 = revPwrRaw;
    io->alexReverseVolts = (double)revPwrRaw * (3.3 / 4095.0);
    io->alexForwardPower = io->alexForwardVolts * io->alexForwardVolts / 0.09;
    io->alexReversePower = io->alexReverseVolts * io->alexReverseVolts / 0.09;
    Settings::instance()->setForwardPower(io->alexForwardPower);
    Settings::instance()->setReversePower(io->alexReversePower);

    // Calculate SWR
    if (io->alexForwardPower > 0.001) {
        qreal rho = sqrt(io->alexReversePower / io->alexForwardPower);
        if (rho > 0.999) rho = 0.999;
        qreal swr = (1.0 + rho) / (1.0 - rho);
        Settings::instance()->setSWR(swr);
    } else {
        Settings::instance()->setSWR(1.0);
    }

    // Bytes 49-50: Supply voltage (16-bit BE, millivolts)
    uint16_t supplyMV = qFromBigEndian<uint16_t>(reinterpret_cast<const uchar*>(buffer.constData() + 49));
    io->supplyVolts = (double)supplyMV / 1000.0;
    io->ccRx.ain6 = supplyMV;
    Settings::instance()->setSupplyVoltage(io->supplyVolts);

    // Byte 59: IO2,IO4,IO5,IO6,IO8 inputs
    uint8_t inputs = (uint8_t)buffer.at(59);
    io->ccRx.hermesI01 = (inputs & 0x01) != 0; // IO2
    io->ccRx.hermesI02 = (inputs & 0x02) != 0; // IO4
    io->ccRx.hermesI03 = (inputs & 0x04) != 0; // IO5
    io->ccRx.hermesI04 = (inputs & 0x08) != 0; // IO6
}

void CProtocol2::resetSequences() {
    m_seqMutex.lock();
    m_sequences.clear();
    m_seqMutex.unlock();
}

void CProtocol2::encodeCCBytes(unsigned char* buffer, THPSDRParameter* io, int& sendState, quint16& port) {
    Settings* s = Settings::instance();
    io->mutex.lock();
    // Protocol 2 High Priority and DDC packets must be 1444 bytes.
    // The provided buffer is already 1444 bytes.
    memset(buffer, 0, 1444);

    switch (sendState) {
        case 0: // General Packet (Port 1024)
            port = 1024;
            {
                uint32_t seq;
                m_seqMutex.lock();
                seq = qToBigEndian(m_sequences[1024]++);
                m_seqMutex.unlock();

                memcpy(buffer, &seq, 4);
                buffer[4] = 0x00; // Command - General Packet to SDR
                
                // Set Ports (Big Endian)
                quint16 ddcSpecPort = qToBigEndian((quint16)1025);
                memcpy(&buffer[5], &ddcSpecPort, 2);
                
                quint16 txSpecPort = qToBigEndian((quint16)1026);
                memcpy(&buffer[7], &txSpecPort, 2);
                
                quint16 hpPCPort = qToBigEndian((quint16)1027);
                memcpy(&buffer[9], &hpPCPort, 2);
                
                // Bytes 11-12: HP to PC port (hardware sends HP status FROM here, default 1025)
                // Use 0 so hardware uses its default.
                // buffer[11..12] already 0.
                
                quint16 ddcAudioPort = qToBigEndian((quint16)1028);
                memcpy(&buffer[13], &ddcAudioPort, 2);
                
                quint16 ducIQPort = qToBigEndian((quint16)1029);
                memcpy(&buffer[15], &ducIQPort, 2);
                
                quint16 ddc0Port = qToBigEndian((quint16)1035);
                memcpy(&buffer[17], &ddc0Port, 2);
                
                // Bytes 19-20: Mic samples source port  (0 = use default 1026)
                // Bytes 21-22: Wideband ADC0 source port (0 = use default 1027)
                // Bytes 23-59: wideband config / endian / reserved — leave 0.
                
                // Other global settings
                buffer[37] = 0x08; // Byte 37 bit[3]=1: send frequency fields as phase words
                buffer[38] = 0x00; // Hardware reset off
                buffer[39] = 0x00; // Big Endian data format (Bit 0=0)

                // Byte 23: wide_enable
                buffer[23] = s->getWidebandData() ? 0x01 : 0x00;
            }
            sendState = 1;
                SleeperThread::msleep(10);
            break;
            
        case 1: // DDC Specific Packet (Port 1025)
            port = 1025;
            {
                uint32_t seq;
                m_seqMutex.lock();
                seq = qToBigEndian(m_sequences[1025]++);
                m_seqMutex.unlock();

                memcpy(buffer, &seq, 4);

                // Byte 4: Number of ADCs
                buffer[4] = 1;
                // Byte 5: Dither enable per ADC (bit 0 = ADC0)
                buffer[5] = 0x00;
                // Byte 6: Random enable per ADC
                // buffer[6] = 0x00;

                // DDC enable bitmask across bytes 7..16 (DDC0..DDC79).
                const int ddcCount = qBound(0, io->receivers, 80);
                for (int ddc = 0; ddc < ddcCount; ++ddc) {
                    const int byteIdx = 7 + (ddc / 8);
                    const int bit = ddc % 8;
                    buffer[byteIdx] |= (uint8_t)(1u << bit);
                }

                // Configure each DDC: 6 bytes starting at buffer[17 + 6*i]
                //   [0] ADC selection  [1-2] sample rate (BE)  [3-4] sync map  [5] sample size
                const QList<TReceiver>& rxData = s->getReceiverDataList();
                for (int ddc = 0; ddc < io->receivers && ddc < rxData.size(); ddc++) {
                    int base = 17 + 6 * ddc;
                    uint16_t ddcRate = 48;
                    switch (rxData.at(ddc).sampleRate) {
                        case 48000:   ddcRate = 48;   break;
                        case 96000:   ddcRate = 96;   break;
                        case 192000:  ddcRate = 192;  break;
                        case 384000:  ddcRate = 384;  break;
                        case 768000:  ddcRate = 768;  break;
                        case 1536000: ddcRate = 1536; break;
                        default:      ddcRate = 48;   break;
                    }
                    uint16_t rateBE = qToBigEndian(ddcRate);
                    
                    buffer[base]     = 0x00;       // ADC0 for all DDCs
                    memcpy(&buffer[base + 1], &rateBE, 2); // sample rate
                    buffer[base + 3] = 0x00;       // sync map high byte (unused)
                    buffer[base + 4] = 0x00;       // sync map low byte (unused)
                    buffer[base + 5] = 24;         // 24-bit samples
                }
            }
            sendState = 2;
            break;
            
        case 2: // Transmitter Specific Packet (Port 1026)
            port = 1026;
            {
                uint32_t seq;
                m_seqMutex.lock();
                seq = qToBigEndian(m_sequences[1026]++);
                m_seqMutex.unlock();

                memcpy(buffer, &seq, 4);
                buffer[4] = 1; // Number of DACs
                
                // DUC 0 settings
                buffer[5] = 0x00;
                if (s->isInternalCw()) buffer[5] |= 0x02; // CW bit
                if (s->getCwKeyerMode() > 0) buffer[5] |= 0x08; // Iambic bit (rough mapping)
                
                // Byte 6 range per v4.3 is 0..127.
                buffer[6] = (unsigned char)qBound(0, s->getCwSidetoneVolume(), 127);
                
                uint16_t sideToneFreq = qToBigEndian((uint16_t)s->getCwSidetoneFreq());
                memcpy(&buffer[7], &sideToneFreq, 2);
                
                buffer[9] = (unsigned char)s->getCwKeyerSpeed();
                buffer[10] = (unsigned char)s->getCwKeyerWeight();
                
                uint16_t hangDelay = qToBigEndian((uint16_t)s->getCwHangTime());
                memcpy(&buffer[11], &hangDelay, 2);

                // v4.3 fields that should be explicitly populated.
                uint16_t ducRate = qToBigEndian((uint16_t)192); // 192ksps fixed for current hardware
                memcpy(&buffer[14], &ducRate, 2);
                buffer[16] = 24; // 24-bit DUC IQ samples
                buffer[17] = 0;  // CW ramp period (0 => firmware/app default)
            }
            sendState = 3;
            break;
            
        case 3: // High Priority Data Packet (Port 1027)
        default:
            port = 1027;
            {
                uint32_t seq;
                m_seqMutex.lock();
                seq = qToBigEndian(m_sequences[1027]++);
                m_seqMutex.unlock();

                memcpy(buffer, &seq, 4);

                // Bit 0: Run, Bit 1: PTT0, Bit 2: Dash, Bit 3: CWX
                buffer[4] = io->rcveIQ_toggle ? 0x01 : 0x00;
                if (io->ccTx.mox || io->ccTx.ptt) {
                    buffer[4] |= 0x02; // PTT0
                }

                // DDC RX frequencies: 4 bytes each starting at buffer[9 + 4*i]
                {
                    const QList<long>& freqs = s->getCtrFrequencies();
                    for (int ddc = 0; ddc < io->receivers && ddc < freqs.size(); ddc++) {
                        uint32_t phaseWord = qToBigEndian(hzToPhaseWord((quint32)freqs.at(ddc)));
                        memcpy(&buffer[9 + 4 * ddc], &phaseWord, 4);
                    }
                }

                // DUC0 TX frequency (buffer[329-332])
                if (s->getCtrFrequencies().size() > 0) {
                    uint32_t txPhaseWord = qToBigEndian(hzToPhaseWord((quint32)s->getCtrFrequencies().at(0)));
                    memcpy(&buffer[329], &txPhaseWord, 4);
                }

                // DUC0 drive level (buffer[345], scale 0-100 to 0-255)
                int drive = qBound(0, (int)io->ccTx.drivelevel, 100);
                buffer[345] = (unsigned char)((drive * 255) / 100);
            }
            sendState = 1; // cycle back to DDC Specific
                SleeperThread::msleep(10);
            break;
    }
    
    io->mutex.unlock();
}

QByteArray CProtocol2::formatStartStop(char value, quint16& port) {
    // Protocol 2 High Priority Data Packet (PC → SDR, Port 1027)
    // The spec and the hpsdrsim both require this packet to be exactly 1444 bytes.
    port = 1027;
    QByteArray commandDatagram(1444, '\0');
    unsigned char* buffer = reinterpret_cast<unsigned char*>(commandDatagram.data());

    // Sequence (big-endian)
    uint32_t seq;
    m_seqMutex.lock();
    seq = qToBigEndian(m_sequences[1027]++);
    m_seqMutex.unlock();
    memcpy(buffer, &seq, 4);
    Settings* s = Settings::instance();
    buffer[4] = value ? 0x01 : 0x00;
    const QList<long>& freqs = s->getCtrFrequencies();
    for (int ddc = 0; ddc < MAX_RECEIVERS && ddc < freqs.size(); ddc++) {
        uint32_t phaseWord = qToBigEndian(hzToPhaseWord((quint32)freqs.at(ddc)));
        memcpy(&buffer[9 + 4 * ddc], &phaseWord, 4);
    }

    return commandDatagram;
}

QByteArray CProtocol2::formatInitFrame(int rx, THPSDRParameter* io, quint16& port) {
    Q_UNUSED(io)
    // Protocol 2 General Configuration Packet (PC → SDR, port 1024, 60 bytes).
    // Must be sent before setting the Run bit so the device knows which ports
    // to use for each data stream.  sendInitFramesToNetworkDevice() calls us
    // once per receiver; we send the real config only for rx==0 and a no-op
    // for subsequent receivers to avoid redundant reconfigurations.
    if (rx > 0) {
        // No additional init needed per-receiver in Protocol 2.
        // Return a null/empty packet; sendInitFramesToNetworkDevice will send
        // 0 bytes which writeDatagram ignores (returns 0, not < 0).
        port = 1024;
        return QByteArray();
    }

    port = 1024;
    QByteArray pkt(60, 0);

    // Bytes 0-3: sequence number (0 for first packet)
    uint32_t seq;
    m_seqMutex.lock();
    seq = qToBigEndian(m_sequences[1024]++);
    m_seqMutex.unlock();
    memcpy(pkt.data(), &seq, 4);

    // Byte 4: 0x00 = General Packet to SDR
    pkt[4] = 0x00;

    // Bytes 5-6: DDC Specific port — hardware listens here for DDC Specific commands (default 1025)
    quint16 ddcSpecPort = qToBigEndian((quint16)1025);
    memcpy(pkt.data() + 5, &ddcSpecPort, 2);

    // Bytes 7-8: DUC Specific port — hardware listens here for DUC Specific commands (default 1026)
    quint16 txIqPort = qToBigEndian((quint16)1026);
    memcpy(pkt.data() + 7, &txIqPort, 2);

    // Bytes 9-10: High Priority from PC port — hardware listens here for HP commands from host (default 1027)
    quint16 hpPCPort = qToBigEndian((quint16)1027);
    memcpy(pkt.data() + 9, &hpPCPort, 2);

    // Bytes 11-12: High Priority to PC port — hardware sends HP status FROM this port (default 1025)
    // Use 0 to keep the hardware default (port 1025 on the hardware side).
    // pkt[11..12] already 0.

    // Bytes 13-14: DDC Audio port — hardware sends audio FROM this port (default 1028)
    quint16 ddcAudioPort = qToBigEndian((quint16)1028);
    memcpy(pkt.data() + 13, &ddcAudioPort, 2);

    // Bytes 15-16: DUC IQ port — hardware receives TX IQ on this port (default 1029)
    quint16 ducIqPort = qToBigEndian((quint16)1029);
    memcpy(pkt.data() + 15, &ducIqPort, 2);

    // Bytes 17-18: DDC0 IQ data source port — hardware sends DDC0 IQ FROM this port (default 1035)
    // DDC1 uses 1036, DDC2 uses 1037, etc.
    quint16 ddc0Port = qToBigEndian((quint16)1035);
    memcpy(pkt.data() + 17, &ddc0Port, 2);

    // Bytes 19-20: Mic samples source port — hardware sends mic data FROM here (default 1026)
    // Use 0 so hardware uses its default.
    // pkt[19..20] already 0.

    // Bytes 21-22: Wideband ADC0 source port (0 = use default 1027).
    // pkt[21..22] already 0.
    pkt[23] = Settings::instance()->getWidebandData() ? 0x01 : 0x00;
    // Byte 23: wide_enable = 1 → request wideband ADC data from device.
    // Bytes 24-25: wide_len = 0 → device default (512 samples/packet).
    // Byte 26: wide_size = 0 → device default (16 bits/sample).
    // Bytes 27-59: wide_rate, ppf, endian, reserved — leave 0.

    return pkt;
}

QByteArray CProtocol2::formatOutputPacket(const QByteArray& audioData, uint32_t& /*sequence*/) {
    // Protocol 2 DUC IQ packet (PC → SDR, port 1029) — exactly 1444 bytes:
    //   Bytes 0-3  : sequence number (big-endian uint32)
    //   Bytes 4-1443: 240 × 6 bytes — I(3 bytes) Q(3 bytes), 24-bit signed big-endian
    //
    // The provided audioData is already framed correctly by DataProcessor::setAudioBufferP2
    // as exactly 1440 bytes of 24-bit IQ samples.

    QByteArray pkt(1444, '\0');
    unsigned char* p = reinterpret_cast<unsigned char*>(pkt.data());

    // Sequence (big-endian)
    uint32_t seq;
    m_seqMutex.lock();
    seq = qToBigEndian(m_sequences[1029]++);
    m_seqMutex.unlock();
    memcpy(p, &seq, 4);

    // Copy the 1440-byte DUC payload
    int copySize = qMin(audioData.size(), 1440);
    if (copySize > 0) {
        memcpy(p + 4, audioData.constData(), copySize);
    }

    return pkt;
}
