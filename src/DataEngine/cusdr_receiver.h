/**
* @file cusdr_receiver.h
* @brief cuSDR receiver header file
* @author Hermann von Hasseln, DL3HVH, Updated for WDSP by Simon Eatough ZL2BRG
* @version 1.0
* Updated 2018-04-10 for WDSP
* Updated 2025-09-10 for qt6
*/

/* Copyright (C)
*
* 2010 - Hermann von Hasseln, DL3HVH, author of cuSDR updated by Simon Eatough ZL2BRG for WDSP
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/

#ifndef CUSDR_RECEIVER_H
#define CUSDR_RECEIVER_H

//#include <QObject>
//#include <QtNetwork>

#include "cusdr_settings.h"
#include "Util/cusdr_highResTimer.h"
#include "QtWDSP/qtwdsp_dspEngine.h"
#include "QtDSP/qtdsp_qComplex.h"
#include "receiveraudiooutput.h"

#ifdef LOG_RECEIVER
#   define RECEIVER_DEBUG qDebug().nospace() << "Receiver::\t"
#else
#   define RECEIVER_DEBUG nullDebug()
#endif


class Receiver : public QObject {

	Q_OBJECT

public:
	explicit Receiver(int rx = 0);
	~Receiver() override;

	void	setupConnections();
	bool	initDSPInterface();

	void	enqueueData();


	QSDR::_ServerMode	getServerMode()	const;
	QSDR::_DSPCore		getDSPCoreMode() const;
	QHostAddress		getPeerAddress()		{ return m_peerAddress; }
	HamBand				getHamBand()			{ return m_receiverData.hamBand; }
	ADCMode				getADCMode()			{ return m_receiverData.adcMode; }
	AGCMode				getAGCMode()			{ return m_receiverData.agcMode; }
	QList<int>			getMercuryAttenuators() { return m_receiverData.mercuryAttenuators; }
	QList<DSPMode>		getDSPModeList()		{ return m_receiverData.dspModeList; }

	int		getAudioMode()			{ return m_audioMode; }
	int		getSocketDescriptor()	{ return m_socketDescriptor; }
	int		getReceiverNo()			{ return m_receiver; }
	int		getClient()				{ return m_client; }
	int		getIQPort()				{ return m_iqPort; }
	int		getBSPort()				{ return m_bsPort; }
	//int		getID()					{ return m_receiverID; }
	int		getSampleRate()			{ return m_receiverData.sampleRate; }
	int		getDisplayDelay()		{ return m_displayTime; }
	qreal	getAGCGain()			{ return m_receiverData.acgGain; }
	float	getAudioVolume()		{ return m_receiverData.audioVolume; }
	long	getCtrFrequency()		{ return m_receiverData.ctrFrequency; }
	long	getVfoFrequency()		{ return m_receiverData.vfoFrequency; }
	double	getFilterLo()			{ return m_receiverData.filterLo; }
	double	getFilterHi()			{ return m_receiverData.filterHi; }
	qreal	getdBmPanScaleMin()		{ return m_dBmPanScaleMin; }
	qreal	getdBmPanScaleMax()		{ return m_dBmPanScaleMax; }
	bool	getConnectedStatus()	{ return m_connected; }
    void 	setAudioBufferSize();

    float	in[BUFFER_SIZE * 2];
    float	out[BUFFER_SIZE * 2];
	float	temp[BUFFER_SIZE * 4];
	float	spectrum[BUFFER_SIZE * 4];
    RadioState m_state  = RadioState::RX;
	QVector<float>	newSpectrum;
    QWDSPEngine*	qtwdsp = nullptr;
 //   std::unique_ptr<QWDSPEngine> qtwdsp;
    std::unique_ptr<HResTimer>	highResTimer;
	ReceiverAudioOutput *m_audioOutput = nullptr;

	CPX			inBuf;
    CPX			outBuf;
    CPX			audioOutputBuf;

    QHQueue<QVector<int32_t>> m_iqQueue;
    int32_t     m_rawIQ[BUFFER_SIZE * 2];

public slots:
    void    enqueueRawData();
	void	setReceiverData(TReceiver data);
	void	setAudioMode(QObject* sender, int mode);
	void	setServerMode(QSDR::_ServerMode mode);
	void	setPeerAddress(QHostAddress addr);
	void	setSocketDescriptor(int value);
	void	setReceiver(int value);
	void	setClient(int value);
	void	setIQPort(int value);
	void	setBSPort(int value);
	void	setConnectedStatus(bool value);
	//void	setID(int value);
    void	setSampleRate(QObject* sender,int value);
	void	setHamBand(QObject* sender, int rx, bool byBtn, HamBand band);
	void	setDspMode(QObject* sender, int rx, DSPMode mode);
	void	setADCMode(QObject* sender, int rx, ADCMode mode);
	void	setAudioVolume(QObject* sender, int rx, float value);
	void	setCtrFrequency(long frequency);
	void	setVfoFrequency(long frequency);
	void	setFilterFrequencies(QObject* sender, int rx, qreal low, qreal high);
	void	setLastCtrFrequencyList(const QList<long> &frequencies);
	void	setLastVfoFrequencyList(const QList<long> &frequencies);
	void	setdBmPanScaleMin(qreal value);
	void	setdBmPanScaleMax(qreal value);
	void	setMercuryAttenuators(const QList<int> &attenuators);

	void	dspProcessing();
	void	stop();

private slots:
	void	setSystemState(
					QObject* sender, 
					QSDR::_Error err, 
					QSDR::_HWInterfaceMode hwmode, 
					QSDR::_ServerMode mode, 
					QSDR::_DataEngineState state);


	void 	setFramesPerSecond(QObject *sender, int rx, int value);

	bool	initQtWDSPInterface();


    
	//void	setAGCMaximumGain_dBm(QObject* sender, int rx, int value);
	void	setAGCFixedGain_dB(QObject* sender, int rx, qreal value);

private:

    QVector<float> interleaveFromCPX(const CPX& in, int size = -1);
	Settings*				set;
	
	QSDR::_ServerMode		m_serverMode;
	QSDR::_HWInterfaceMode	m_hwInterface;
	QSDR::_DataEngineState	m_dataEngineState;

	TReceiver 		m_receiverData;
	QHostAddress	m_peerAddress;
	quint16			m_peerPort;

	QMutex				m_mutex;

	QElapsedTimer		m_smeterTime;
	double				m_sMeterValue;

	volatile bool	m_stopped;

	int		m_receiver;
	int		m_samplerate;
	int		m_audioMode; // 1 = audio on, 0 = audio off
	int		m_socketDescriptor;
	int		m_client;
    int		m_iqPort;
    int		m_bsPort;
	int		m_displayTime;

	//qreal	m_calOffset;
	qreal	m_dBmPanScaleMin;
	qreal	m_dBmPanScaleMax;
    int 	m_audiobuffersize;

	bool	m_connected;
    int     m_rateTransitionDropBuffers;
    QMutex  m_dspMutex;

signals:
	void	messageEvent(QString msg);
	void	spectrumBufferChanged(int rx, const qVectorFloat& buffer);
	void	sMeterValueChanged(int rx, double value);
	void	outputBufferSignal(int rx, const CPX &buffer);
	void	audioBufferSignal(int rx, const CPX &buffer, int);

};

#endif  // CUSDR_RECEIVER_H
