/**
* @file cusdr_sliceProcessor.h
* @brief Per-slice DSP worker (IQ queue, WDSP, spectrum, audio)
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

#ifndef CUSDR_SLICE_PROCESSOR_H
#define CUSDR_SLICE_PROCESSOR_H

#include "cusdr_settings.h"
#include "Util/cusdr_highResTimer.h"
#include "QtWDSP/qtwdsp_dspEngine.h"
#include "QtDSP/qtdsp_qComplex.h"
#include "receiveraudiooutput.h"

#ifdef HAVE_CODEC2
#include "AudioEngine/cusdr_freedvprocessor.h"
#endif

#ifdef LOG_SLICE_PROCESSOR
#   define SLICE_PROCESSOR_DEBUG qDebug().nospace() << "SliceProcessor::\t"
#else
#   define SLICE_PROCESSOR_DEBUG nullDebug()
#endif


class SliceModel;
class SliceProcessor : public QObject {

	Q_OBJECT

public:
	explicit SliceProcessor(SliceModel *model, QObject *parent = nullptr);
	~SliceProcessor() override;

	void	setupConnections();
	bool	initDSPInterface();

	void	enqueueData();


	QSDR::_ServerMode	getServerMode()	const;
	QHostAddress		getPeerAddress()		{ return m_peerAddress; }

	int		getAudioMode()			{ return m_audioMode; }
	int		getSocketDescriptor()	{ return m_socketDescriptor; }
	int		getReceiverNo()			{ return m_receiver; }
	int		getClient()				{ return m_client; }
	int		getIQPort()				{ return m_iqPort; }
	int		getBSPort()				{ return m_bsPort; }
	int		getDisplayDelay()		{ return m_displayTime; }
	bool	getConnectedStatus()	{ return m_connected; }
    void 	setAudioBufferSize();

    float	in[BUFFER_SIZE * 2];
    float	out[BUFFER_SIZE * 2];
	float	temp[BUFFER_SIZE * 4];
	float	spectrum[BUFFER_SIZE * 4];
    RadioState m_state  = RadioState::RX;
	QVector<float>	newSpectrum;
    QWDSPEngine*	qtwdsp = nullptr;
    std::unique_ptr<HResTimer>	highResTimer;
	ReceiverAudioOutput *m_audioOutput = nullptr;

	CPX			inBuf;
    CPX			outBuf;
    CPX			audioOutputBuf;

    QHQueue<QVector<int32_t>> m_iqQueue;
    QHQueue<QVector<float>>   m_soapyQueue;
    int32_t     m_rawIQ[BUFFER_SIZE * 2];

public slots:
    void    enqueueRawData();
    void    enqueueRawData(const QVector<int32_t> &rawBlock);
    void    enqueueSoapyData(const QVector<float> &data);
    void    dspProcessingSoapy();
	void	setReceiverData(TReceiver data);
	void	setAudioMode(int mode);
	void	setServerMode(QSDR::_ServerMode mode);
	void	setPeerAddress(QHostAddress addr);
	void	setSocketDescriptor(int value);
	void	setReceiver(int value);
	void	setClient(int value);
	void	setIQPort(int value);
	void	setBSPort(int value);
	void	setConnectedStatus(bool value);
    void	setSampleRate(int value);
	void	setFreeDVMode(int rx, int mode);

	void	dspProcessing();
    void    dspProcessing(const QVector<int32_t> &rawIQ);
	void	stop();

private:
    void    dspProcessingCore();

private slots:
	void	setSystemState(
					QSDR::_Error err, 
					QSDR::_HWInterfaceMode hwmode, 
					QSDR::_ServerMode mode, 
					QSDR::_DataEngineState state);

	void 	setFramesPerSecond(int rx, int value);

	bool	initQtWDSPInterface();

private:

    QVector<float> interleaveFromCPX(const CPX& in, int size = -1);
    SliceModel*      m_sliceModel;
    Settings*                               set;
	
	QSDR::_ServerMode		m_serverMode;
	QSDR::_HWInterfaceMode	m_hwInterface;
	QSDR::_DataEngineState	m_dataEngineState;

	QHostAddress	m_peerAddress;
	quint16			m_peerPort;

	QMutex				m_mutex;

	QElapsedTimer		m_smeterTime;
	QElapsedTimer		m_dspCallTimer;
	double				m_dspTimeMin = 1e9;
	double				m_dspTimeMax = 0.0;
	double				m_dspTimeAccum = 0.0;
	quint64				m_dspCallCount = 0;
	double				m_sMeterValue;

	volatile bool	m_stopped;

	int		m_receiver;
	int		m_samplerate;
	int		m_audioMode;
	int		m_socketDescriptor;
	int		m_client;
    int		m_iqPort;
    int		m_bsPort;
	int		m_displayTime;

    int 	m_audiobuffersize;

	bool	m_connected;
    int     m_rateTransitionDropBuffers;
    QMutex  m_dspMutex;

#ifdef HAVE_CODEC2
	FreeDVProcessor* m_freeDVProcessor = nullptr;
	int m_freeDVMode = 0;
	quint64 m_freeDVRxFrames = 0;
#endif

signals:
	void	messageEvent(QString msg);
	void	spectrumBufferChanged(int rx, const qVectorFloat& buffer);
	void	sMeterValueChanged(int rx, double value);
	void	outputBufferSignal(int rx, const CPX &buffer);
	void	audioBufferSignal(int rx, const CPX &buffer, int);

};

#endif  // CUSDR_SLICE_PROCESSOR_H
