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


DataIO::DataIO(THPSDRParameter *ioData)
	: QObject()
	, m_settings(Settings::instance())
	, io(ioData)
    , m_dataIOSocket(nullptr)
	, m_dataIOSocketOn(false)
	, m_networkDeviceRunning(false)
	, m_setNetworkDeviceHeader(true)
	, m_p2IqPacketCount(0)
	, m_sequence(0)
	, m_oldSequence(0xFFFFFFFF)
	, m_sequenceWideBand(0)
	, m_oldSequenceWideBand(0xFFFFFFFF)
	, m_wbBuffers(31)
	, m_wbCount(0)
	, m_socketBufferSize(m_settings->getSocketBufferSize())
	, m_sendEP4(false)
	, m_manualBufferSize(m_settings->getManualSocketBufferSize())
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

	  connect(m_settings, &Settings::sampleRateChanged, 
            this, &DataIO::setSampleRate);

    connect(m_settings, &Settings::manualSocketBufferChanged, 
            this, &DataIO::setManualSocketBufferSize);

    connect(m_settings, &Settings::socketBufferSizeChanged, 
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
        m_pSoundCardOut->Stop();
        m_pSoundCardOut.reset(); // Reset smart pointer instead of delete
    }
}

void DataIO::initDataReceiverSocket() {

    m_p2IqPacketCount = 0; // reset per-session IQ packet log counter

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
    m_logicalPorts.clear();
    m_oldP2IqSequenceByPort.clear();
    m_dataIOSocket = nullptr;

    int newBufferSize = 16 * 1024;

	if (m_manualBufferSize) {
		newBufferSize = m_socketBufferSize * 1024;
	}
    else {
        newBufferSize = rxSocketBufferSizeForRate(io->samplerate);
    }

    // Bind to AnyIPv4 to ensure we catch all packets destined for this port,
    // regardless of whether they were sent to a broadcast, local, or specific interface IP.
    // The IP filtering in readDeviceData ensures we only process data from the selected SDR.
    const QHostAddress bindAddr = QHostAddress::AnyIPv4;

    for (quint16 port : ports) {
        // For local devices use ephemeral port 0 so our packets don't share
        // port 1024 with the simulator socket.
        const quint16 bindPort = (port == DEVICE_PORT) ? 0 : port;

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
            m_logicalPorts[socket->localPort()] = port;
            if (port == ports.first()) m_dataIOSocket = socket;
            
            DATAIO_DEBUG << "Bound receiver socket to logical port " << port << " (localPort=" << socket->localPort() << ") with buffer size " << newBufferSize;
        } else {
            DATAIO_DEBUG << "Failed to bind receiver socket to port " << port << ": " << socket->errorString();
            delete socket;
        }
    }

	m_dataIOSocketOn = true;
}


void DataIO::readDeviceData() {
    QUdpSocket* socket = qobject_cast<QUdpSocket*>(sender());
    if (!socket || !io->protocol) return;

    if (io->protocol->getHeaderSize() == 16) {
        readDeviceDataP2(socket);
    } else {
        readDeviceDataP1(socket);
    }
}

void DataIO::readDeviceDataP1(QUdpSocket* socket) {
    bool hasData = false;
    while (socket->hasPendingDatagrams()) {
        if (m_stopped) break;
        QMutexLocker locker(&io->networkIOMutex);
        QHostAddress senderAddress;
        quint16 senderPort = 0;

        QByteArray datagram;
        datagram.resize(socket->pendingDatagramSize());
        qint64 size = socket->readDatagram(datagram.data(), datagram.size(), &senderAddress, &senderPort);

        if (size <= 0) continue;

        if (io->protocol->isPacketValid((const unsigned char*)datagram.data(), size)) {
            int type = io->protocol->getPacketType((const unsigned char*)datagram.data());

            if (type == 0x06 || type == 0x02) { // IQ data (P1 EP6 or EP2 loopback)
                m_sequence = io->protocol->getSequence((const unsigned char*)datagram.data());

                if (m_oldSequence != 0xFFFFFFFF && m_sequence != m_oldSequence + 1) {
                    if (m_packetLossTime.elapsed() > 100) {
                        m_settings->setPacketLoss(2);
                        m_packetLossTime.restart();
                    }
                }
                m_oldSequence = m_sequence;

                if (!io->iq_queue.isFull()) {
                    io->iq_queue.enqueue(datagram);
                    hasData = true;
                }
            }
            else if (type == 0x04) { // wide band data (P1)
                m_sequenceWideBand = io->protocol->getSequence((const unsigned char*)datagram.data());

                if (m_oldSequenceWideBand != 0xFFFFFFFF) {
                    int32_t diff = (int32_t)(m_sequenceWideBand - m_oldSequenceWideBand);
                    if (diff != 1 && (diff < 0 || diff > 50)) {
                        continue; 
                    }
                    if (diff != 1) {
                        if (m_packetLossTime.elapsed() > 100) {
                            DATAIO_DEBUG << "wideband packet loss: " << (diff - 1) << " packets";
                            m_settings->setPacketLoss(2);
                            m_packetLossTime.restart();
                        }
                    }
                }
                m_oldSequenceWideBand = m_sequenceWideBand;

                if ((m_wbBuffers & (m_sequenceWideBand & 0xFF)) == 0) {
                    m_sendEP4 = true;
                    m_wbCount = 0;
                    m_wbDatagram.resize(0);
                }

                if (m_sendEP4) {
                    m_wbDatagram.append(datagram.mid(io->protocol->getHeaderSize(), size - io->protocol->getHeaderSize()));
                    if (m_wbCount++ == m_wbBuffers) {
                        m_sendEP4 = false;
                        io->wb_queue.enqueue(m_wbDatagram);
                        m_wbDatagram.resize(0);
                    }
                }
            }
        }
    }
    if (hasData) emit readydata();
}

void DataIO::readDeviceDataP2(QUdpSocket* socket) {
    bool hasData = false;
    while (socket->hasPendingDatagrams()) {
        if (m_stopped) break;
        QMutexLocker locker(&io->networkIOMutex);
        QHostAddress senderAddress;
        quint16 senderPort = 0;

        QByteArray datagram;
        datagram.resize(socket->pendingDatagramSize());
        qint64 size = socket->readDatagram(datagram.data(), datagram.size(), &senderAddress, &senderPort);

        if (size <= 0) continue;

        if (io->protocol->isPacketValid((const unsigned char*)datagram.data(), size)) {
            int type = io->protocol->getPacketType((const unsigned char*)datagram.data());

            if (type == 0x06) { // IQ data (P2)
                m_sequence = io->protocol->getSequence((const unsigned char*)datagram.data());

                const quint16 seqKeyPort = senderPort;
                const bool hasOld = m_oldP2IqSequenceByPort.contains(seqKeyPort);
                const uint32_t oldSeq = hasOld ? m_oldP2IqSequenceByPort.value(seqKeyPort) : 0xFFFFFFFF;
                if (oldSeq != 0xFFFFFFFF && m_sequence != oldSeq + 1) {
                    if (m_packetLossTime.elapsed() > 100) {
                        m_settings->setPacketLoss(2);
                        m_packetLossTime.restart();
                    }
                }
                m_oldP2IqSequenceByPort[seqKeyPort] = m_sequence;

                if (!io->iq_queue.isFull()) {
                    const int hdrSize = io->protocol->getHeaderSize();
                    const quint16 localPort = socket->localPort();
                    const quint16 logicalPort = m_logicalPorts.value(localPort, localPort);
                    int ddcIdx = 0;
                    if (senderPort >= 1035) {
                        ddcIdx = (int)(senderPort - 1035);
                    } else if (logicalPort >= 1035) {
                        ddcIdx = (int)(logicalPort - 1035);
                    }
                    if (ddcIdx < 0 || ddcIdx >= MAX_RECEIVERS) ddcIdx = 0;
                    
                    QByteArray payload(1, (char)(unsigned char)ddcIdx);
                    payload.append(datagram.mid(hdrSize, (int)size - hdrSize));
                    io->iq_queue.enqueue(payload);
                    hasData = true;
                }
            }
            else if (type == 0x05) { // High Priority Status (P2)
                io->protocol->decodeCCBytes(datagram, io);
            }
            else if (type == 0x04) { // wide band data (P2)
                m_sequenceWideBand = io->protocol->getSequence((const unsigned char*)datagram.data());

                if (m_oldSequenceWideBand != 0xFFFFFFFF) {
                    int32_t diff = (int32_t)(m_sequenceWideBand - m_oldSequenceWideBand);
                    if (diff != 1 && (diff < 0 || diff > 50)) {
                        continue; 
                    }
                    if (diff != 1) {
                        if (m_packetLossTime.elapsed() > 100) {
                            DATAIO_DEBUG << "P2 wideband packet loss: " << (diff - 1) << " packets";
                            m_settings->setPacketLoss(2);
                            m_packetLossTime.restart();
                        }
                    }
                }
                m_oldSequenceWideBand = m_sequenceWideBand;

                if ((m_wbBuffers & (m_sequenceWideBand & 0xFF)) == 0) {
                    m_sendEP4 = true;
                    m_wbCount = 0;
                    m_wbDatagram.resize(0);
                }

                if (m_sendEP4) {
                    if (size >= 1024) {
                        m_wbDatagram.append(datagram.right(1024));
                        if (m_wbCount++ == m_wbBuffers) {
                            m_sendEP4 = false;
                            io->wb_queue.enqueue(m_wbDatagram);
                            m_wbDatagram.resize(0);
                        }
                    }
                }
            }
        }
    }
    if (hasData) emit readydata();
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

    if (io->protocol->getHeaderSize() == 16) {
        sendInitFramesToNetworkDeviceP2(rx);
    } else {
        sendInitFramesToNetworkDeviceP1(rx);
    }
}

void DataIO::sendInitFramesToNetworkDeviceP1(int rx) {
    quint16 port = DEVICE_PORT;
    QByteArray initDatagram = io->protocol->formatInitFrame(rx, io, port);
    if (initDatagram.isEmpty()) return;

    if (m_dataIOSocket->writeDatagram(initDatagram.data(), initDatagram.size(), io->hpsdrDeviceIPAddress, port) < 0) {
        io->networkIOMutex.lock();
        DATAIO_DEBUG << "P1 error sending init data to device: " << qPrintable(m_dataIOSocket->errorString());
        io->networkIOMutex.unlock();
    }
    else {
        io->networkIOMutex.lock();
        DATAIO_DEBUG << "P1 init frames sent to network device. " << rx << " port " << port;
        io->networkIOMutex.unlock();
    }
}

void DataIO::sendInitFramesToNetworkDeviceP2(int rx) {
    quint16 port = DEVICE_PORT;
    QByteArray initDatagram = io->protocol->formatInitFrame(rx, io, port);

    // Protocol 2 returns an empty datagram for rx > 0 (config only needed once).
    if (initDatagram.isEmpty()) return;

    // General Config (port 1024) must be exactly 60 bytes.
    const int sendSize = 60;
    if (m_dataIOSocket->writeDatagram(initDatagram.data(), sendSize, io->hpsdrDeviceIPAddress, port) < 0) {
        io->networkIOMutex.lock();
        DATAIO_DEBUG << "P2 error sending init data to device: " << qPrintable(m_dataIOSocket->errorString());
        io->networkIOMutex.unlock();
    }
    else {
        io->networkIOMutex.lock();
        DATAIO_DEBUG << "P2 init frames sent to network device. " << rx << " port " << port;
        io->networkIOMutex.unlock();
    }
}

void DataIO::networkDeviceStartStop(char value) {
    if (!io->protocol || !m_dataIOSocket) {
        DATAIO_DEBUG << "[NDS] SKIPPED: protocol or socket is null";
        return;
    }

    if (io->protocol->getHeaderSize() == 16) {
        networkDeviceStartStopP2(value);
    } else {
        networkDeviceStartStopP1(value);
    }
}

void DataIO::networkDeviceStartStopP1(char value) {
	TNetworkDevicecard metis = m_settings->getCurrentMetisCard();
    quint16 port = DEVICE_PORT;
    m_commandDatagram = io->protocol->formatStartStop(value, port);

    if (m_dataIOSocket->writeDatagram(m_commandDatagram, metis.ip_address, port) == m_commandDatagram.size()) {
        if (value != 0) {
            io->networkIOMutex.lock();
            DATAIO_DEBUG << "P1 sent start command to device at: "<< qPrintable(metis.ip_address.toString()) << " port " << port;
            io->networkIOMutex.unlock();
            m_networkDeviceRunning = true;
        }
        else {
            io->networkIOMutex.lock();
            DATAIO_DEBUG << "P1 sent stop command to device at: "<< qPrintable(metis.ip_address.toString()) << " port " << port;
            io->networkIOMutex.unlock();
            m_networkDeviceRunning = false;
        }
    }
    else {
        DATAIO_DEBUG << "P1 device start/stop: sending command to device failed.";
    }
}

void DataIO::networkDeviceStartStopP2(char value) {
	TNetworkDevicecard metis = m_settings->getCurrentMetisCard();
    quint16 port = DEVICE_PORT;
    const bool p2Start = (value != 0);
    if (p2Start) { io->ccTx.mox = false; io->ccTx.ptt = false; }

    if (p2Start) {
        DATAIO_DEBUG << "[P2-START] receivers=" << io->receivers
                     << " device=" << qPrintable(metis.ip_address.toString());
        // Re-issue P2 init right before start so DDC/port config is fresh
        sendInitFramesToNetworkDeviceP2(0);

        // Flush any stale data from the network stack before starting
        flushNetworkStack();

        // Also push one deterministic control burst immediately so DDC enable
        // and RX frequency are applied before/with Run=1.
        unsigned char p2CmdBuf[1444];
        int startupState = 0;
        for (int i = 0; i < 4; ++i) {
            quint16 ctlPort = DEVICE_PORT;
            memset(p2CmdBuf, 0, sizeof(p2CmdBuf));
            io->protocol->encodeCCBytes(p2CmdBuf, io, startupState, ctlPort);
            // Per hpsdrsim requirements and V4.3 spec:
            //   port 1025 (DDC Specific) : 1444 bytes
            //   port 1027 (HP Data)      : 1444 bytes
            //   other ports (1024, 1026) :   60 bytes
            const int sendSize = (ctlPort == 1025 || ctlPort == 1027) ? 1444 : 60;
            qint64 sent = m_dataIOSocket->writeDatagram((const char*)p2CmdBuf, sendSize, metis.ip_address, ctlPort);
            if (sent < 0)
                DATAIO_DEBUG << "[P2-START] preburst[" << i << "] port" << ctlPort << "FAILED:" << m_dataIOSocket->errorString();
            else {
                if (ctlPort == 1025)
                    DATAIO_DEBUG << "[P2-START] preburst[" << i
                                 << "] DDC Specific -> port 1025, ddcEnableMask=0x"
                                 << QByteArray(1, (char)p2CmdBuf[7]).toHex().constData();
                else
                    DATAIO_DEBUG << "[P2-START] preburst[" << i << "] -> port" << ctlPort << "sent" << sent << "bytes";
            }
            SleeperThread::msleep(2);
        }

        // The burst already sent the start command (Case 3) as its last packet.
        // We m_settings m_commandDatagram here for consistency and for the resend below.
        m_commandDatagram = io->protocol->formatStartStop(value, port);
        SleeperThread::msleep(15);
    } else {
        m_commandDatagram = io->protocol->formatStartStop(value, port);
        if (m_dataIOSocket->writeDatagram(m_commandDatagram, metis.ip_address, port) != m_commandDatagram.size()) {
             DATAIO_DEBUG << "P2 stop command failed.";
        }
    }

    // For Start, we've already sent one Run=1 packet in the burst.
    // We send one more (the "official" start) plus a delayed resend for reliability.
    if (p2Start) {
        if (m_dataIOSocket->writeDatagram(m_commandDatagram, metis.ip_address, port) == m_commandDatagram.size()) {
            SleeperThread::msleep(20);
            // Regenerate for resend to get a fresh sequence number
            m_commandDatagram = io->protocol->formatStartStop(value, port);
            m_dataIOSocket->writeDatagram(m_commandDatagram, metis.ip_address, port);

            io->networkIOMutex.lock();
            DATAIO_DEBUG << "P2 sent start command to device at: "<< qPrintable(metis.ip_address.toString()) << " port " << port;
            io->networkIOMutex.unlock();
            m_networkDeviceRunning = true;
        } else {
            DATAIO_DEBUG << "P2 start command failed.";
        }
    } else {
        io->networkIOMutex.lock();
        DATAIO_DEBUG << "P2 sent stop command to device at: "<< qPrintable(metis.ip_address.toString()) << " port " << port;
        io->networkIOMutex.unlock();
        m_networkDeviceRunning = false;

        // Flush any pending data from the network stack
        flushNetworkStack();
        DATAIO_DEBUG << "P2 flushed network stack after stop command.";
    }
}

void DataIO::flushNetworkStack() {
    for (QUdpSocket* socket : m_sockets.values()) {
        while (socket->hasPendingDatagrams()) {
            socket->readDatagram((char *)m_buffer, sizeof(m_buffer));
        }
    }
}

void DataIO::sendAudio(u_char *buf) {
	// Standard HPSDR Protocol 1 TX frame layout per V1.59 PDF:
	//   Bytes 0-7:   Header (SYNC, SYNC, SYNC, C0..C4)
	//   Bytes 8-511: 63 blocks of 8 bytes each
	// Each 8-byte block:
	//   Offset 0-1: Left Audio monitor (16-bit)
	//   Offset 2-3: Right Audio monitor (16-bit)
	//   Offset 4-5: I TX (16-bit)
	//   Offset 6-7: Q TX (16-bit)
	static TYPECPX cbuf[252];
	int i, j;
	short sample;

	for(i = 8, j = 0; i < 512; i += 8, j++) {
		// Extract Left/Right monitor audio from the first 4 bytes of each 8-byte group.
		sample = buf[i] << 8 | buf[i+1]; // left
		cbuf[j].re = (double)sample;
		sample = buf[i+2] << 8 | buf[i+3]; // right
		cbuf[j].im = (double)sample;
	}
#ifndef USE_INTERNAL_AUDIO
    if((m_stopped != true) && m_pSoundCardOut)
        m_pSoundCardOut->PutOutQueue(63, cbuf);
#endif
}

void DataIO::writeData() {
    if (!io->protocol || !m_dataIOSocket) return;

    if (io->protocol->getHeaderSize() == 16) {
        writeDataP2();
    } else {
        writeDataP1();
    }
}

void DataIO::writeDataP1() {
    // Protocol 1 two-call toggle: first call stores the Metis header,
    // second call appends audio and sends the 1032-byte packet.
    if (io->hpsdrDeviceIPAddress.isNull()) return;

	if (m_setNetworkDeviceHeader) {
		m_outDatagram = io->protocol->formatOutputPacket(io->audioDatagram, m_sendSequence);
        m_setNetworkDeviceHeader = false;
    }
	else {
		m_outDatagram += io->audioDatagram;
		if (m_dataIOSocket->writeDatagram(m_outDatagram, m_settings->getCurrentMetisCard().ip_address, DEVICE_PORT) < 0) {
			DATAIO_DEBUG << "P1 error sending data to device: " << m_dataIOSocket->errorString();
		}

		if (m_oldSendSequence != 0 && m_sendSequence != m_oldSendSequence + 1) {
			DATAIO_DEBUG << "P1 output sequence error: old =" << m_oldSendSequence << "; new =" << m_sendSequence;
		}

		m_oldSendSequence = m_sendSequence;
		m_setNetworkDeviceHeader = true;
    }
}

void DataIO::writeDataP2() {
    // Protocol 2: formatOutputPacket returns the complete 1444-byte DUC IQ packet.
    // Send it in a single call to port 1029; bypass the P1 two-call toggle.
    QByteArray ducPkt = io->protocol->formatOutputPacket(io->audioDatagram, m_sendSequence);
    static const quint16 DUC_PORT = 1029;
    if (m_dataIOSocket->writeDatagram(ducPkt,
                                      m_settings->getCurrentMetisCard().ip_address,
                                      DUC_PORT) < 0) {
        DATAIO_DEBUG << "P2 TX: error sending DUC IQ: " << m_dataIOSocket->errorString();
    }
    m_oldSendSequence = m_sendSequence - 1; // keep tracking consistent
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
	int socketBufferSize = 1024 * m_settings->getSocketBufferSize();

	io->networkIOMutex.lock();
    if (m_manualBufferSize) {
        DATAIO_DEBUG << "m_settings data IO socket BufferSize to " << m_socketBufferSize;
    } else {
        DATAIO_DEBUG << "m_settings data IO socket BufferSize to 32 kB.";
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
    DATAIO_DEBUG << "socket buffer size m_settings to" << (bufferSize / 1024) << "kB for sample rate" << value;

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
    if (io->protocol && io->protocol->getHeaderSize() == 8) { // Protocol 1
        m_wbBuffers = 31; // 32 packets * 1024 bytes = 32768
    } else {
        m_wbBuffers = val - 1;
    }
}
