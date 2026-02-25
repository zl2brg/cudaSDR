/**
* @file  cusdr_dataIO.h
* @brief Data IO header file
* @author Hermann von Hasseln, DL3HVH
* @version 0.1
* @date 2011-10-01
*/

/*   
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

#ifndef _CUSDR_DATAIO_H
#define _CUSDR_DATAIO_H

#include "IHardwareIO.h"
#include "cusdr_settings.h"
#include "soundout.h"

#ifdef LOG_DATAIO
#   define DATAIO_DEBUG qDebug().nospace() << "DataIO::\t"
#else
#   define DATAIO_DEBUG nullDebug()
#endif


class DataIO : public IHardwareIO {

    Q_OBJECT

public:
    explicit DataIO(THPSDRParameter* ioData = nullptr);
	~DataIO() override;

public slots:
	// IHardwareIO interface
	void initIO()            override;   ///< Binds UDP socket; connected to thread started()
	void stop()              override;
	void writeData()         override;
	void sendAudio(unsigned char* buf) override;
	void sendInitFrames(int rx)  override;
	void sendCommand(char cmd)   override;
	void set_wbBuffers(int val)  override;

	// Legacy / internal helpers (called by the overrides above)
	void readData();   ///< Read from file-based input buffer (non-network mode)
	
private slots:
	void setSampleRate(QObject* sender, int value);
	void setManualSocketBufferSize(QObject* sender, bool value);
	void setSocketBufferSize(QObject* sender, int value);
	void displayDataReceiverSocketError(QAbstractSocket::SocketError error);
	void readDeviceData();
	void new_readDeviceData();
	// Internal Protocol 1 helpers
	void initDataReceiverSocket();       ///< Called by initIO()
	void sendInitFramesToNetworkDevice(int rx); ///< Called by sendInitFrames()
	void networkDeviceStartStop(char value);    ///< Called by sendCommand()

private:
	Settings*		set;
	std::unique_ptr<QUdpSocket>	m_dataIOSocket;
	//QMutex			m_mutex;
	QByteArray		m_commandDatagram;
	QByteArray		m_datagram;
	QByteArray		m_wbDatagram;
	QByteArray		m_twoFramesDatagram;
	QByteArray		m_metisGetDataSignature;
	QByteArray		m_outDatagram;
	QByteArray		m_deviceSendDataSignature;
	QString			m_message;
	unsigned char 	m_buffer[METIS_DATA_SIZE];
	QByteArray  	m_iqbuffer;

    QElapsedTimer	m_packetLossTime;

	THPSDRParameter*	io;
	//TNetworkDevicecard 	netDevice;

	bool	m_dataIOSocketOn;
	bool	m_networkDeviceRunning;
	bool	m_setNetworkDeviceHeader;

	long	m_sequence;
	long	m_oldSequence;
	long	m_sequenceWideBand;
	long	m_oldSequenceWideBand;
	long	m_sendSequence;
	long	m_oldSendSequence;

    std::unique_ptr<CSoundOut> m_pSoundCardOut;
	int		m_wbBuffers;
	int		m_wbCount;
	int		m_socketBufferSize;

	bool	m_sendEP4;
	bool	m_manualBufferSize;
	bool	m_packetsToggle;
	bool	m_firstFrame;
	
	volatile bool	m_stopped;

signals:
	// readydata() and messageEvent() are inherited from IHardwareIO
	void    readydata();
};

#endif // _CUSDR_DATAIO_H
