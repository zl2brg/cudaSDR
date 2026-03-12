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

    int newBufferSize = 16 * 1024;

	if (m_manualBufferSize) {
		newBufferSize = m_socketBufferSize * 1024;
	}
	else {
		if (io->samplerate == 384000) newBufferSize = 128*1024;
		else if (io->samplerate == 192000) newBufferSize = 64*1024;
		else if (io->samplerate == 96000) newBufferSize = 32*1024;
		else if (io->samplerate == 48000) newBufferSize = 16*1024;
	}

    // If the simulator is running on the same Linux machine it will be
    // discovered at the machine's own LAN IP (or 127.0.0.1).  In either case
    // both the app and the simulator are bound to port 1024 with SO_REUSEADDR.
    // When the app sends a unicast command to its own IP:1024, Linux delivers
    // the datagram back to whichever socket wins the tie-break -- usually the
    // app's own socket -- so the simulator never receives init frames or the
    // start command (only the broadcast discovery reaches it, because broadcast
    // goes to ALL matching sockets).
    //
    // Fix: for local devices bind to an ephemeral port (port 0).  The OS picks
    // a free port and our outgoing packets have a different source port than
    // 1024.  The simulator records the source (IP:ephemeral) of the start-stop
    // command and streams IQ data back to that same ephemeral port, so we still
    // receive everything -- with no port 1024 collision.
    //
    // For genuine remote devices keep the classic LAN-IP:1024 binding.
    const bool deviceIsLocal =
        io->hpsdrDeviceIPAddress.isLoopback() ||
        QNetworkInterface::allAddresses().contains(io->hpsdrDeviceIPAddress);

    const QHostAddress bindAddr = deviceIsLocal
        ? QHostAddress(QHostAddress::AnyIPv4)
        : QHostAddress(set->getHPSDRDeviceLocalAddr());

    // TODO(P2-MULTI-RX): getRequiredPorts() currently returns a fixed list
    // ending with port 1035 (DDC0 source port) regardless of the number of
    // receivers.  For N receivers the hardware streams DDC0 from port 1035,
    // DDC1 from port 1036, ..., DDC(N-1) from port 1034+N.  All these ports
    // must be added to 'ports' after discovery reports num_DDCs, and a socket
    // must be opened (and readDatagram /readDeviceData connected) for each.
    // The CProtocol2::getRequiredPorts() method must be updated to return the
    // dynamic port list once the device's DDC count is known.
    for (quint16 port : ports) {
        if (m_sockets.contains(port)) continue;

        // For local devices use ephemeral port 0 so our packets don't share
        // port 1024 with the simulator socket (see comment above).
        const quint16 bindPort = deviceIsLocal ? 0 : port;

        QUdpSocket* socket = new QUdpSocket();
        if (socket->bind(bindAddr,
                                 bindPort,
                                 QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress))
        {
#if defined(Q_OS_WIN32)
            ::setsockopt(socket->socketDescriptor(), SOL_SOCKET, SO_RCVBUF, (char *)&newBufferSize, sizeof(newBufferSize));
#endif
            connect(socket, &QUdpSocket::errorOccurred, this, &DataIO::displayDataReceiverSocketError);
            connect(socket, &QUdpSocket::readyRead, this, &DataIO::readDeviceData);

            m_sockets[port] = socket;
            if (port == ports.first()) m_dataIOSocket = socket;

            io->networkIOMutex.lock();
            DATAIO_DEBUG << "data receiver socket bound successful to local port "
                         << socket->localPort() << " (logical port " << port << ")";
            io->networkIOMutex.unlock();
        } else {
            io->networkIOMutex.lock();
            DATAIO_DEBUG << "data receiver socket binding failed for port " << port;
            io->networkIOMutex.unlock();
            delete socket;
        }
    }

    if (m_dataIOSocket) {
        m_dataIOSocketOn = true;
        set->setPacketLoss(1);
    } else {
        m_dataIOSocketOn = false;
        io->networkIOMutex.lock();
        DATAIO_DEBUG << "Warning: No data receiver socket could be bound.";
        io->networkIOMutex.unlock();
    }
}


void DataIO::new_readDeviceData() {
    QUdpSocket* socket = qobject_cast<QUdpSocket*>(sender());
    if (!socket) return;

    qint64  size = 0;
    while (socket->hasPendingDatagrams()) {
        QMutexLocker locker(&io->networkIOMutex);
        QHostAddress senderAddress;
        quint16 senderPort = 0;
        size = socket->readDatagram((char *)m_buffer, sizeof(m_buffer), &senderAddress, &senderPort);
        if (io->protocol && io->protocol->isPacketValid(m_buffer, size)) {
            int type = io->protocol->getPacketType(m_buffer);
            if (type == 0x06) {
                static int newIqCount = 0;
                if (++newIqCount % 1000 == 0) {
                    DATAIO_DEBUG << "P2: [new] Received 1000 IQ packets on port " << socket->localPort() << " size " << size;
                }

                m_sequence = io->protocol->getSequence(m_buffer);

                if (m_sequence != m_oldSequence + 1) {
                    if (m_packetLossTime.elapsed() > 100) {
                        set->setPacketLoss(2);
                        m_packetLossTime.restart();
                    }
                }

                m_oldSequence = m_sequence;

                if (!io->iq_queue.isFull()) {
                    // P2 multi-RX: DDC0 sends from source port 1035, DDC1 from 1036, etc.
                    const int hdrSize = io->protocol->getHeaderSize();
                    if (hdrSize == 16) { // Protocol 2
                        int ddcIdx = (senderPort >= 1035) ? (int)(senderPort - 1035) : 0;
                        QByteArray payload(1, (char)(unsigned char)ddcIdx);
                        payload.append(QByteArray((const char *)&m_buffer[hdrSize], size - hdrSize));
                        io->iq_queue.enqueue(payload);
                    } else {
                        io->iq_queue.enqueue(QByteArray((const char *)&m_buffer[io->protocol->getHeaderSize()], size - io->protocol->getHeaderSize()));
                    }
                    emit (readydata());
                }
            }
            else if (type == 0x05) { // High Priority Status (Protocol 2)
                DATAIO_DEBUG << "P2: [new] Received High Priority Status packet on port " << socket->localPort() << " size " << size;
                io->protocol->decodeCCBytes(QByteArray((const char*)m_buffer, size), io);
            }
            else if (type == 0x04) { // wide band data

                m_sequenceWideBand = io->protocol->getSequence(m_buffer);

                if (m_sequenceWideBand != m_oldSequenceWideBand + 1) {
                    DATAIO_DEBUG << "wideband readData missed " << m_sequenceWideBand - m_oldSequenceWideBand << " packages.";
                    if (m_packetLossTime.elapsed() > 100) {
                        set->setPacketLoss(2);
                        m_packetLossTime.restart();
                    }
                }

                m_oldSequenceWideBand = m_sequenceWideBand;

                // three 'if's from KISS Konsole
                if ((m_wbBuffers & (m_sequenceWideBand & 0xFF)) == 0)
                {
                    m_sendEP4 = true;
                    m_wbCount = 0;
                }

                if (m_sendEP4)
                {
                    io->wb_queue.enqueue(QByteArray((const char *)&m_buffer[io->protocol->getHeaderSize()], size - io->protocol->getHeaderSize()));
                }
                if (m_wbCount++ == m_wbBuffers)
                {
                    m_sendEP4 = false;
                }
            }
        }
    }
}

void DataIO::readDeviceData() {
    QUdpSocket* socket = qobject_cast<QUdpSocket*>(sender());
    if (!socket) return;

	while (socket->hasPendingDatagrams()) {
		QMutexLocker locker(&io->networkIOMutex);
        QHostAddress senderAddress;
        quint16 senderPort = 0;
        qint64 size = socket->readDatagram(m_datagram.data(), m_datagram.size(), &senderAddress, &senderPort);
		if (io->protocol && io->protocol->isPacketValid((const unsigned char*)m_datagram.data(), size)) {
            int type = io->protocol->getPacketType((const unsigned char*)m_datagram.data());
			if (type == 0x06) {
                static int iqCount = 0;
                if (++iqCount % 1000 == 0) {
                    DATAIO_DEBUG << "P2: Received 1000 IQ packets on port " << socket->localPort() << " size " << size;
                }

				m_sequence = io->protocol->getSequence((const unsigned char*)m_datagram.data());

				if (m_sequence != m_oldSequence + 1) {
					if (m_packetLossTime.elapsed() > 100) {
						set->setPacketLoss(2);
						m_packetLossTime.restart();
					}
				}

				m_oldSequence = m_sequence;

                if (!io->iq_queue.isFull()) {
                    // P2 multi-RX: prepend DDC receiver index so CProtocol2::processInputBuffer()
                    // can route to the correct RX.  Each DDC sends from a fixed source port:
                    // DDC0 → 1035, DDC1 → 1036, etc.  This works for both local (simulator)
                    // and remote hardware because the source port is always DDC-specific.
                    const int hdrSize = io->protocol->getHeaderSize();
                    if (hdrSize == 16) { // Protocol 2
                        int ddcIdx = (senderPort >= 1035) ? (int)(senderPort - 1035) : 0;
                        QByteArray payload(1, (char)(unsigned char)ddcIdx);
                        payload.append(m_datagram.mid(hdrSize, size - hdrSize));
                        io->iq_queue.enqueue(payload);
                    } else {
                        io->iq_queue.enqueue(m_datagram.mid(io->protocol->getHeaderSize(), size - io->protocol->getHeaderSize()));
                    }
                    emit (readydata());
                }
            }
            else if (type == 0x05) { // High Priority Status (Protocol 2)
                DATAIO_DEBUG << "P2: Received High Priority Status packet on port " << socket->localPort() << " size " << size;
                io->protocol->decodeCCBytes(m_datagram.left(size), io);
            }
            else if (type == 0x04) { // wide band data
				m_sequenceWideBand = io->protocol->getSequence((const unsigned char*)m_datagram.data());

				if (m_sequenceWideBand != m_oldSequenceWideBand + 1) {
					DATAIO_DEBUG << "wideband readData missed " << m_sequenceWideBand - m_oldSequenceWideBand << " packages.";
					if (m_packetLossTime.elapsed() > 100) {
					    set->setPacketLoss(2);
					    m_packetLossTime.restart();
					}
				}
				
				m_oldSequenceWideBand = m_sequenceWideBand;

				// three 'if's from KISS Konsole
				if ((m_wbBuffers & (m_sequenceWideBand & 0xFF)) == 0)
				{						
					m_sendEP4 = true;
					m_wbCount = 0;
                    m_wbDatagram.resize(0);
				}

				if (m_sendEP4)
				{
					m_wbDatagram.append(m_datagram.mid(io->protocol->getHeaderSize(), size - io->protocol->getHeaderSize()));
				    if (m_wbCount++ == m_wbBuffers)
				    {
					    // enqueue
					    m_sendEP4 = false;
					    io->wb_queue.enqueue(m_wbDatagram);
					    m_wbDatagram.resize(0);
				    }
                }
			}
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
        const bool p2Start = (value != 0) && (io->protocol->getHeaderSize() == 16);

        if (p2Start) {
            // Re-issue P2 init right before start so DDC/port config is fresh
            // even if an earlier init was missed during thread/socket bring-up.
            sendInitFramesToNetworkDevice(0);

            // Also push one deterministic control burst immediately so DDC enable
            // and RX frequency are applied before/with Run=1.
            unsigned char p2CmdBuf[1444];
            int startupState = 0;
            for (int i = 0; i < 4; ++i) {
                quint16 ctlPort = DEVICE_PORT;
                memset(p2CmdBuf, 0, sizeof(p2CmdBuf));
                io->protocol->encodeCCBytes(p2CmdBuf, io, startupState, ctlPort);
                const int sendSize = (ctlPort == 1025 || ctlPort == 1027) ? 1444 : 60;
                m_dataIOSocket->writeDatagram((const char*)p2CmdBuf, sendSize, metis.ip_address, ctlPort);
                SleeperThread::msleep(2);
            }

            SleeperThread::msleep(15);
        }

		if (m_dataIOSocket->writeDatagram(m_commandDatagram, metis.ip_address, port) == m_commandDatagram.size()) {

            // Some P2 devices/simulators intermittently miss the first run command
            // during bring-up; a single delayed resend improves startup reliability.
            if (p2Start) {
                SleeperThread::msleep(20);
                m_dataIOSocket->writeDatagram(m_commandDatagram, metis.ip_address, port);
            }

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
    if (io->protocol->getHeaderSize() != 8) {
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
#endif
	io->networkIOMutex.unlock();
}

void DataIO::setSampleRate(QObject *sender, int value) {

	Q_UNUSED(sender)

	int bufferSize = 16 * 1024;
	io->networkIOMutex.lock();
	switch (value) {
	
		case 48000:
			bufferSize = 16*1024;
			DATAIO_DEBUG << "socket buffer size set to 16 kB.";
			break;
			
		case 96000:
			bufferSize = 32*1024;
			DATAIO_DEBUG << "socket buffer size set to 32 kB.";
			break;
			
		case 192000:
			bufferSize = 64*1024;
			DATAIO_DEBUG << "socket buffer size set to 64 kB.";
			break;
			
		case 384000:
			bufferSize = 128*1024;
			DATAIO_DEBUG << "socket buffer size set to 128 kB.";
			break;

		default:
			DATAIO_DEBUG << "invalid sample rate !\n";
			break;
	}

#if defined(Q_OS_WIN32)
    for (auto socket : m_sockets) {
        if (socket && socket->isValid()) {
            ::setsockopt(socket->socketDescriptor(), SOL_SOCKET,
                         SO_RCVBUF, (char *)&bufferSize, sizeof(bufferSize));
        }
    }
#endif

	io->networkIOMutex.unlock();
}


void DataIO::set_wbBuffers(int val) {
    if (io->protocol && io->protocol->getHeaderSize() == 8) { // Protocol 1
        m_wbBuffers = 31; // 32 packets * 1024 bytes = 32768
    } else {
        m_wbBuffers = val - 1;
    }
}
