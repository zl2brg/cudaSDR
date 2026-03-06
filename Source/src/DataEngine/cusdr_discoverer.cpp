/**
* @file  cusdr_discoverer.cpp
* @brief HPSDR device discoverer class
* @author Hermann von Hasseln, DL3HVH
* @version 0.1
* @date 2012-05-19
*/

/*
 *   
 *   Copyright 2012 Hermann von Hasseln, DL3HVH
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

#define LOG_DISCOVERER

#include "cusdr_discoverer.h"
#include "Util/cusdr_buttons.h"


//#include <QComboBox>
//#include <QDialogButtonBox>
//#include <QLabel>
//#include <QPushButton>
//#include <QVBoxLayout>
//#include <QCheckBox>
//#include <QSlider>
//#include <QSpinBox>



//#define	btn_height		18
//#define	btn_width		74

Discoverer::Discoverer(THPSDRParameter *ioData)
    : QObject()
	, set(Settings::instance())
	, io(ioData)
{
	m_deviceCards = set->getMetisCardsList();
}

Discoverer::~Discoverer() {
}

TNetworkDevicecard mc;

void Discoverer::initHPSDRDevice() {

	m_searchTime.start();

	int deviceNo = 0;
	while (deviceNo == 0) {

		deviceNo = findHPSDRDevices();

		if (deviceNo > 1) {
			
			set->setHPSDRDeviceNumber(deviceNo);
			break;
		}

		if (deviceNo > 0) {

			set->setHPSDRDeviceNumber(deviceNo);
			break;
		}

		if (m_searchTime.elapsed() > 1000) {

			set->setHPSDRDeviceNumber(0);
			break;
		}

		io->networkIOMutex.lock();
		DISCOVERER_DEBUG << "no device found - trying again...";
		io->networkIOMutex.unlock();
	}

	io->networkIOMutex.lock();
	io->devicefound.wakeAll();
	io->networkIOMutex.unlock();
}

int Discoverer::findHPSDRDevices() {

	int devicesFound = 0;
    m_deviceCards.clear();

    // Protocol 1 discovery packet: EF FE 02 00...00  (63 bytes)
    // Protocol 2 discovery packet: 00 00 00 00 02 00...00  (60 bytes)
    // Both are broadcast to port 1024.  We send them sequentially on the same
    // socket and collect all responses before parsing.

	m_findDatagram.resize(63);
    m_findDatagram[0] = (char)0xEF;
    m_findDatagram[1] = (char)0xFE;
    m_findDatagram[2] = (char)0x02;
	for (int i = 3; i < 63; i++)
		m_findDatagram[i] = (char)0x00;

    QByteArray p2FindDatagram(60, 0x00);
    p2FindDatagram[4] = (char)0x02; // Protocol 2 discovery command

	QUdpSocket socket;

/*	CHECKED_CONNECT(
		&socket,
        SIGNAL(connect(QAbstractSocket::errorOccurred)),
		this,
		SLOT(displayDiscoverySocketError(QAbstractSocket::SocketError)));

*/
    connect(&socket, &QAbstractSocket::errorOccurred,
            this, &Discoverer::displayDiscoverySocketError);
	io->networkIOMutex.lock();
	DISCOVERER_DEBUG << "using " << qPrintable(QHostAddress(set->getHPSDRDeviceLocalAddr()).toString()) << " for discovery.";
	io->networkIOMutex.unlock();

	// clear comboBox entries in the network dialogue
	set->clearNetworkIOComboBoxEntry();

#if defined(Q_OS_WIN32)

	if (socket.bind(
				QHostAddress(set->getHPSDRDeviceLocalAddr()), 0,
				QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress))
				//QUdpSocket::ReuseAddressHint))
	{
		set->setMetisPort(this, socket.localPort());
		io->networkIOMutex.lock();
		DISCOVERER_DEBUG << "discovery_socket bound successfully to port " << socket.localPort();
		io->networkIOMutex.unlock();
	}
	else {

		io->networkIOMutex.lock();
		DISCOVERER_DEBUG << "discovery_socket bind failed.";
		io->networkIOMutex.unlock();

		socket.close();
		return 0;
	}
#elif defined(Q_OS_LINUX)

	if (socket.bind(
				QHostAddress(set->getHPSDRDeviceLocalAddr()),
				QUdpSocket::DefaultForPlatform))
	{
        connect(&socket, &QAbstractSocket::errorOccurred,
                this, &Discoverer::displayDiscoverySocketError);;

		set->setMetisPort(this, socket.localPort());
		io->networkIOMutex.lock();
		DISCOVERER_DEBUG << "discovery_socket bound successfully to port " << socket.localPort();
		io->networkIOMutex.unlock();
	}
	else {
		
		io->networkIOMutex.lock();
		DISCOVERER_DEBUG << "discovery_socket bind failed.";
		io->networkIOMutex.unlock();

		socket.close();
		return 0;
	}
#endif

	if (socket.writeDatagram(m_findDatagram, QHostAddress::Broadcast, DEVICE_PORT) == 63) {

		io->networkIOMutex.lock();
		DISCOVERER_DEBUG << "Protocol 1 discovery data sent.";
		io->networkIOMutex.unlock();
	}
	else {

		io->networkIOMutex.lock();
		DISCOVERER_DEBUG << "Protocol 1 discovery data not sent.";
		io->networkIOMutex.unlock();
	}

	if (socket.writeDatagram(p2FindDatagram, QHostAddress::Broadcast, DEVICE_PORT) == 60) {

		io->networkIOMutex.lock();
		DISCOVERER_DEBUG << "Protocol 2 discovery data sent.";
		io->networkIOMutex.unlock();
	}
	else {

		io->networkIOMutex.lock();
		DISCOVERER_DEBUG << "Protocol 2 discovery data not sent.";
		io->networkIOMutex.unlock();
	}


	// wait a little
	//SleeperThread::msleep(30);
	SleeperThread::msleep(500);

	while (socket.hasPendingDatagrams()) {

		quint16 port;
        // Reset device card structure for each device found
        mc = TNetworkDevicecard();
			
		m_deviceDatagram.resize(socket.pendingDatagramSize());
		socket.readDatagram(m_deviceDatagram.data(), m_deviceDatagram.size(), &mc.ip_address, &port);

		// ---- Protocol 1 response: EF FE 02/03 + MAC + firmware + boardID ----
		if (m_deviceDatagram.size() >= 11 &&
			m_deviceDatagram[0] == (char)0xEF && m_deviceDatagram[1] == (char)0xFE)
		{
			if (m_deviceDatagram[2] == (char)0x02) {

				sprintf(mc.mac_address, "%02X:%02X:%02X:%02X:%02X:%02X",
					m_deviceDatagram[3] & 0xFF, m_deviceDatagram[4] & 0xFF, m_deviceDatagram[5] & 0xFF,
					m_deviceDatagram[6] & 0xFF, m_deviceDatagram[7] & 0xFF, m_deviceDatagram[8] & 0xFF);

				io->networkIOMutex.lock();
				DISCOVERER_DEBUG << "[P1] Device found at " << qPrintable(mc.ip_address.toString()) << ":" << port << "; Mac addr: [" << mc.mac_address << "]";
				DISCOVERER_DEBUG << "[P1] Device code version: " << qPrintable(QString::number(m_deviceDatagram.at(9), 16));
				io->networkIOMutex.unlock();

				mc.protocol = 1; // Always Protocol 1 for EF FE responses
                mc.status = 0x02;

				if (m_deviceDatagram.size() >= 10) {
					int version = (unsigned char)m_deviceDatagram.at(9);
					int boardId = (unsigned char)m_deviceDatagram.at(10);
                    mc.sw_version = version;
					if (boardId == 1)
						set->setHermesVersion(version);
					else if (boardId == 0)
						set->setMetisVersion(version);
				}

				int no = (unsigned char)m_deviceDatagram.at(10);
				devicesFound += addDevice(mc, no, 1);
			}
			else if (m_deviceDatagram[2] == (char)0x03) {

				io->networkIOMutex.lock();
				DISCOVERER_DEBUG << "[P1] Device already sending data - trying to shut down...";
				io->networkIOMutex.unlock();

				shutdownHPSDRDevice();
				clear();
			}
		}
		// ---- Protocol 2 response: 00 00 00 00 02/03 + MAC + ... ----
		// Bytes: [0-3]=seq(0), [4]=status, [5-10]=MAC, [11]=device, [12]=res, [13]=firmware, [14]=receivers, [15]=transmitters
		else if (m_deviceDatagram.size() >= 14 &&
				 m_deviceDatagram[0] == 0x00 && m_deviceDatagram[1] == 0x00 &&
				 m_deviceDatagram[2] == 0x00 && m_deviceDatagram[3] == 0x00)
		{
			int status = (unsigned char)m_deviceDatagram.at(4);
			if (status == 0x02 || status == 0x03) {

				sprintf(mc.mac_address, "%02X:%02X:%02X:%02X:%02X:%02X",
					m_deviceDatagram[5] & 0xFF, m_deviceDatagram[6] & 0xFF, m_deviceDatagram[7] & 0xFF,
					m_deviceDatagram[8] & 0xFF, m_deviceDatagram[9] & 0xFF, m_deviceDatagram[10] & 0xFF);

				int no      = (unsigned char)m_deviceDatagram.at(11);
				int version = (unsigned char)m_deviceDatagram.at(13);
                int num_ddcs = (m_deviceDatagram.size() >= 15) ? (unsigned char)m_deviceDatagram.at(14) : 1;
                int num_dacs = (m_deviceDatagram.size() >= 16) ? (unsigned char)m_deviceDatagram.at(15) : 1;

				io->networkIOMutex.lock();
				DISCOVERER_DEBUG << "[P2] Device found at " << qPrintable(mc.ip_address.toString()) << ":" << port 
                                 << "; Mac: [" << mc.mac_address << "] board=" << no << " fw=" << version 
                                 << " receivers=" << num_ddcs;
				io->networkIOMutex.unlock();

                mc.sw_version = version;
                mc.status = status;
                mc.max_receivers = num_ddcs;
                mc.max_transmitters = num_dacs;
                mc.adcs = num_ddcs;
                mc.dacs = num_dacs;

				set->setHermesVersion(version); // Most P2 devices are Hermes-class
				devicesFound += addDevice(mc, no, 2);

				if (status == 0x03) {
					io->networkIOMutex.lock();
					DISCOVERER_DEBUG << "[P2] Device already running.";
					io->networkIOMutex.unlock();
				}
			}
		}
	}
	set->setMetisCardList(m_deviceCards);

	if (devicesFound == 1) {

		set->setCurrentHPSDRDevice(m_deviceCards.at(0));
		io->networkIOMutex.lock();
		DISCOVERER_DEBUG << "Device selected: " << qPrintable(m_deviceCards.at(0).ip_address.toString());
		io->networkIOMutex.unlock();
	}

	socket.close();
	return devicesFound;
}

int Discoverer::addDevice(TNetworkDevicecard &mc, int boardId, int protocol) {

	QString str;
	switch (boardId) {
		case 0: str = "Metis"; break;
		case 1: str = "Hermes"; break;
		case 2: str = "Griffin"; break;
		case 4: str = "Angelia"; break;
		case 5: str = "Orion"; break;
		case 6: str = "Hermes-Lite"; break;
		default: str = QString("Board-%1").arg(boardId); break;
	}

	mc.boardID = boardId;
	mc.boardName = str;
	mc.protocol = protocol;

    if (protocol == 1) {
        if (boardId == 1) { // Hermes
            mc.adcs = 1;
            mc.dacs = 1;
            mc.frequency_min = 0;
        } else {
            mc.adcs = 1;
            mc.dacs = 1;
        }
        mc.max_receivers = mc.adcs;
        mc.max_transmitters = mc.dacs;
    }

	mc.frequency_max = (boardId == 6) ? 30720000 : 61440000;

	io->networkIOMutex.lock();
	DISCOVERER_DEBUG << "Board ID: " << boardId << " (" << qPrintable(str) << ") protocol=" << protocol;
	io->networkIOMutex.unlock();

	m_deviceCards.append(mc);

	str += " (";
	str += mc.ip_address.toString();
	str += ")";
	set->addNetworkIOComboBoxEntry(str);

	return 1;
}

void Discoverer::displayDiscoverySocketError(QAbstractSocket::SocketError error) {

	io->networkIOMutex.lock();
	DISCOVERER_DEBUG << "discovery socket error: " << error;
	io->networkIOMutex.unlock();
}

void Discoverer::clear() {

	//m_metisDeviceComboBox->clear();
	m_deviceCards.clear();
}

void Discoverer::shutdownHPSDRDevice() {

	QByteArray arr;
	arr.resize(64);
	arr[0] = (char)0xEF;
	arr[1] = (char)0xFE;
	arr[2] = (char)0x04;
	arr[3] = (char)0x00;

	for (int i = 4; i < 64; i++) arr[i] = 0x00;

	QUdpSocket socket;
	QHostAddress addr = mc.ip_address;

	for (int i = 0; i < 10; i++) {

		if (socket.writeDatagram(arr, addr, DEVICE_PORT) < 0) {
			DISCOVERER_DEBUG << "forced shutdown socket write failed.";
		}
	}
}
