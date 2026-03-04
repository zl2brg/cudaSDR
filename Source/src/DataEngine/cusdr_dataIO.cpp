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
	m_datagram.resize(1032);
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

    for (quint16 port : ports) {
        if (m_sockets.contains(port)) continue;

        QUdpSocket* socket = new QUdpSocket();
        if (socket->bind(QHostAddress(set->getHPSDRDeviceLocalAddr()),
                                 port,
                                 QUdpSocket::DontShareAddress))
        {
#if defined(Q_OS_WIN32)
            ::setsockopt(socket->socketDescriptor(), SOL_SOCKET, SO_RCVBUF, (char *)&newBufferSize, sizeof(newBufferSize));
#endif
            connect(socket, &QUdpSocket::errorOccurred, this, &DataIO::displayDataReceiverSocketError);
            connect(socket, &QUdpSocket::readyRead, this, &DataIO::readDeviceData);

            m_sockets[port] = socket;
            if (port == ports.first()) m_dataIOSocket = socket;

            io->networkIOMutex.lock();
            DATAIO_DEBUG << "data receiver socket bound successful to local port " << port;
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
        size = socket->readDatagram((char *)m_buffer, sizeof(m_buffer));
        if (io->protocol && io->protocol->isPacketValid(m_buffer, size)) {
            int type = io->protocol->getPacketType(m_buffer);
            if (type == 0x06) {

                m_sequence = io->protocol->getSequence(m_buffer);

                if (m_sequence != m_oldSequence + 1) {
                    if (m_packetLossTime.elapsed() > 100) {
                        set->setPacketLoss(2);
                        m_packetLossTime.restart();
                    }
                }

                m_oldSequence = m_sequence;

                if (!io->iq_queue.isFull()) {
                    io->iq_queue.enqueue(QByteArray::fromRawData((const char *)&m_buffer[io->protocol->getHeaderSize()], 1024));
                    emit (readydata());
                }
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
                    io->wb_queue.enqueue(QByteArray::fromRawData((const char *)&m_buffer[io->protocol->getHeaderSize()], 1024));
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
        qint64 size = socket->readDatagram(m_datagram.data(), m_datagram.size());
		if (io->protocol && io->protocol->isPacketValid((const unsigned char*)m_datagram.data(), size)) {
            int type = io->protocol->getPacketType((const unsigned char*)m_datagram.data());
			if (type == 0x06) {

				m_sequence = io->protocol->getSequence((const unsigned char*)m_datagram.data());

				if (m_sequence != m_oldSequence + 1) {

					if (m_packetLossTime.elapsed() > 100) {
						
						set->setPacketLoss(2);
						m_packetLossTime.restart();
					}
				}

				m_oldSequence = m_sequence;

                if (!io->iq_queue.isFull()) {
					io->iq_queue.enqueue(m_datagram.mid(io->protocol->getHeaderSize(), BUFFER_SIZE));
					emit (readydata());
				}
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
					m_wbDatagram.append(m_datagram.mid(io->protocol->getHeaderSize(), BUFFER_SIZE));
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
	//RRK send audio bytes here
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
