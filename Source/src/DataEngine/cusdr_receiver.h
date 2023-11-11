/**
* @file cusdr_receiver.h
* @brief cuSDR receiver header file
* @author Hermann von Hasseln, DL3HVH
* @version 0.1
* @date 2010-11-12
*/

/* Copyright (C)
*
* 2010 - Hermann von Hasseln, DL3HVH
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

    void	deleteDSPInterface();

	void	enqueueData();


	QSDR::_ServerMode	getServerMode()	const;
	QSDR::_DSPCore		getDSPCoreMode() const;
	QHostAddress		getPeerAddress()		{ return m_peerAddress; }
	HamBand				getHamBand()			{ return m_hamBand; }
	ADCMode				getADCMode()			{ return m_adcMode; }
	AGCMode				getAGCMode()			{ return m_agcMode; }
	QList<int>			getMercuryAttenuators() { return m_mercuryAttenuators; }
	QList<DSPMode>		getDSPModeList()		{ return m_dspModeList; }

	int		getAudioMode()			{ return m_audioMode; }
	int		getSocketDescriptor()	{ return m_socketDescriptor; }
	int		getReceiverNo()			{ return m_receiver; }
	int		getClient()				{ return m_client; }
	int		getIQPort()				{ return m_iqPort; }
	int		getBSPort()				{ return m_bsPort; }
	//int		getID()					{ return m_receiverID; }
	int		getSampleRate()			{ return m_sampleRate; }
	int		getDisplayDelay()		{ return m_displayTime; }
	qreal	getAGCGain()			{ return m_agcGain; }
	float	getAudioVolume()		{ return m_audioVolume; }
	long	getCtrFrequency()		{ return m_ctrFrequency; }
	long	getVfoFrequency()		{ return m_vfoFrequency; }
    long    getTxFrequency();
	double	getFilterLo()			{ return m_filterLo; }
	double	getFilterHi()			{ return m_filterHi; }
	qreal	getdBmPanScaleMin()		{ return m_dBmPanScaleMin; }
	qreal	getdBmPanScaleMax()		{ return m_dBmPanScaleMax; }
	bool	getConnectedStatus()	{ return m_connected; }
	void 	setAudioBufferSize();
    void    init_analyzer(int refreshrate);
    void    calcDisplayAveraging();
    void    start();

    float	in[BUFFER_SIZE * 2];
    float	out[BUFFER_SIZE * 2];
	float	temp[BUFFER_SIZE * 4];
	float	spectrum[BUFFER_SIZE * 4];
    qVectorFloat spectrumBuffer;
    RadioState m_state  = RadioState::RX;

	QVector<float>	newSpectrum;

	QWDSPEngine *qtwdsp;
	HResTimer	*highResTimer;

	CPX			inBuf;
    CPX			outBuf;
    CPX			audioOutputBuf;

	QHQueue<CPX>	inQueue;

public slots:
    void    setNCOFrequency(int rx, long value);
    void    setSampleSize(int rx, int size);
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
	void	setSampleRate(int value);
	void	setHamBand(QObject* sender, int rx, bool byBtn, HamBand band);
	void	setDspMode(QObject* sender, int rx, DSPMode mode);
	void	setADCMode(QObject* sender, int rx, ADCMode mode);
	void	setAGCMode(QObject* sender, int rx, AGCMode mode, bool hang);
	void	setAGCGain(QObject* sender, int rx, int value);
	void	setAudioVolume(QObject* sender, int rx, float value);
	void	setCtrFrequency(long frequency);
	void	setVfoFrequency(long frequency);
	void	setFilterFrequencies(QObject* sender, int rx, qreal low, qreal high);
	void	setLastCtrFrequencyList(const QList<long> &frequencies);
	void	setLastVfoFrequencyList(const QList<long> &frequencies);
	void	setdBmPanScaleMin(qreal value);
	void	setdBmPanScaleMax(qreal value);
	void	setMercuryAttenuators(const QList<int> &attenuators);
    void    setAGC_Line(int rx);
//    double getSMeterInstValue();
    void ProcessFrequencyShift(CPX &in, CPX &out);
    void setDisplayrAveragegMode(int rx, int mode);
    void setDisplayDetectorMode(int rx, int mode);
    void setDisplyAveragingCount(QObject *sender, int rx, int count);
    void setfftSize(int rx, int value);
    void setfmsqLevel(int rx, int value);
    void setNoiseBlankerMode(int rx);
    void setNoiseBlankerMode(int rx, int nb);
    void setNoiseFilterMode(int rx, int nr);

    void setNr2Ae(int rx, bool value);

    void setNr2GainMethod(int rx, int value);

    void setNr2NpeMethod(int rx, int value);

    void setanf(int rx, bool value);

    void setsnb(int rx, bool value);
    void setNrAGC(int rx, int value);
    bool	initWDSPInterface();
    void	dspProcessing();
	void	stop();
    bool openReceiver();

    void closeReceiver();

private slots:
	void	setSystemState(
					QObject* sender, 
					QSDR::_Error err, 
					QSDR::_HWInterfaceMode hwmode, 
					QSDR::_ServerMode mode, 
					QSDR::_DataEngineState state);

	void	setSampleRate(QObject *sender, int value);
	void 	setDisplayFramesPerSecond(QObject *sender, int rx, int value);



    //void	setAGCMaximumGain_dBm(QObject* sender, int rx, int value);
	void	setAGCMaximumGain_dB(QObject* sender, int rx, qreal value);
	void	setAGCFixedGain_dB(QObject* sender, int rx, qreal value);
	void	setAGCThreshold_dB(QObject* sender, int rx, qreal value);
	void 	setAGCHangLevel_dB(QObject* sender, int rx, qreal value);
	void 	setAGCHangThreshold(QObject* sender, int rx, int value);
	void	setAGCSlope_dB(QObject *sender, int rx, qreal value);
	void	setAGCAttackTime(QObject* sender, int rx, qreal value);
	void 	setAGCDecayTime(QObject* sender, int rx, qreal value);
	void 	setAGCHangTime(QObject* sender, int rx, qreal value);


private:
	Settings*				set;
	
	QSDR::_DSPCore			m_dspCore;
	QSDR::_ServerMode		m_serverMode;
	QSDR::_HWInterfaceMode	m_hwInterface;
	QSDR::_DataEngineState	m_dataEngineState;

	TReceiver 		m_receiverData;
	QHostAddress	m_peerAddress;
	quint16			m_peerPort;

	HamBand				m_hamBand;
	DSPMode				m_dspMode;
	AGCMode				m_agcMode;
	ADCMode				m_adcMode;
	TDefaultFilterMode	m_filterMode;

	QList<long>			m_lastCtrFrequencyList;
	QList<long>			m_lastVfoFrequencyList;
	QList<DSPMode>		m_dspModeList;
	QList<int>			m_mercuryAttenuators;

	QElapsedTimer   	m_smeterTime;
	QMutex				m_mutex;

	volatile bool	m_stopped;

	int		m_receiver;
	int		m_samplerate;
	int		m_audioMode; // 1 = audio on, 0 = audio off
	int		m_socketDescriptor;
	int		m_client;
    int		m_iqPort;
    int		m_bsPort;
	int		m_sampleRate;
	int		m_displayTime;

	long	m_ctrFrequency;
	long	m_vfoFrequency;

	float	m_audioVolume;
	double	m_sMeterValue;

	qreal	m_agcGain;
	qreal	m_agcFixedGain_dB;
	qreal	m_agcMaximumGain_dB;
	qreal	m_agcThreshold_dBm;
	qreal	m_agcHangThreshold;
	qreal	m_agcHangLevel;
	int		m_agcSlope;
	int		m_agcAttackTime;
	int		m_agcDecayTime;
	int		m_agcHangTime;
	//qreal	m_calOffset;
	qreal	m_filterLo;
	qreal	m_filterHi;
	qreal	m_dBmPanScaleMin;
	qreal	m_dBmPanScaleMax;
	int 	m_audiobuffersize;
	int     m_refreshrate;

	bool	m_connected;
	bool	m_hangEnabled;
    int		m_spectrumSize;
    int		m_fftMultiplier;
    int 	m_fftSize;
    int     m_averageCount;
    double  m_display_avb;
    int     m_display_average;
    int     m_PanAvMode;
    int     m_PanDetMode;
    float	m_volume;
    qreal   m_agcThreshold;
    double   m_agcMaximumGain;
    DSPMode	m_dspmode;
    int 	m_nb;
    int 	m_nb2;
    int 	m_nr;
    int 	m_nr2;
    int 	m_anf;
    int 	m_snb;
    int     m_nr_agc;
    int     m_nr2_ae;
    int     m_nr2_npe_method;
    int     m_nr2_gain_method;
    int     m_nbMode;
    int     m_nrMode;

	//void	setupConnections();

signals:
	void	messageEvent(QString msg);
	void	spectrumBufferChanged(int rx, const qVectorFloat& buffer);
	void	sMeterValueChanged(int rx, double value);
	void	outputBufferSignal(int rx, const CPX &buffer);
	void	audioBufferSignal(int rx, const CPX &buffer, int);



//    void closeReceiver();
};

#endif  // CUSDR_RECEIVER_H
