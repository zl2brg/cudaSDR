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

#include "cusdr_settings.h"
#include "Util/cusdr_queue.h"
#include "soundout.h"

#ifdef LOG_DATAIO
#   define DATAIO_DEBUG qDebug().nospace() << "DataIO::\t"
#else
#   define DATAIO_DEBUG nullDebug()
#endif


class RadioModel;
class DataEngine;
class IHPSDRProtocol;

class DataIO : public QObject {

    Q_OBJECT

public:
    explicit DataIO(QObject* parent = nullptr);
    void set_wbBuffers(int val);
    void setRadioModel(RadioModel* model) { m_radioModel = model; }
    void setDataEngine(DataEngine* engine) { m_dataEngine = engine; }
    void setProtocol(IHPSDRProtocol* protocol) { m_protocol = protocol; }
    void setSampleRate(int rate) { m_sampleRate = rate; }
	~DataIO() override;

	// Packet / audio queues and network sync (formerly THPSDRParameter)
	QByteArray				audioDatagram;
	QHQueue<TIQPacket>		iq_queue;
	QHQueue<QByteArray>		au_queue;
	QHQueue<QByteArray>		wb_queue;
	QHQueue<QVector<float>> soapy_iq_queue{100};
	QHQueue<QVector<float>> soapy_tx_iq_queue{100};
	QList<qreal>			inputBuffer;
	QMutex					networkIOMutex;
	QWaitCondition			devicefound;
	QHostAddress			hpsdrDeviceIPAddress;

public slots:
	void	stop();
	void	initDataReceiverSocket();
	/** Send armed P1 start, attach readyRead, then emit receiverReady(). Must run after exec(). */
	void	finishStartup();
	void	readData();
	void 	writeData();
	void 	sendAudio(u_char *buf);
	qint64	sendProtocol2ControlDatagram(const QByteArray &datagram, const QHostAddress &address, quint16 port);
	void	sendInitFramesToNetworkDevice(int rx);
	void	networkDeviceStartStop(char value);
	/** Arm P1 start so finishStartup() sends it from the bound socket. */
	void	armProtocol1Start(int rxCount, char startByte);
	/** DataIO-thread shutdown: drop readyRead, send P1/P2 stop, close sockets. */
	void	beginShutdown();
	/** Set the stop flag from any thread so readyRead loops can exit. */
	void	requestStop();
	
private slots:
	void setSampleRateSlot(int value);
	void setManualSocketBufferSize(bool value);
	void setSocketBufferSize(int value);
	void displayDataReceiverSocketError(QAbstractSocket::SocketError error);
	void readDeviceData();

private:
	void readDeviceDataP1(QUdpSocket* socket);
	void readDeviceDataP2(QUdpSocket* socket);
	void processWidebandPacket(qint64 size);

	Settings*		set;
	QUdpSocket*	    m_dataIOSocket;
    QMap<quint16, QUdpSocket*> m_sockets;
	QMap<QUdpSocket*, quint16> m_socketLogicalPorts;
	QByteArray		m_commandDatagram;
	QByteArray		m_datagram;
	QByteArray		m_wbDatagram;
	QByteArray		m_outDatagram;

    QElapsedTimer	m_packetLossTime;
    QElapsedTimer   m_iqArrivalTimer;
    quint64         m_iqPacketCount = 0;
    quint64         m_iqDropCount   = 0;
    double          m_iqGapMax      = 0;
    double          m_iqGapAccum    = 0;

	DataEngine*			m_dataEngine = nullptr;
	RadioModel*			m_radioModel = nullptr;
	IHPSDRProtocol*		m_protocol = nullptr;
	int					m_sampleRate = 48000;

	bool	m_dataIOSocketOn;
	bool	m_networkDeviceRunning;
	bool	m_setNetworkDeviceHeader;

	uint32_t	m_sequence;
	uint32_t	m_oldSequence;
	uint32_t	m_sequenceWideBand;
	uint32_t	m_oldSequenceWideBand;
	uint32_t	m_sendSequence;
	uint32_t	m_oldSendSequence;

    std::unique_ptr<CSoundOut> m_pSoundCardOut;
	int		m_wbBuffers;
	int		m_wbCount;
	int		m_socketBufferSize;

	bool	m_sendEP4;
	bool	m_manualBufferSize;
	
	QElapsedTimer	m_widebandLogTimer;
	uint32_t		m_widebandMissedAccum;
	
	volatile bool	m_stopped;
	bool	m_pendingP1Start = false;
	int		m_pendingP1RxCount = 0;
	char	m_pendingP1StartByte = 0;

signals:
	void	messageEvent(QString message);
	void    readydata();
	void	receiverReady();
	void	startupFailed();
};

#endif // _CUSDR_DATAIO_H
