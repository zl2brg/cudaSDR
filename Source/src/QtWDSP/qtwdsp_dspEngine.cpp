
/**
* @file  qtwdsp_dspEngine.cpp
* @brief QtWDSP DSP engine class
* @author Hermann von Hasseln, DL3HVH
* @version 0.1
* @date 2012-04-07
*/

/*   
 *   Copyright (C) 2007, 2008, 2009, 2010 Philip A Covington, N8VB
 *
 *	 adapted for QtDSP by (C) 2012 Hermann von Hasseln, DL3HVH
 *
 *   The ProcessFrequencyShift method is adpated from cuteSDR by (C) Moe Wheatley, AE4JY.
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
#define LOG_WDSP_ENGINE

// use: WDSP_ENGINE_DEBUG << "debug message";

#include "qtwdsp_dspEngine.h"
#include "cusdr_receiver.h"


double wmyLog(double x, double base) {

	return log(x) / log(base);
}

QWDSPEngine::QWDSPEngine(QObject *parent, int rx, int size)
	: QObject(parent)
	, set(Settings::instance())
	, m_qtdspOn(false)
	, m_rx(rx)
	, m_size(size)
	, m_samplerate(set->getSampleRate())
	, m_fftMultiplier(1)
	, m_volume(0.0f)

{
    int result;
    qRegisterMetaType<QVector<cpx> >();
	qRegisterMetaType<CPX>();
    m_refreshrate = set->getFramesPerSecond(m_rx);
    m_averageCount= set->getSpectrumAveragingCnt(m_rx);
    m_PanAvMode = set->getPanAveragingMode(m_rx);
    m_PanDetMode = set->getPanDetectorMode(m_rx);
    m_agcSlope = set->getAGCSlope(m_rx);
    m_agcMaximumGain = set->getAGCMaximumGain_dB(m_rx);
	m_agcHangThreshold = set->getAGCHangThreshold(m_rx);
	spectrumBuffer.resize(BUFFER_SIZE*4);
	m_fftSize = getfftVal(set->getfftSize(m_rx));
	m_nr_agc = set->getNrAGC(m_rx);
    m_nr2_ae = set->getNr2ae(m_rx);
    m_nr2_gain_method = set->getNr2GainMethod(m_rx);
    m_nr2_npe_method = set->getNr2NpeMethod(m_rx);
    m_nbMode = set->getnbMode(m_rx);
	m_nrMode = set->getnrMode(m_rx);
	m_anf = set->getAnf(m_rx);
    m_snb= set->getSnb(m_rx);

    setNCOFrequency(m_rx, m_rxData.vfoFrequency - m_rxData.ctrFrequency);
	WDSP_ENGINE_DEBUG << "init DSPEngine with size: " << m_size;
	SleeperThread::msleep(100);

	setupConnections();

    WDSP_ENGINE_DEBUG << "Opening WDSP channel" << m_rx << "m_size=" << m_size << "Sample rate=" << m_samplerate;
    OpenChannel(m_rx,
                m_size,
                2048, // ,
                m_samplerate,
                48000, // dsp rate
                48000, // output rate
                0, // receive
                0, // run
                0.010, 0.025, 0.0, 0.010, 0);

	create_anbEXT(m_rx,1,size,m_samplerate,0.0001,0.0001,0.0001,0.05,20);
	create_nobEXT(m_rx,1,0,size,m_samplerate,0.0001,0.0001,0.0001,0.05,20);
	fprintf(stderr,"RXASetNC %d\n",m_fftSize);
	RXASetNC(m_rx, m_fftSize);

	setFilterMode(m_rx);
//	fprintf(stderr,"RXASetMP %d\n",rx->low_latency);
//	RXASetMP(rx->id, rx->low_latency);
    SetRXAFMDeviation(m_rx, (double)8000.0);
	SetRXAMode(m_rx, FMN);
	RXASetNC(m_rx,4096);
    SetRXAPanelRun(m_rx, 1);
    SetRXAPanelSelect(m_rx,3);
	XCreateAnalyzer(m_rx, &result,262144, 1, 1, (char *) "");
    if(result != 0) {
        WDSP_ENGINE_DEBUG <<  "XCreateAnalyzer id=%d failed: %d\n" <<  result;
    }
	init_analyzer(m_refreshrate);
    calcDisplayAveraging();
    SetDisplayAvBackmult(rx, 0, m_display_avb);
    SetDisplayNumAverage(rx, 0, m_display_average);
    SetDisplayDetectorMode(rx,0,m_PanDetMode);
    SetDisplayAverageMode(rx,0,m_PanAvMode);
	SetRXAFMSQRun(rx,1);
    SetChannelState(m_rx,1,0);
}



QWDSPEngine::~QWDSPEngine() {
	SetChannelState(m_rx,0,0);
    SetChannelState(TX_ID,0,0);
    CloseChannel(m_rx);
    CloseChannel(TX_ID);
    DestroyAnalyzer(m_rx);
    SetRXAFMSQRun(m_rx,0);
	destroy_nobEXT(m_rx);
	destroy_anbEXT(m_rx);

}

void QWDSPEngine::setupConnections() {

	CHECKED_CONNECT(
			set,
			SIGNAL(ncoFrequencyChanged(int, long)),
			this,
			SLOT(setNCOFrequency(int, long)));

	CHECKED_CONNECT(
			set,
			SIGNAL(sampleSizeChanged(int, int)),
			this,
			SLOT(setSampleSize(int, int)));

	CHECKED_CONNECT(
			set,
			SIGNAL(framesPerSecondChanged(QObject*, int, int)),
			this,
			SLOT(setFramesPerSecond(QObject*, int, int)));

    CHECKED_CONNECT(
            set,
            SIGNAL(framesPerSecondChanged(QObject*, int, int)),
            this,
            SLOT(setFramesPerSecond(QObject*, int, int)));

    CHECKED_CONNECT(
            set,
            SIGNAL(panAveragingModeChanged( int, int)),
            this,
            SLOT(setPanAdaptorAveragingMode( int, int)));


    CHECKED_CONNECT(
            set,
            SIGNAL(panDetectorModeChanged( int, int)),
            this,
            SLOT(setPanAdaptorDetectorMode( int, int)));

    CHECKED_CONNECT(
            set,
            SIGNAL(spectrumAveragingCntChanged(QObject*, int , int)),
            this,
            SLOT(setPanAdaptorAveragingCnt(QObject*, int, int)));

	CHECKED_CONNECT(
			set,
			SIGNAL(fftSizeChanged( int , int)),
			this,
			SLOT(setfftSize(int, int)));

	CHECKED_CONNECT(
			set,
			SIGNAL(fmsqLevelChanged( int , int)),
			this,
			SLOT(setfmsqLevel(int, int)));

	CHECKED_CONNECT(
			set,
			SIGNAL(noiseBlankerChanged( int , int)),
			this,
			SLOT(setNoiseBlankerMode(int, int )));

	CHECKED_CONNECT(
			set,
			SIGNAL(noiseFilterChanged( int , int)),
			this,
			SLOT(setNoiseFilterMode(int, int )));

    CHECKED_CONNECT(
            set,
            SIGNAL(nr2AeChanged(int, bool)),
            this,
            SLOT(setNr2Ae(int, bool )));

    CHECKED_CONNECT(
            set,
            SIGNAL(nr2NpeMethodChanged(int, int)),
            this,
            SLOT(setNr2NpeMethod(int, int )));

    CHECKED_CONNECT(
            set,
            SIGNAL(nr2GainMethodChanged(int, int)),
            this,
            SLOT(setNr2GainMethod(int, int )));

    CHECKED_CONNECT(
            set,
            SIGNAL(nrAgcChanged(int, int)),
            this,
            SLOT(setNrAGC(int, int )));

	CHECKED_CONNECT(
			set,
			SIGNAL(anfChanged(int, bool)),
			this,
			SLOT(setanf(int, bool )));

	CHECKED_CONNECT(
			set,
			SIGNAL(snbChanged(int, bool)),
			this,
			SLOT(setsnb(int, bool )));

//	CHECKED_CONNECT(
//		wpagc,
//		SIGNAL(hangLeveldBLineChanged(qreal)),
//		this,
//		SLOT(setAGCHangLeveldBLine(qreal)));
//
//	CHECKED_CONNECT(
//		wpagc,
//		SIGNAL(minimumVoltageChanged(QObject *, int, qreal)),
//		this,
//		SLOT(setAGCThresholdLine(QObject *, int, qreal)));


}


void QWDSPEngine::setQtDSPStatus(bool value) {
	
	m_qtdspOn = value; 
}



int QWDSPEngine::getfftVal(int size) {
	int fftSize;
	switch (size){

		case 0:
			fftSize = 2048;
			break;
		case 1:
			fftSize = 4096;
			break;
		case 2:
			fftSize = 8192;
			break;
		case 3:
			fftSize = 16384;
			break;
		case 4:
			fftSize = 32768;
			break;
		case 5:
			fftSize = 655356;
			break;
		case 6:
			fftSize = 131072;
			break;
		case 7:
			fftSize = 655356;
			break;
		default:
			fftSize = 2048;
			WDSP_ENGINE_DEBUG <<  "invalid fft size set" <<  size;
			break;
	}
	return fftSize;
}


//#define POSTGEN

void QWDSPEngine::set_txrx(RadioState state) {
    bool tone = true;
    switch (state){
        case RadioState::RX:
        if (tone)
        {
#ifdef POSTGEN
            SetTXAPostGenRun(TX_ID,0);
#else
         SetTXAPreGenRun(TX_ID,0);
#endif
        }
            SetChannelState(TX_ID,0,1);
            SetChannelState(0,1,1);
            qDebug() << "RX";
        break;
        case RadioState::TUNE:
        case RadioState::MOX:

            if (tone)
            {
#ifdef POSTGEN
              SetTXAPostGenToneFreq(TX_ID,1000);
              SetTXAPostGenToneMag(TX_ID,0.5);
              SetTXAPostGenMode(TX_ID,0);

//            SetChannelState(0,0,1);

              SetTXAPostGenRun(TX_ID,1);
#else
            SetTXAPreGenToneFreq(TX_ID,1000);
            SetTXAPreGenToneMag(TX_ID,1);
            SetTXAPreGenMode(TX_ID,0);

//            SetChannelState(0,0,1);

            SetTXAPreGenRun(TX_ID,1);

#endif
            }
            SetTXAMode(TX_ID,0);

//            SetTXABandpassFreqs(TX_ID, 100,1000);
            SetTXAPanelGain1(TX_ID,pow(1.0, 10));
            SetTXAPanelRun(TX_ID, 1);
            SetTXABandpassFreqs(TX_ID,1000,1);
            SetTXABandpassWindow(TX_ID,1);
            SetTXABandpassRun(TX_ID,1);
            SetChannelState(TX_ID,1,0);
        case RadioState::DUPLEX:
        default:
         break;
    }
}
