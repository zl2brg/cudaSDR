#include "CProtocol2.h"
#include "cusdr_dataEngine.h"
#include "cusdr_settings.h"

#ifdef LOG_P2_NETWORK
#define P2_ROUTE_DEBUG qDebug().nospace() << "P2Route::\t"
#else
#define P2_ROUTE_DEBUG nullDebug()
#endif

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
    return (len > 4);
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
    // Return 0x04 for Wideband ADC data:
    //   16-byte header (4 seq + 12 zeros) + 512 samples x 16-bit = 1040 bytes.
    // Anything else is ignored.
    if (m_lastPacketLen == 1040) return 0x04;  // Wideband (16 hdr + 512*2 payload)
    if (m_lastPacketLen > 1000) return 0x06;
    if (m_lastPacketLen == 60) return 0x05;
    return 0xFF;
}

void CProtocol2::processInputBuffer(const QByteArray& buffer, DataEngine* de, quint16 sourcePort) {
    static quint64 p2ProcessCalls = 0;
    static quint64 p2DspKickCount = 0;
    static quint64 p2NoQtWdspCount = 0;
    static quint64 p2DspInvokeFailCount = 0;
    static quint64 p2RouteLogCount = 0;

    if (buffer.isEmpty()) return;
    ++p2ProcessCalls;
    if ((p2ProcessCalls % 500) == 1) {
        qDebug() << "P2 processInputBuffer call=" << p2ProcessCalls
                 << " size=" << buffer.size()
                 << " sourcePort=" << sourcePort
                 << " receivers=" << de->io.receivers
                 << " rxCount=" << de->RX.count();
    }

    if (de->io.receivers <= 0 || de->RX.isEmpty()) {
        if ((p2ProcessCalls % 100) == 1) {
            qDebug() << "P2 processInputBuffer early-return receivers/rx list";
        }
        return;
    }

    int ddcIndex = 0;
    if (sourcePort >= 1035) {
        ddcIndex = (int)(sourcePort - 1035);
    }

    if (ddcIndex < 0 || ddcIndex >= MAX_RECEIVERS) {
        if ((p2ProcessCalls % 100) == 1) {
            qDebug() << "P2 dropping packet with out-of-range DDC index" << ddcIndex
                     << "sourcePort=" << sourcePort;
        }
        return;
    }

    if (ddcIndex >= de->io.receivers || ddcIndex >= de->RX.count()) {
        if ((p2ProcessCalls % 100) == 1) {
            qDebug() << "P2 dropping packet for inactive DDC" << ddcIndex
                     << "sourcePort=" << sourcePort
                     << "receivers=" << de->io.receivers
                     << "rxCount=" << de->RX.count();
        }
        return;
    }

    int& rxSamples = m_rxSamplesPerDDC[ddcIndex];
    Receiver *rx = de->RX.at(ddcIndex);
    int s = 0; // IQ payload starts at the beginning of the buffer
    int samplesInPacket = buffer.size() / 6;

    ++p2RouteLogCount;
    if ((p2RouteLogCount % 500) == 1) {
        P2_ROUTE_DEBUG << "route srcPort=" << sourcePort
                       << " -> ddcIndex=" << ddcIndex
                       << " receivers=" << de->io.receivers
                       << " payload=" << buffer.size();
    }

    if ((p2ProcessCalls % 500) == 1) {
        qDebug() << "P2 sample accounting: samplesInPacket=" << samplesInPacket
                 << " ddcIndex=" << ddcIndex
                 << " rxSamples(before)=" << rxSamples
                 << " BUFFER_SIZE=" << BUFFER_SIZE;
    }

    for (int i = 0; i < samplesInPacket && s + 6 <= buffer.size(); i++) {
        int iSample = (int)((signed char)buffer.at(s++)) << 16;
        iSample |= (int)((unsigned char)buffer.at(s++)) << 8;
        iSample |= (int)((unsigned char)buffer.at(s++));

        int qSample = (int)((signed char)buffer.at(s++)) << 16;
        qSample |= (int)((unsigned char)buffer.at(s++)) << 8;
        qSample |= (int)((unsigned char)buffer.at(s++));

        if (rx->qtwdsp) {
            rx->inBuf[rxSamples].re = (double)iSample / 8388607.0;
            rx->inBuf[rxSamples].im = (double)qSample / 8388607.0;
        } else {
            ++p2NoQtWdspCount;
            if ((p2NoQtWdspCount % 1000) == 1) {
                qDebug() << "P2 RX" << ddcIndex << "qtwdsp not ready count=" << p2NoQtWdspCount;
            }
        }

        rxSamples++;
        if (rxSamples == BUFFER_SIZE) {
            if (rx->qtwdsp) {
                rx->enqueueData();
                bool invoked = QMetaObject::invokeMethod(rx, "dspProcessing", Qt::BlockingQueuedConnection);
                ++p2DspKickCount;
                if ((p2DspKickCount % 100) == 1) {
                    P2_ROUTE_DEBUG << "dspKick rx=" << ddcIndex
                                   << " srcPort=" << sourcePort
                                   << " count=" << p2DspKickCount
                                   << " invoked=" << invoked;
                }
                if (!invoked) {
                    ++p2DspInvokeFailCount;
                    if ((p2DspInvokeFailCount % 10) == 1) {
                        qDebug() << "P2 DSP invoke failed count=" << p2DspInvokeFailCount;
                    }
                }
            }
            rxSamples = 0;
        }
    }
}

void CProtocol2::decodeCCBytes(const QByteArray& buffer, THPSDRParameter* io) {
    // Protocol 2 High Priority Status Packet (default port 1025, hardware→host)
    // Per spec v4.3 p.45-48:
    if (buffer.size() < 60) return;

    Settings* set = Settings::instance();

    io->ccRx.previous_dash = io->ccRx.dash;
    io->ccRx.previous_dot  = io->ccRx.dot;

    // Byte 4: [0]=PTT, [1]=Dot, [2]=Dash, [4]=PLL locked, [5]=FIFO empty, [6]=FIFO full
    bool ptt = (buffer.at(4) & 0x01);
    if (ptt != io->ccRx.ptt) {
        io->ccRx.ptt = ptt;
        set->setRadioState(ptt ? RadioState::MOX : RadioState::RX);
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
        set->setADCOverflow(2);
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

    // Bytes 34-35: Temperature (16-bit BE, degrees C x 100)
    uint16_t tempRaw = qFromBigEndian<uint16_t>(reinterpret_cast<const uchar*>(buffer.constData() + 34));
    Settings::instance()->setTemperature((double)tempRaw / 100.0);

    // Bytes 36-37: Supply voltage (16-bit BE, millivolts)
    uint16_t supplyMV = qFromBigEndian<uint16_t>(reinterpret_cast<const uchar*>(buffer.constData() + 36));
    io->supplyVolts = (double)supplyMV / 1000.0;
    io->ccRx.ain6 = supplyMV;
    Settings::instance()->setSupplyVoltage(io->supplyVolts);

    // Byte 59: IO2,IO4,IO5,IO6,IO8 inputs
    uint8_t inputs = (uint8_t)buffer.at(59);
    io->ccRx.hermesI01 = !(inputs & 0x01); // Bit 0 is IO2 (active low)
    io->ccRx.hermesI02 = !(inputs & 0x02); // Bit 1 is IO4
    io->ccRx.hermesI03 = !(inputs & 0x04); // Bit 2 is IO5
    io->ccRx.hermesI04 = !(inputs & 0x08); // Bit 3 is IO6
}

void CProtocol2::encodeCCBytes(unsigned char* buffer, THPSDRParameter* io, int& sendState, quint16& port) {
    Settings* set = Settings::instance();
    io->mutex.lock();
    // Protocol 2 High Priority and DDC packets must be 1444 bytes.
    // The provided buffer is already 1444 bytes.
    memset(buffer, 0, 1444);

    if (sendState == 0) sendState = 1;
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

                // Byte 4: Number of ADCs
                buffer[4] = 1;
                // Byte 5: Dither enable per ADC (bit 0 = ADC0)
                buffer[5] = 0x00;
                // Byte 6: Random enable per ADC
                // buffer[6] = 0x00;

                // DDC enable bitmask: one bit per DDC (bit 0 = DDC0, bit 1 = DDC1, ...)
                buffer[7] = (uint8_t)((1 << io->receivers) - 1);

                // Configure each DDC: 6 bytes starting at buffer[17 + 6*i]
                //   [0] ADC selection  [1-2] sample rate (BE)  [3-4] sync map  [5] sample size
                const QList<TReceiver>& rxData = set->getReceiverDataList();
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
                // ...existing case 3 body...
                uint32_t seq = qToBigEndian(m_sequences[1027]++);
                memcpy(buffer, &seq, 4);

                // During startup staging, keep Run low until explicit final
                // start command is sent by formatStartStop().
                buffer[4] = io->rcveIQ_toggle ? 0x01 : 0x00;
                if (io->ccTx.mox || io->ccTx.ptt) {
                    buffer[4] |= 0x02; // PTT0
                }

                // DDC RX frequencies: 4 bytes each starting at buffer[9 + 4*i]
                {
                    const QList<long>& freqs = set->getCtrFrequencies();
                    for (int ddc = 0; ddc < io->receivers && ddc < freqs.size(); ddc++) {
                        uint32_t freq = qToBigEndian((uint32_t)freqs.at(ddc));
                        memcpy(&buffer[9 + 4 * ddc], &freq, 4);
                    }
                }

                // DUC0 TX frequency (buffer[333-336])
                uint32_t txfreq = qToBigEndian((uint32_t)set->getCtrFrequencies().at(0));
                memcpy(&buffer[333], &txfreq, 4);

                // DUC0 drive level (buffer[345], scale 0-100 to 0-255)
                int drive = qBound(0, (int)io->ccTx.drivelevel, 100);
                buffer[345] = (unsigned char)((drive * 255) / 100);
            }
            sendState = 1; // cycle back to DDC Specific
            break;
    }
    
    io->mutex.unlock();
}

QByteArray CProtocol2::formatStartStop(char value, quint16& port) {
    // Protocol 2 High Priority Data Packet (PC → SDR, Port 1027)
    // The spec and the hpsdrsim both require this packet to be exactly 1444 bytes.
    // The hpsdrsim highprio_thread does `if (rc != 1444) { break; }` and exits
    // if it gets anything shorter, so the run bit is never seen and RX never starts.
    port = 1027;
    QByteArray commandDatagram(1444, '\0');
    uint32_t seq = qToBigEndian(m_sequences[1027]++);
    memcpy(commandDatagram.data(), &seq, 4);
    commandDatagram[4] = value ? 0x01 : 0x00;
    return commandDatagram;
}

QByteArray CProtocol2::formatInitFrame(int rx, THPSDRParameter* io, quint16& port) {
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
    uint32_t seq = qToBigEndian(m_sequences[1024]++);
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
    // Use 0 to keep the hardware default.
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

    // Byte 23: wide_enable = 1 → request wideband ADC data from device.
    // Bytes 24-25: wide_len = 0 → device default (512 samples/packet).
    // Byte 26: wide_size = 0 → device default (16 bits/sample).
    // Bytes 27-59: wide_rate, ppf, endian, reserved — leave 0.
    pkt[23] = 1;

    return pkt;
}

QByteArray CProtocol2::formatOutputPacket(const QByteArray& audioData, uint32_t& sequence) {
    // Protocol 2 DUC IQ packet (PC → SDR, port 1029) — exactly 1444 bytes:
    //   Bytes 0-3  : sequence number (big-endian uint32)
    //   Bytes 4-1443: 240 × 6 bytes — I(3 bytes) Q(3 bytes), 24-bit signed big-endian
    //
    // Source: P1 output_buffer (512 bytes, IO_BUFFER_SIZE):
    //   Bytes 0-7:     Metis/P1 header (8 bytes, not used here)
    //   Bytes 8+n*8+0,+1 : L RX audio  (unused for TX)
    //   Bytes 8+n*8+2,+3 : R RX audio  (unused for TX)
    //   Bytes 8+n*8+4,+5 : TX I  (16-bit signed big-endian)
    //   Bytes 8+n*8+6,+7 : TX Q  (16-bit signed big-endian)
    //   63 samples total: (512 - 8) / 8 = 63
    //
    // 16→24-bit conversion: [hi, lo, 0x00] preserves sign and scales correctly.
    // Samples 63-239 are zero-padded (silent) since we only have 63 per call.

    const int NUM_P2_SAMPLES = 240;           // hpsdrsim tx_thread expects exactly this
    const int P1_HEADER      = 8;             // IO_HEADER_SIZE
    const int P1_SAMPLE_BYTES= 8;             // L(2)+R(2)+I(2)+Q(2)
    const int P1_SAMPLES     = 63;            // (512 - 8) / 8

    QByteArray pkt(4 + NUM_P2_SAMPLES * 6, '\0');
    unsigned char* p = reinterpret_cast<unsigned char*>(pkt.data());

    // Sequence (big-endian)
    uint32_t seq = qToBigEndian(sequence);
    memcpy(p, &seq, 4);
    p += 4;

    const unsigned char* src = reinterpret_cast<const unsigned char*>(audioData.constData());
    const int nSamples = (audioData.size() >= P1_HEADER)
                         ? qMin((audioData.size() - P1_HEADER) / P1_SAMPLE_BYTES, P1_SAMPLES)
                         : 0;

    for (int i = 0; i < nSamples; ++i) {
        const unsigned char* s = src + P1_HEADER + i * P1_SAMPLE_BYTES;
        // TX I at byte-offset +4,+5; TX Q at +6,+7
        p[0] = s[4]; p[1] = s[5]; p[2] = 0x00;  // I: hi, lo, 0
        p[3] = s[6]; p[4] = s[7]; p[5] = 0x00;  // Q: hi, lo, 0
        p += 6;
    }
    // Bytes beyond nSamples*6 are already zero-initialised.

    sequence++;
    return pkt;
}
