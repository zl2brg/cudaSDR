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
#include "cusdr_dataIO.h"
#include "protocol_boundary_utils.h"
#include <SoapySDR/Device.hpp>
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

Discoverer::Discoverer(DataIO *dataIO)
    : QObject()
	, set(Settings::instance())
	, m_dataIO(dataIO)
{
	m_deviceCards = set->getMetisCardsList();
}

Discoverer::~Discoverer() {
}

TNetworkDevicecard mc;

#ifdef HAVE_SOAPYSDR
void Discoverer::discoverSoapyDevices() {
    QList<TSoapyDevice> list;
    auto results = SoapySDR::Device::enumerate();
    
    for (const auto &res : results) {
        TSoapyDevice dev;
        if (res.count("driver")) dev.driver = QString::fromStdString(res.at("driver"));
        
        // Skip generic audio devices
        if (dev.driver == "audio") continue;

        if (res.count("hardware")) dev.hardware = QString::fromStdString(res.at("hardware"));
        if (res.count("name")) dev.name = QString::fromStdString(res.at("name"));
        if (res.count("serial")) dev.serial = QString::fromStdString(res.at("serial"));
        if (res.count("label")) dev.label = QString::fromStdString(res.at("label"));
        else dev.label = dev.name.isEmpty() ? dev.driver : dev.name;
        
        for (const auto &it : res) {
            dev.args[QString::fromStdString(it.first)] = QString::fromStdString(it.second);
        }
        list.append(dev);
    }
    
    emit soapyDeviceListFound(list);
}
#endif

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

		m_dataIO->networkIOMutex.lock();
		DISCOVERER_DEBUG << "no device found - trying again...";
		m_dataIO->networkIOMutex.unlock();
	}

	m_dataIO->networkIOMutex.lock();
	m_dataIO->devicefound.wakeAll();
	m_dataIO->networkIOMutex.unlock();
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
	m_dataIO->networkIOMutex.lock();
	DISCOVERER_DEBUG << "using " << qPrintable(QHostAddress(set->getHPSDRDeviceLocalAddr()).toString()) << " for discovery.";
	m_dataIO->networkIOMutex.unlock();

	// clear comboBox entries in the network dialogue
	set->clearNetworkIOComboBoxEntry();

    if (socket.bind(QHostAddress(set->getHPSDRDeviceLocalAddr()), 0,
                    QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress))
    {
        set->setMetisPort(socket.localPort());
        {
            QMutexLocker l(&m_dataIO->networkIOMutex);
            DISCOVERER_DEBUG << "discovery_socket bound successfully to port " << socket.localPort();
        }
    }
    else {
        {
            QMutexLocker l(&m_dataIO->networkIOMutex);
            DISCOVERER_DEBUG << "discovery_socket bind failed: " << socket.errorString();
        }
        socket.close();
        return 0;
    }

	if (socket.writeDatagram(m_findDatagram, QHostAddress::Broadcast, DEVICE_PORT) == 63) {

		m_dataIO->networkIOMutex.lock();
		DISCOVERER_DEBUG << "Protocol 1 discovery data sent.";
		m_dataIO->networkIOMutex.unlock();
	}
	else {

		m_dataIO->networkIOMutex.lock();
		DISCOVERER_DEBUG << "Protocol 1 discovery data not sent.";
		m_dataIO->networkIOMutex.unlock();
	}

	if (socket.writeDatagram(p2FindDatagram, QHostAddress::Broadcast, DEVICE_PORT) == 60) {

		m_dataIO->networkIOMutex.lock();
		DISCOVERER_DEBUG << "Protocol 2 discovery data sent.";
		m_dataIO->networkIOMutex.unlock();
	}
	else {

		m_dataIO->networkIOMutex.lock();
		DISCOVERER_DEBUG << "Protocol 2 discovery data not sent.";
		m_dataIO->networkIOMutex.unlock();
	}


	// wait a little
	//QThread::msleep(30);
	QThread::msleep(500);

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
			if (ProtocolBoundaryUtils::isProtocol1DiscoveryProbeEcho(
					reinterpret_cast<const unsigned char*>(m_deviceDatagram.constData()),
					m_deviceDatagram.size())) {
				DISCOVERER_DEBUG << "ignoring Protocol 1 discovery probe echo from "
								 << qPrintable(mc.ip_address.toString());
				continue;
			}
			if (m_deviceDatagram[2] == (char)0x02) {

				sprintf(mc.mac_address, "%02X:%02X:%02X:%02X:%02X:%02X",
					m_deviceDatagram[3] & 0xFF, m_deviceDatagram[4] & 0xFF, m_deviceDatagram[5] & 0xFF,
					m_deviceDatagram[6] & 0xFF, m_deviceDatagram[7] & 0xFF, m_deviceDatagram[8] & 0xFF);

				m_dataIO->networkIOMutex.lock();
				DISCOVERER_DEBUG << "[P1] Device found at " << qPrintable(mc.ip_address.toString()) << ":" << port << "; Mac addr: [" << mc.mac_address << "]";
				DISCOVERER_DEBUG << "[P1] Device code version: " << qPrintable(QString::number(m_deviceDatagram.at(9), 16));
				m_dataIO->networkIOMutex.unlock();

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
				int minorVer = (m_deviceDatagram.size() >= 22) ? (unsigned char)m_deviceDatagram.at(21) : 0;
				int swVer = (m_deviceDatagram.size() >= 10) ? (unsigned char)m_deviceDatagram.at(9) : 0;
				mc.sw_version = swVer;
				if (no == 0)
					set->setMetisVersion(swVer);
				else
					set->setHermesVersion(swVer);

				devicesFound += addDevice(mc, no, 1, swVer, minorVer);
			}
			else if (m_deviceDatagram[2] == (char)0x03) {

				m_dataIO->networkIOMutex.lock();
				DISCOVERER_DEBUG << "[P1] Device already sending data - trying to shut down...";
				m_dataIO->networkIOMutex.unlock();

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

				m_dataIO->networkIOMutex.lock();
				DISCOVERER_DEBUG << "[P2] Device found at " << qPrintable(mc.ip_address.toString()) << ":" << port 
                                 << "; Mac: [" << mc.mac_address << "] board=" << no << " fw=" << version 
                                 << " receivers=" << num_ddcs;
				m_dataIO->networkIOMutex.unlock();

                mc.sw_version = version;
                mc.status = status;

				set->setHermesVersion(version); // Most P2 devices are Hermes/Orion-class
				devicesFound += addDevice(mc, no, 2, version, 0, num_ddcs, num_dacs);

				if (status == 0x03) {
					m_dataIO->networkIOMutex.lock();
					DISCOVERER_DEBUG << "[P2] Device already running.";
					m_dataIO->networkIOMutex.unlock();
				}
			}
		}
	}
	set->setMetisCardList(m_deviceCards);

	if (devicesFound == 1) {

		set->setCurrentHPSDRDevice(m_deviceCards.at(0));
		m_dataIO->networkIOMutex.lock();
		DISCOVERER_DEBUG << "Device selected: " << qPrintable(m_deviceCards.at(0).ip_address.toString());
		m_dataIO->networkIOMutex.unlock();
	}

	socket.close();
	return devicesFound;
}

int Discoverer::addDevice(TNetworkDevicecard &mc, int boardId, int protocol, int swVersion, int minorVersion, int numDdcs, int numDacs) {
	ProtocolBoundaryUtils::HpsdrDeviceInfo info = ProtocolBoundaryUtils::decodeHpsdrDevice(boardId, protocol, swVersion, minorVersion);

	mc.boardID = boardId;
	mc.boardName = info.boardName;
	mc.protocol = protocol;
	mc.adcs = (numDdcs > 0 && protocol == 2) ? qMin(numDdcs, info.adcs) : info.adcs;
	mc.dacs = (numDacs > 0 && protocol == 2) ? numDacs : info.dacs;
	mc.max_receivers = (numDdcs > 0 && protocol == 2) ? numDdcs : info.maxReceivers;
	mc.max_transmitters = (numDacs > 0 && protocol == 2) ? numDacs : info.maxTransmitters;
	mc.frequency_min = info.frequencyMin;
	mc.frequency_max = info.frequencyMax;

	QString displayStr = QString("%1 (%2, %3, P%4)")
		.arg(info.modelName,
		     mc.ip_address.toString(),
		     info.firmwareString.isEmpty() ? QString("ID 0x%1").arg(boardId, 2, 16, QLatin1Char('0')) : info.firmwareString)
		.arg(protocol);

	m_dataIO->networkIOMutex.lock();
	DISCOVERER_DEBUG << "Discovered " << qPrintable(displayStr) << " ADCs=" << mc.adcs << " maxRx=" << mc.max_receivers;
	m_dataIO->networkIOMutex.unlock();

	m_deviceCards.append(mc);
	set->addNetworkIOComboBoxEntry(displayStr);

	return 1;
}

void Discoverer::displayDiscoverySocketError(QAbstractSocket::SocketError error) {

	m_dataIO->networkIOMutex.lock();
	DISCOVERER_DEBUG << "discovery socket error: " << error;
	m_dataIO->networkIOMutex.unlock();
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
