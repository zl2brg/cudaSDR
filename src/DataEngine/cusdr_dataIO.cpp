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
#include "IHPSDRProtocol.h"
#include "soundout.h"
#include <QNetworkInterface>

#if defined(Q_OS_WIN32)
#include <winsock2.h>
#endif
#include <iostream>
using namespace std;

#ifdef LOG_P2_NETWORK
#define P2_NET_DEBUG DATAIO_DEBUG
#else
#define P2_NET_DEBUG nullDebug()
#endif


DataIO::DataIO(THPSDRParameter *ioData)
	: QObject()
	, set(Settings::instance())
	, io(ioData)
    , m_dataIOSocket(nullptr)
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
	, m_packetsToggle(true)
	, m_firstFrame(true)
	, m_stopped(false)
{
	// Size the datagram buffer to the largest possible packet:
	// Protocol 1: 1032 bytes (METIS_DATA_SIZE)
	// Protocol 2: up to 1444 bytes (DDC IQ data packet)
	m_datagram.resize(1444);
	m_iqbuffer.resize(1024);
	m_wbDatagram.resize(0);
	m_twoFramesDatagram.resize(0);

	m_sendSequence = 0L;
	m_oldSendSequence = 0L;

	m_packetLossTime.start();

	  connect(set, &Settings::sampleRateChanged, 
            this, &DataIO::setSampleRate);

    connect(set, &Settings::manualSocketBufferChanged, 
            this, &DataIO::setManualSocketBufferSize);

    connect(set, &Settings::socketBufferSizeChanged, 
            this, &DataIO::setSocketBufferSize);

	m_message = "m_sendSequence = %1, bytes sent: %2";
#ifndef USE_INTERNAL_AUDIO
     m_pSoundCardOut = std::make_unique<CSoundOut>(this);

    //RRK pass -1 to get the systems "default" audio device
    m_pSoundCardOut->Start(-1, true, 48000, false);
    m_pSoundCardOut->SetVolume(80);
#endif
}

namespace {
constexpr int kProtocol1HeaderSize = 8;
constexpr int kProtocol2HeaderSize = 16;
constexpr int kPacketTypeP1IqPrimary = 0x06;
constexpr int kPacketTypeP1IqLoopback = 0x02;
constexpr int kPacketTypeWideband = 0x04;
constexpr int kPacketTypeP2HighPriorityStatus = 0x05;

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
    return protocol && protocol->getHeaderSize() == kProtocol2HeaderSize;
}

bool isProtocol1(IHPSDRProtocol* protocol) {
    return protocol && protocol->getHeaderSize() == kProtocol1HeaderSize;
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
    io->networkIOMutex.lock();
        m_stopped = true;
    io->networkIOMutex.unlock();

    if (m_pSoundCardOut) {
        SleeperThread::msleep(100);
        m_pSoundCardOut->Stop();
        m_pSoundCardOut.reset(); // Reset smart pointer instead of delete
    }
}

void DataIO::initDataReceiverSocket() {

    QList<quint16> ports = { DEVICE_PORT };
    if (io->protocol) {
        ports = io->protocol->getRequiredPorts();
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
        newBufferSize = rxSocketBufferSizeForRate(io->samplerate);
    }

    const bool sameHostProtocol2 = isProtocol2(io->protocol) && isLocalAddress(io->hpsdrDeviceIPAddress);
    const QHostAddress bindAddr = sameHostProtocol2
        ? QHostAddress(QHostAddress::AnyIPv4)
        : QHostAddress(set->getHPSDRDeviceLocalAddr());

    if (sameHostProtocol2) {
        // On the same host, the simulator binds 1024/1025/1027/1035+ itself.
        // For P2 it sends all traffic back to the source port of the General
        // Packet, so we must avoid binding 1024 locally and use one ephemeral
        // socket for both control and receive.
        ports = { DEVICE_PORT };
        DATAIO_DEBUG << "initDataReceiverSocket: same-host Protocol 2 detected; using one ephemeral socket";
    }

    for (quint16 port : ports) {
        const quint16 bindPort = sameHostProtocol2 ? 0 : port;
        QUdpSocket* socket = new QUdpSocket(this);
        if (socket->bind(bindAddr,
                         bindPort,
                         QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress))
        {
#if defined(Q_OS_WIN32)
            ::setsockopt(socket->socketDescriptor(), SOL_SOCKET, SO_RCVBUF, (char *)&newBufferSize, sizeof(newBufferSize));
#else
            socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, newBufferSize);
#endif
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
}


void DataIO::readDeviceData() {
    QUdpSocket* socket = qobject_cast<QUdpSocket*>(sender());
    if (!socket || !io->protocol) return;

    if (isProtocol2(io->protocol)) {
        readDeviceDataP2(socket);
    } else {
        readDeviceDataP1(socket);
    }
}

void DataIO::readDeviceDataP1(QUdpSocket* socket) {
    while (socket->hasPendingDatagrams()) {
        QMutexLocker locker(&io->networkIOMutex);
        QHostAddress senderAddress;
        quint16 senderPort = 0;
        qint64 size = socket->readDatagram(m_datagram.data(), m_datagram.size(), &senderAddress, &senderPort);
        if (!io->protocol || !io->protocol->isPacketValid((const unsigned char*)m_datagram.data(), size)) continue;

        int type = io->protocol->getPacketType((const unsigned char*)m_datagram.data());
        if (type == kPacketTypeP1IqPrimary || type == kPacketTypeP1IqLoopback) { // IQ data (P1 EP6 or EP2 loopback)
            m_sequence = io->protocol->getSequence((const unsigned char*)m_datagram.data());
            if (m_sequence != m_oldSequence + 1) {
                if (m_packetLossTime.elapsed() > 100) {
                    set->setPacketLoss(2);
                    m_packetLossTime.restart();
                }
            }
            m_oldSequence = m_sequence;

            if (!io->iq_queue.isFull()) {
                const int hdrSize = io->protocol->getHeaderSize();
                io->iq_queue.enqueue(TIQPacket(m_datagram.mid(hdrSize, size - hdrSize), 0));
                emit (readydata());
            }
        }
        else if (type == kPacketTypeWideband) {
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
        QMutexLocker locker(&io->networkIOMutex);
        QHostAddress senderAddress;
        quint16 senderPort = 0;
        qint64 size = socket->readDatagram(m_datagram.data(), m_datagram.size(), &senderAddress, &senderPort);
        ++p2DatagramsSeen;

        if ((p2DatagramsSeen % 500) == 1) {
            P2_NET_DEBUG << "P2 RX datagram: localPort=" << socket->localPort()
                         << " sender=" << senderAddress.toString()
                         << " senderPort=" << senderPort
                         << " size=" << size
                         << " total=" << p2DatagramsSeen;
        }

        if (!io->protocol || !io->protocol->isPacketValid((const unsigned char*)m_datagram.data(), size)) continue;

        // Protocol 2 simulator may source wideband packets from an ephemeral
        // UDP source port. Classify by packet size first, then by port.
        if (size == 1040) { // Wideband ADC packet: 16-byte header + 1024 payload
            ++p2WidePacketsSeen;
            if ((p2WidePacketsSeen % 100) == 1) {
                P2_NET_DEBUG << "P2 wideband: localPort=" << socket->localPort()
                             << " sender=" << senderAddress.toString()
                             << " senderPort=" << senderPort
                             << " wideTotal=" << p2WidePacketsSeen;
            }
            processWidebandPacket(size);
        }
        else if (size >= 1444) { // DDC IQ packet (typically 1444 bytes)
            ++p2IqPacketsSeen;
            m_sequence = io->protocol->getSequence((const unsigned char*)m_datagram.data());

            if (m_sequence != m_oldSequence + 1) {
                if (m_packetLossTime.elapsed() > 100) {
                    set->setPacketLoss(2);
                    m_packetLossTime.restart();
                }
            }
            m_oldSequence = m_sequence;

            if (!io->iq_queue.isFull()) {
                const int hdrSize = io->protocol->getHeaderSize();
                quint16 effectiveSourcePort = senderPort;
                if (effectiveSourcePort < 1035 || effectiveSourcePort >= (1035 + MAX_RECEIVERS)) {
                    effectiveSourcePort = m_socketLogicalPorts.value(socket, socket->localPort());
                }
                io->iq_queue.enqueue(TIQPacket(m_datagram.mid(hdrSize, size - hdrSize), effectiveSourcePort));

                if ((p2IqPacketsSeen % 500) == 1) {
                    P2_NET_DEBUG << "P2 IQ enqueue: localPort=" << socket->localPort()
                                 << " senderPort=" << senderPort
                                 << " effectiveSourcePort=" << effectiveSourcePort
                                 << " size=" << size
                                 << " hdr=" << hdrSize
                                 << " payload=" << (size - hdrSize)
                                 << " queueCount=" << io->iq_queue.count()
                                 << " iqTotal=" << p2IqPacketsSeen;
                }
                emit (readydata());
            } else {
                P2_NET_DEBUG << "P2 IQ queue FULL: localPort=" << socket->localPort()
                             << " senderPort=" << senderPort
                             << " size=" << size
                             << " iqTotal=" << p2IqPacketsSeen;
            }
        }
        else if (size == 60) { // High Priority Status (P2)
            ++p2HpPacketsSeen;
            if ((p2HpPacketsSeen % 100) == 1) {
                P2_NET_DEBUG << "P2 HP status: localPort=" << socket->localPort()
                             << " sender=" << senderAddress.toString()
                             << " senderPort=" << senderPort
                             << " hpTotal=" << p2HpPacketsSeen;
            }
            io->protocol->decodeCCBytes(m_datagram.left(size), io);
        }
        else {
            if ((p2DatagramsSeen % 500) == 1) {
                P2_NET_DEBUG << "P2 unclassified datagram: localPort=" << socket->localPort()
                             << " sender=" << senderAddress.toString()
                             << " senderPort=" << senderPort
                             << " size=" << size;
            }
        }
    }
}

void DataIO::processWidebandPacket(qint64 size) {
    if (!io->protocol) return;
    m_sequenceWideBand = io->protocol->getSequence((const unsigned char*)m_datagram.data());

    if (m_sequenceWideBand != m_oldSequenceWideBand + 1) {
        DATAIO_DEBUG << "wideband readData missed " << m_sequenceWideBand - m_oldSequenceWideBand << " packages.";
        if (m_packetLossTime.elapsed() > 100) {
            set->setPacketLoss(2);
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
        const int hdrSize = io->protocol->getHeaderSize();
        m_wbDatagram.append(m_datagram.mid(hdrSize, size - hdrSize));
        if (m_wbCount++ == m_wbBuffers) {
            m_sendEP4 = false;
            io->wb_queue.enqueue(m_wbDatagram);
            m_wbDatagram.resize(0);
        }
    }
}

void DataIO::readData() {

	qint64 length = io->inputBuffer.length();
	
	//int buffers = qRound(length/(2*BUFFER_SIZE));
	int buffers = qRound((float) length/128);

	DATAIO_DEBUG << "input buffer length " << length << " buffers " << buffers;
   // qDebug() << "input buffer length " << length << " buffers " << buffers;
	while (!m_stopped) {
	
		for (int i = 0; i < buffers; i++) {

			//io->data_queue.enqueue(io->inputBuffer.mid(i*2*BUFFER_SIZE, 2*BUFFER_SIZE));
			io->data_queue.enqueue(io->inputBuffer.mid(i*128, 128));
			if (m_stopped) break;
		}
	}
	m_stopped = false;
}

void DataIO::sendInitFramesToNetworkDevice(int rx) {

	if (!io->protocol || !m_dataIOSocket) return;
    quint16 port = DEVICE_PORT;
    QByteArray initDatagram = io->protocol->formatInitFrame(rx, io, port);

    // Protocol 2 returns an empty datagram for rx > 0 (config only needed once).
    if (initDatagram.isEmpty()) return;

	if (m_dataIOSocket->writeDatagram(initDatagram.data(), initDatagram.size(), io->hpsdrDeviceIPAddress, port) < 0) {

		io->networkIOMutex.lock();
		DATAIO_DEBUG << "error sending init data to device: " << qPrintable(m_dataIOSocket->errorString());
		io->networkIOMutex.unlock();
	}
	else {

		io->networkIOMutex.lock();
		DATAIO_DEBUG << "init frames sent to network device. " << rx << " port " << port;
		io->networkIOMutex.unlock();
	}
}

void DataIO::networkDeviceStartStop(char value) {

	TNetworkDevicecard metis = set->getCurrentMetisCard();

    if (io->protocol && m_dataIOSocket) {
        quint16 port = DEVICE_PORT;
		m_commandDatagram = io->protocol->formatStartStop(value, port);
		if (m_dataIOSocket->writeDatagram(m_commandDatagram, metis.ip_address, port) == m_commandDatagram.size()) {

			if (value != 0) {

				io->networkIOMutex.lock();
				DATAIO_DEBUG << "sent start command to device at: "<< qPrintable(metis.ip_address.toString()) << " port " << port;
				io->networkIOMutex.unlock();
				m_networkDeviceRunning = true;
			}
			else {

				io->networkIOMutex.lock();
				DATAIO_DEBUG << "sent stop command to device at: "<< qPrintable(metis.ip_address.toString()) << " port " << port;
				io->networkIOMutex.unlock();
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
#ifndef USE_INTERNAL_AUDIO
    if((m_stopped != true) && m_pSoundCardOut)
        m_pSoundCardOut->PutOutQueue(63, cbuf);
#endif
}

void DataIO::writeData() {

    if (!io->protocol || !m_dataIOSocket) return;

    // Protocol 2: formatOutputPacket returns the complete 1444-byte DUC IQ packet.
    // Send it in a single call to port 1029; bypass the P1 two-call toggle.
    if (isProtocol2(io->protocol)) {
        QByteArray ducPkt = io->protocol->formatOutputPacket(io->audioDatagram, m_sendSequence);
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

		m_outDatagram = io->protocol->formatOutputPacket(io->audioDatagram, m_sendSequence);
        m_setNetworkDeviceHeader = false;
    }
	else {

		m_outDatagram += io->audioDatagram;
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

void DataIO::setManualSocketBufferSize(QObject *sender, bool value) {

	Q_UNUSED (sender)

	m_manualBufferSize = value;
	int socketBufferSize = 1024 * set->getSocketBufferSize();

	io->networkIOMutex.lock();
    if (m_manualBufferSize) {
        DATAIO_DEBUG << "set data IO socket BufferSize to " << m_socketBufferSize;
    } else {
        DATAIO_DEBUG << "set data IO socket BufferSize to 32 kB.";
        socketBufferSize = 32 * 1024;
    }

#if defined(Q_OS_WIN32)
    for (auto socket : m_sockets) {
        if (socket && socket->isValid()) {
            ::setsockopt(socket->socketDescriptor(), SOL_SOCKET,
                         SO_RCVBUF, (char *)&socketBufferSize, sizeof(socketBufferSize));
        }
    }
#else
    for (auto socket : m_sockets) {
        if (socket && socket->isValid()) {
            socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, socketBufferSize);
        }
    }
#endif
	io->networkIOMutex.unlock();
}

void DataIO::setSocketBufferSize(QObject *sender, int value) {

	Q_UNUSED (sender)

	int socketBufferSize = value * 1024;
	DATAIO_DEBUG << "m_socketBufferSize = " << value;

	io->networkIOMutex.lock();
#if defined(Q_OS_WIN32)
    for (auto socket : m_sockets) {
        if (socket && socket->isValid()) {
            ::setsockopt(socket->socketDescriptor(), SOL_SOCKET,
                         SO_RCVBUF, (char *)&socketBufferSize, sizeof(socketBufferSize));
        }
    }
#else
    for (auto socket : m_sockets) {
        if (socket && socket->isValid()) {
            socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, socketBufferSize);
        }
    }
#endif
	io->networkIOMutex.unlock();
}

void DataIO::setSampleRate(QObject *sender, int value) {

	Q_UNUSED(sender)

    int bufferSize = rxSocketBufferSizeForRate(value);
	io->networkIOMutex.lock();
    DATAIO_DEBUG << "socket buffer size set to" << (bufferSize / 1024) << "kB for sample rate" << value;

#if defined(Q_OS_WIN32)
    for (auto socket : m_sockets) {
        if (socket && socket->isValid()) {
            ::setsockopt(socket->socketDescriptor(), SOL_SOCKET,
                         SO_RCVBUF, (char *)&bufferSize, sizeof(bufferSize));
        }
    }
#else
    for (auto socket : m_sockets) {
        if (socket && socket->isValid()) {
            socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, bufferSize);
        }
    }
#endif

	io->networkIOMutex.unlock();

#ifndef USE_INTERNAL_AUDIO
    // Reset the sound card output queue so stale samples from the old rate
    // don't cause choppy audio after the DSP channel is rebuilt.
    if (m_pSoundCardOut)
        m_pSoundCardOut->Reset();
#endif
}


void DataIO::set_wbBuffers(int val) {
    if (isProtocol1(io->protocol)) { // Protocol 1
        m_wbBuffers = 31; // 32 packets * 1024 bytes = 32768
    } else {
        m_wbBuffers = val - 1;
    }
}
