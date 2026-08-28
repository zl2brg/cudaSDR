/**
* @file  cusdr_dataIO.cpp
* @brief Data IO class
* @author Hermann von Hasseln, DL3HVH
* @version 0.1
* @date 2011-10-01
 *
 *
*/

/* Possible TODO's
 * Client / Server archetecture
 *- Serial port PTT switch support
- DAW type TX audio processing
- A high degree of customization of the spectral display
- PureSignal linearization
- Squelch - done
- ASIO support
- Focusmaster support
- Programmable RX filter presets - done
- User selectable filter shapes
- Windows support
- Multiple simultaneous CAT connections
- MIDI controller support

 *   Copyright 2011 Hermann von Hasseln, DL3HVH
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#define LOG_DATAIO

#include "cusdr_dataIO.h"
#include "cusdr_dataEngine.h"
#include "Models/RadioTelemetry.h"
#include "protocol_boundary_utils.h"
#include "IHPSDRProtocol.h"
#include "soundout.h"
#include <QNetworkInterface>


#ifdef LOG_P2_NETWORK
#define P2_NET_DEBUG DATAIO_DEBUG
#else
#define P2_NET_DEBUG nullDebug()
#endif

namespace {
void reportIqSequenceSync(uint32_t sequence, uint32_t& oldSequence, QElapsedTimer& packetLossTimer)
{
    RadioTelemetry* tel = telemetryFromSettings();
    if (!tel) {
        oldSequence = sequence;
        return;
    }

    const bool firstPacket = (oldSequence == 0xFFFFFFFFu);
    const bool inSequence = firstPacket || (sequence == oldSequence + 1);
    if (inSequence) {
        tel->setProtocolSync(1);
    } else if (packetLossTimer.elapsed() > 100) {
        tel->setProtocolSync(2);
        tel->setPacketLoss(2);
        packetLossTimer.restart();
    }
    oldSequence = sequence;
}
} // namespace

DataIO::DataIO(QObject* parent)
	: QObject(parent)
	, set(Settings::instance())
    , m_dataIOSocket(nullptr)
	, m_dataEngine(nullptr)
	, m_dataIOSocketOn(false)
	, m_networkDeviceRunning(false)
	, m_setNetworkDeviceHeader(true)
	, m_sequence(0)
	, m_oldSequence(0xFFFFFFFF)
	, m_sequenceWideBand(0)
	, m_oldSequenceWideBand(0xFFFFFFFF)
	, m_wbBuffers(31)
	, m_wbCount(0)
	, m_socketBufferSize(set->getSocketBufferSize())
	, m_sendEP4(false)
	, m_manualBufferSize(set->getManualSocketBufferSize())
	, m_widebandMissedAccum(0)
	, m_stopped(false)
{
	// Size the datagram buffer to the largest possible packet:
	// Protocol 1: 1032 bytes (METIS_DATA_SIZE)
	// Protocol 2: up to 1444 bytes (DDC IQ data packet)
	m_datagram.resize(1444);
	m_wbDatagram.resize(0);

	m_sendSequence = 0L;
	m_oldSendSequence = 0L;

	m_packetLossTime.start();

	  connect(set, &Settings::sampleRateChanged, 
            this, &DataIO::setSampleRateSlot);

    connect(set, &Settings::manualSocketBufferChanged, 
            this, &DataIO::setManualSocketBufferSize);

    connect(set, &Settings::socketBufferSizeChanged, 
            this, &DataIO::setSocketBufferSize);

#ifndef USE_INTERNAL_AUDIO
     m_pSoundCardOut = std::make_unique<CSoundOut>(this);

    //RRK pass -1 to get the systems "default" audio device
    m_pSoundCardOut->Start(-1, true, 48000, false);
    m_pSoundCardOut->SetVolume(80);
#endif
}

namespace {

int rxSocketBufferSizeForRate(int sampleRate) {
    switch (sampleRate) {
        case 48000:
            return 16 * 1024;
        case 96000:
            return 32 * 1024;
        case 192000:
            return 64 * 1024;
        case 384000:
            return 128 * 1024;
        case 768000:
            return 256 * 1024;
        case 1536000:
            return 512 * 1024;
        default:
            return 16 * 1024;
    }
}

bool isProtocol2(IHPSDRProtocol* protocol) {
    return protocol && protocol->getHeaderSize() == ProtocolBoundaryUtils::kProtocol2HeaderSize;
}

bool isProtocol1(IHPSDRProtocol* protocol) {
    return protocol && protocol->getHeaderSize() == ProtocolBoundaryUtils::kProtocol1HeaderSize;
}

bool isLocalAddress(const QHostAddress& address) {
    if (address.isNull() || address.isLoopback()) {
        return true;
    }

    const QList<QHostAddress> localAddresses = QNetworkInterface::allAddresses();
    for (const QHostAddress& localAddress : localAddresses) {
        if (localAddress == address) {
            return true;
        }
    }

    return false;
}
}

DataIO::~DataIO() {
    stop();
    for (auto socket : m_sockets) {
        if (socket) {
            socket->close();
            delete socket;
        }
    }
    m_sockets.clear();
    m_dataIOSocket = nullptr;
}

void DataIO::stop() {
    {
        QMutexLocker locker(&networkIOMutex);
        m_stopped = true;
    }

    if (m_pSoundCardOut) {
        QThread::msleep(100);
        m_pSoundCardOut->Stop();
        m_pSoundCardOut.reset(); // Reset smart pointer instead of delete
    }
}

void DataIO::initDataReceiverSocket() {
    m_stopped = false;

    QList<quint16> ports = { DEVICE_PORT };
    if (m_protocol) {
        ports = m_protocol->getRequiredPorts();
    }

    // Close and clear existing extra sockets
    for (auto socket : m_sockets) {
        if (socket) {
            socket->close();
            delete socket;
        }
    }
    m_sockets.clear();
    m_dataIOSocket = nullptr;

    int newBufferSize = 16 * 1024;

	if (m_manualBufferSize) {
		newBufferSize = m_socketBufferSize * 1024;
	}
    else {
        newBufferSize = rxSocketBufferSizeForRate(m_sampleRate);
    }

    const bool sameHostDevice = isLocalAddress(hpsdrDeviceIPAddress);
    const QHostAddress bindAddr = sameHostDevice
        ? QHostAddress(QHostAddress::AnyIPv4)
        : QHostAddress(set->getHPSDRDeviceLocalAddr());

    if (sameHostDevice) {
        // On the same host, hpsdrsim already owns 1024 (and P2's extra ports).
        // Both protocols reply to the source port of the start/control datagram,
        // so bind one ephemeral socket instead of competing for 1024.
        ports = { DEVICE_PORT };
        DATAIO_DEBUG << "initDataReceiverSocket: same-host device detected; using one ephemeral socket";
    }

    for (quint16 port : ports) {
        const quint16 bindPort = sameHostDevice ? 0 : port;
        QUdpSocket* socket = new QUdpSocket(this);
        if (socket->bind(bindAddr,
                         bindPort,
                         QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress))
        {
socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, newBufferSize);
            connect(socket, &QUdpSocket::errorOccurred, this, &DataIO::displayDataReceiverSocketError);
            connect(socket, &QUdpSocket::readyRead, this, &DataIO::readDeviceData);

            m_sockets[socket->localPort()] = socket;
            m_socketLogicalPorts[socket] = port;
            if (port == ports.first()) m_dataIOSocket = socket;
            
            DATAIO_DEBUG << "Bound receiver socket to logical port " << port << " (localPort=" << socket->localPort() << ") with buffer size " << newBufferSize;
        } else {
            DATAIO_DEBUG << "Failed to bind receiver socket to port " << bindPort << " for logical port " << port << ": " << socket->errorString();
            delete socket;
        }
    }

	m_dataIOSocketOn = true;

    if (m_pendingP1Start && isProtocol1(m_protocol)) {
        const int rxCount = m_pendingP1RxCount;
        const char startByte = m_pendingP1StartByte;
        m_pendingP1Start = false;
        for (int i = 0; i < rxCount; ++i)
            sendInitFramesToNetworkDevice(i);
        networkDeviceStartStop(startByte);
        DATAIO_DEBUG << "Protocol 1 start sent from bound socket localPort="
                     << (m_dataIOSocket ? m_dataIOSocket->localPort() : 0);
    }
}

void DataIO::armProtocol1Start(int rxCount, char startByte) {
    m_pendingP1RxCount = rxCount;
    m_pendingP1StartByte = startByte;
    m_pendingP1Start = true;
}


void DataIO::readDeviceData() {
    QUdpSocket* socket = qobject_cast<QUdpSocket*>(sender());
    if (!socket || !m_protocol) return;

    if (isProtocol2(m_protocol)) {
        readDeviceDataP2(socket);
    } else {
        readDeviceDataP1(socket);
    }
}

void DataIO::readDeviceDataP1(QUdpSocket* socket) {
    while (socket->hasPendingDatagrams()) {
        if (!m_iqArrivalTimer.isValid())
            m_iqArrivalTimer.start();
        QHostAddress Address;
        quint16 Port = 0;
        qint64 size = socket->readDatagram(m_datagram.data(), m_datagram.size(), &Address, &Port);
        if (!m_protocol || !m_protocol->isPacketValid((const unsigned char*)m_datagram.data(), size)) continue;

        int type = m_protocol->getPacketType((const unsigned char*)m_datagram.data());
        if (type == ProtocolBoundaryUtils::kPacketTypeP1IqPrimary || type == ProtocolBoundaryUtils::kPacketTypeP1IqLoopback) { // IQ data (P1 EP6 or EP2 loopback)
            m_sequence = m_protocol->getSequence((const unsigned char*)m_datagram.data());
            reportIqSequenceSync(m_sequence, m_oldSequence, m_packetLossTime);
            ++m_iqPacketCount;

            {
                const int hdrSize = m_protocol->getHeaderSize();
                bool enqueued = false;
                {
                    QMutexLocker locker(&networkIOMutex);
                    if (!iq_queue.isFull()) {
                    iq_queue.enqueue(TIQPacket(m_datagram.mid(hdrSize, size - hdrSize), 0));
                        enqueued = true;
                    }
                }
                if (enqueued)
                    emit (readydata());
                else
                    ++m_iqDropCount;
            }

            if (m_iqArrivalTimer.elapsed() >= 5000) {
                if (m_iqDropCount > 0) {
                    DATAIO_DEBUG << "IQ queue drops in last 5s:" << m_iqDropCount
                                 << "(received" << m_iqPacketCount << "packets)";
                }
                m_iqPacketCount = 0;
                m_iqDropCount = 0;
                m_iqArrivalTimer.restart();
            }
        }
        else if (type == ProtocolBoundaryUtils::kPacketTypeWideband) {
            processWidebandPacket(size);
        }
    }
}

void DataIO::readDeviceDataP2(QUdpSocket* socket) {
    static quint64 p2DatagramsSeen = 0;
    static quint64 p2IqPacketsSeen = 0;
    static quint64 p2HpPacketsSeen = 0;
    static quint64 p2WidePacketsSeen = 0;

    while (socket->hasPendingDatagrams()) {
        if (!m_iqArrivalTimer.isValid())
            m_iqArrivalTimer.start();
        QHostAddress Address;
        quint16 Port = 0;
        qint64 size = socket->readDatagram(m_datagram.data(), m_datagram.size(), &Address, &Port);
        ++p2DatagramsSeen;

        if ((p2DatagramsSeen % 500) == 1) {
            P2_NET_DEBUG << "P2 RX datagram: localPort=" << socket->localPort()
                         << " =" << Address.toString()
                         << " Port=" << Port
                         << " size=" << size
                         << " total=" << p2DatagramsSeen;
        }

        if (!m_protocol || !m_protocol->isPacketValid((const unsigned char*)m_datagram.data(), size)) continue;

        // Protocol 2 simulator may source wideband packets from an ephemeral
        // UDP source port. Classify by packet size first, then by port.
        if (size == ProtocolBoundaryUtils::kProtocol2WidebandPacketSize) { // Wideband ADC packet: 16-byte header + 1024 payload
            ++p2WidePacketsSeen;
            if ((p2WidePacketsSeen % 100) == 1) {
                P2_NET_DEBUG << "P2 wideband: localPort=" << socket->localPort()
                             << " =" << Address.toString()
                             << " Port=" << Port
                             << " wideTotal=" << p2WidePacketsSeen;
            }
            processWidebandPacket(size);
        }
        else if (size >= ProtocolBoundaryUtils::kProtocol2IqPacketSize) { // DDC IQ packet (typically 1444 bytes)
            ++p2IqPacketsSeen;
            ++m_iqPacketCount;
            m_sequence = m_protocol->getSequence((const unsigned char*)m_datagram.data());
            reportIqSequenceSync(m_sequence, m_oldSequence, m_packetLossTime);

            {
                bool enqueued = false;
                const int hdrSize = m_protocol->getHeaderSize();
                quint16 effectiveSourcePort = Port;
                if (effectiveSourcePort < ProtocolBoundaryUtils::Ports::P2Ddc0Port
                    || effectiveSourcePort >= (ProtocolBoundaryUtils::Ports::P2Ddc0Port + MAX_RECEIVERS)) {
                    // Same-host ephemeral socket: simulator may source IQ from a
                    // non-1035 port. Map to DDC0 (or logical port if it is a DDC).
                    const quint16 logical = m_socketLogicalPorts.value(socket, ProtocolBoundaryUtils::Ports::P2Ddc0Port);
                    effectiveSourcePort = (logical >= ProtocolBoundaryUtils::Ports::P2Ddc0Port
                                           && logical < (ProtocolBoundaryUtils::Ports::P2Ddc0Port + MAX_RECEIVERS))
                        ? logical
                        : ProtocolBoundaryUtils::Ports::P2Ddc0Port;
                }
                {
                    QMutexLocker locker(&networkIOMutex);
                    if (!iq_queue.isFull()) {
                        iq_queue.enqueue(TIQPacket(m_datagram.mid(hdrSize, size - hdrSize), effectiveSourcePort));
                        enqueued = true;
                    }
                }

                if (enqueued && (p2IqPacketsSeen % 500) == 1) {
                    P2_NET_DEBUG << "P2 IQ enqueue: localPort=" << socket->localPort()
                                 << " Port=" << Port
                                 << " effectiveSourcePort=" << effectiveSourcePort
                                 << " size=" << size
                                 << " hdr=" << hdrSize
                                 << " payload=" << (size - hdrSize)
                                 << " queueCount=" << iq_queue.count()
                                 << " iqTotal=" << p2IqPacketsSeen;
                }
                if (enqueued) {
                    emit (readydata());
                } else {
                    ++m_iqDropCount;
                    P2_NET_DEBUG << "P2 IQ queue FULL: localPort=" << socket->localPort()
                                 << " Port=" << Port
                                 << " size=" << size
                                 << " iqTotal=" << p2IqPacketsSeen;
                }
            }

            if (m_iqArrivalTimer.elapsed() >= 5000) {
                if (m_iqDropCount > 0) {
                    DATAIO_DEBUG << "P2 IQ queue drops in last 5s:" << m_iqDropCount
                                 << "(received" << m_iqPacketCount << "packets)";
                }
                m_iqPacketCount = 0;
                m_iqDropCount = 0;
                m_iqArrivalTimer.restart();
            }
        }
        else if (size == ProtocolBoundaryUtils::kProtocol2HpStatusPacketSize) { // High Priority Status (P2)
            ++p2HpPacketsSeen;
            if ((p2HpPacketsSeen % 100) == 1) {
                P2_NET_DEBUG << "P2 HP status: localPort=" << socket->localPort()
                             << " =" << Address.toString()
                             << " Port=" << Port
                             << " hpTotal=" << p2HpPacketsSeen;
            }
            if (m_dataEngine)
                m_dataEngine->decodeCCBytes(m_datagram.left(size));
            else if (m_protocol)
                m_protocol->decodeCCBytes(m_datagram.left(size), m_dataEngine);
        }
        else {
            if ((p2DatagramsSeen % 500) == 1) {
                P2_NET_DEBUG << "P2 unclassified datagram: localPort=" << socket->localPort()
                             << " =" << Address.toString()
                             << " Port=" << Port
                             << " size=" << size;
            }
        }
    }
}

void DataIO::processWidebandPacket(qint64 size) {
    if (!m_protocol) return;
    m_sequenceWideBand = m_protocol->getSequence((const unsigned char*)m_datagram.data());

    // Check for missed packets (skip first packet and ignore wraparound artifacts)
    uint32_t missed = m_sequenceWideBand - m_oldSequenceWideBand;
    if (m_oldSequenceWideBand != 0xFFFFFFFF && missed > 1 && missed < 1000) {
        m_widebandMissedAccum += (missed - 1);  // accumulated lost packets
        
        // Log consolidated result once every 10 seconds
        if (!m_widebandLogTimer.isValid()) {
            m_widebandLogTimer.start();
        }
        if (m_widebandLogTimer.elapsed() > 10000) {
            if (m_widebandMissedAccum > 0) {
                DATAIO_DEBUG << "wideband readData: total missed " << m_widebandMissedAccum << " packages in 10 seconds.";
            }
            m_widebandMissedAccum = 0;
            m_widebandLogTimer.restart();
        }
        
        if (m_packetLossTime.elapsed() > 100) {
            if (RadioTelemetry* tel = telemetryFromSettings()) {
                tel->setPacketLoss(2);
            }
            m_packetLossTime.restart();
        }
    }
    m_oldSequenceWideBand = m_sequenceWideBand;

    if ((m_wbBuffers & (m_sequenceWideBand & 0xFF)) == 0) {
        m_sendEP4 = true;
        m_wbCount = 0;
        m_wbDatagram.resize(0);
    }

    if (m_sendEP4) {
        const int hdrSize = m_protocol->getHeaderSize();
        m_wbDatagram.append(m_datagram.mid(hdrSize, size - hdrSize));
        if (m_wbCount++ == m_wbBuffers) {
            m_sendEP4 = false;
            wb_queue.enqueue(m_wbDatagram);
            m_wbDatagram.resize(0);
        }
    }
}

void DataIO::readData() {

	qint64 length = inputBuffer.length();
	
	int buffers = qRound((float) length/128);

	DATAIO_DEBUG << "input buffer length " << length << " buffers " << buffers;
	while (!m_stopped) {
		for (int i = 0; i < buffers; i++) {
            QList<qreal> samples = inputBuffer.mid(i*128, 128);
            QVector<float> floatSamples;
            floatSamples.reserve(samples.size());
            for (qreal s : samples) floatSamples.append(static_cast<float>(s));
			soapy_iq_queue.enqueue(floatSamples);
			if (m_stopped) break;
		}
	}
	m_stopped = false;
}

void DataIO::sendInitFramesToNetworkDevice(int rx) {

	if (!m_protocol || !m_dataIOSocket) return;
    quint16 port = DEVICE_PORT;
    QByteArray initDatagram = m_protocol->formatInitFrame(rx, m_dataEngine, m_radioModel, port);

    // Protocol 2 returns an empty datagram for rx > 0 (config only needed once).
    if (initDatagram.isEmpty()) return;

	if (m_dataIOSocket->writeDatagram(initDatagram.data(), initDatagram.size(), hpsdrDeviceIPAddress, port) < 0) {
		DATAIO_DEBUG << "error sending init data to device: " << qPrintable(m_dataIOSocket->errorString());
	}
	else {
		DATAIO_DEBUG << "init frames sent to network device. " << rx << " port " << port;
	}
}

void DataIO::networkDeviceStartStop(char value) {

	TNetworkDevicecard metis = set->getCurrentMetisCard();

    if (m_protocol && m_dataIOSocket) {
        quint16 port = DEVICE_PORT;
		m_commandDatagram = m_protocol->formatStartStop(value, port);
		if (m_dataIOSocket->writeDatagram(m_commandDatagram, metis.ip_address, port) == m_commandDatagram.size()) {

			if (value != 0) {
				DATAIO_DEBUG << "sent start command to device at: "<< qPrintable(metis.ip_address.toString()) << " port " << port;
				m_networkDeviceRunning = true;
			}
			else {
				DATAIO_DEBUG << "sent stop command to device at: "<< qPrintable(metis.ip_address.toString()) << " port " << port;
				m_networkDeviceRunning = false;
			}
		}
		else
			DATAIO_DEBUG << "device start/stop: sending command to device failed.";
    }
}

void DataIO::sendAudio(u_char *buf) {
	// TODO(P2-TX-AUDIO): This function decodes audio from the P1 Metis/Hermes
	// output_buffer format: a 512-byte frame with an 8-byte Metis header followed
	// by interleaved L/R/I/Q 16-bit samples at bytes 8, 16, 24 ...
	// In Protocol 2, the equivalent function is full_txBuffer() in DataProcessor
	// which calls formatOutputPacket() and sends a DUC IQ packet to port 1029.
	// This DataIO::sendAudio path is called from full_txBuffer() only for
	// QSDR::Metis / QSDR::Hermes interfaces.  For P2, no equivalent HW interface
	// enum value routes here, so RX audio playback is silently skipped.
	// Fix: either map P2 hardware to an existing enum, or add a QSDR::ProtocolV2
	// enum case and handle it here or in full_txBuffer().
#ifndef USE_INTERNAL_AUDIO
	static TYPECPX cbuf[252];
	int i, j;
	short sample;

	for(i = 8, j = 0; i < 512; i += 8, j++) {
		//bytes are L,R,I,Q skip the I,Q
		sample = buf[i] << 8 | buf[i+1]; //left
		cbuf[j].re = (double)sample;
		sample = buf[i+2] << 8 | buf[i+3]; //right
		cbuf[j].im = (double)sample;
	}
    if((m_stopped != true) && m_pSoundCardOut)
        m_pSoundCardOut->PutOutQueue(63, cbuf);
#else
    Q_UNUSED(buf)
#endif
}

void DataIO::writeData() {

    if (!m_protocol || !m_dataIOSocket) return;

    // Protocol 2: formatOutputPacket returns the complete 1444-byte DUC IQ packet.
    // Send it in a single call to port 1029; bypass the P1 two-call toggle.
    if (isProtocol2(m_protocol)) {
        QByteArray ducPkt = m_protocol->formatOutputPacket(audioDatagram, m_sendSequence);
        static const quint16 DUC_PORT = 1029;
        if (m_dataIOSocket->writeDatagram(ducPkt,
                                          set->getCurrentMetisCard().ip_address,
                                          DUC_PORT) < 0) {
            DATAIO_DEBUG << "P2 TX: error sending DUC IQ: " << m_dataIOSocket->errorString();
        }
        m_oldSendSequence = m_sendSequence - 1; // keep tracking consistent
        return;
    }

    // Protocol 1 two-call toggle: first call stores the Metis header,
    // second call appends audio and sends the 1032-byte packet.
	if (m_setNetworkDeviceHeader) {

		m_outDatagram = m_protocol->formatOutputPacket(audioDatagram, m_sendSequence);
        m_setNetworkDeviceHeader = false;
    }
	else {

		m_outDatagram += audioDatagram;
		if (m_dataIOSocket->writeDatagram(m_outDatagram, set->getCurrentMetisCard().ip_address, DEVICE_PORT) < 0) {
			DATAIO_DEBUG << "error sending data to device: " << m_dataIOSocket->errorString();
		}

		if (m_sendSequence != m_oldSendSequence + 1) {
			DATAIO_DEBUG << "output sequence error: old = " << m_oldSendSequence << "; new =" << m_sendSequence;
		}

		m_oldSendSequence = m_sendSequence;
		m_setNetworkDeviceHeader = true;
    }
}

qint64 DataIO::sendProtocol2ControlDatagram(const QByteArray &datagram, const QHostAddress &address, quint16 port) {
    if (!m_dataIOSocket) {
        DATAIO_DEBUG << "P2 control send skipped: m_dataIOSocket is null";
        return -1;
    }
    if (datagram.isEmpty()) {
        return 0;
    }
    return m_dataIOSocket->writeDatagram(datagram, address, port);
}

void DataIO::displayDataReceiverSocketError(QAbstractSocket::SocketError error) {
    QUdpSocket* socket = qobject_cast<QUdpSocket*>(sender());
    QString errorStr = socket ? socket->errorString() : "Unknown socket error";
	DATAIO_DEBUG << "data IO socket error: " << errorStr;
	DATAIO_DEBUG << "data IO socket error: " << error;
}

void DataIO::setManualSocketBufferSize(bool value) {

	m_manualBufferSize = value;
	int socketBufferSize = 1024 * set->getSocketBufferSize();

	QMutexLocker locker(&networkIOMutex);
    if (m_manualBufferSize) {
        DATAIO_DEBUG << "set data IO socket BufferSize to " << m_socketBufferSize;
    } else {
        DATAIO_DEBUG << "set data IO socket BufferSize to 32 kB.";
        socketBufferSize = 32 * 1024;
    }

    for (auto socket : m_sockets) {
        if (socket && socket->isValid()) {
            socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, socketBufferSize);
        }
    }
}

void DataIO::setSocketBufferSize(int value) {

	int socketBufferSize = value * 1024;
	DATAIO_DEBUG << "m_socketBufferSize = " << value;

	QMutexLocker locker(&networkIOMutex);
    for (auto socket : m_sockets) {
        if (socket && socket->isValid()) {
            socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, socketBufferSize);
        }
    }
}

void DataIO::setSampleRateSlot(int value) {
	m_sampleRate = value;

	int bufferSize = rxSocketBufferSizeForRate(value);
	{
	QMutexLocker locker(&networkIOMutex);
    DATAIO_DEBUG << "socket buffer size set to" << (bufferSize / 1024) << "kB for sample rate" << value;

    for (auto socket : m_sockets) {
        if (socket && socket->isValid()) {
            socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, bufferSize);
        }
    }
	} // QMutexLocker released here

#ifndef USE_INTERNAL_AUDIO
    // Reset the sound card output queue so stale samples from the old rate
    // don't cause choppy audio after the DSP channel is rebuilt.
    if (m_pSoundCardOut)
        m_pSoundCardOut->Reset();
#endif
}


void DataIO::set_wbBuffers(int val) {
    if (isProtocol1(m_protocol)) { // Protocol 1
        m_wbBuffers = 31; // 32 packets * 1024 bytes = 32768
    } else {
        m_wbBuffers = val - 1;
    }
}
