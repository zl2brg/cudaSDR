
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



void QWDSPEngine::processDSP(CPX &in, CPX &out) {

int error;
	m_mutex.lock();
    fexchange0(m_rx, (double *) in.data(),  (double *) out.data(), &error);
    if(error!=0) {
        WDSP_ENGINE_DEBUG << "WDSP channel read error " << error;
    }
    else  Spectrum0(1, m_rx, 0, 0, (double *) in.data());
	m_mutex.unlock();

}


double QWDSPEngine::getSMeterInstValue() {

    return  GetRXAMeter(m_rx,RXA_S_AV);

}

void QWDSPEngine::setVolume(float value) {
    SetRXAPanelGain1(m_rx,  (double) value);
    WDSP_ENGINE_DEBUG << "WDSP volume set to " <<value;
}

void QWDSPEngine::setQtDSPStatus(bool value) { 
	
	m_qtdspOn = value; 
}

void QWDSPEngine::setDSPMode(DSPMode mode) {

	m_dspmode = mode;
	WDSP_ENGINE_DEBUG << "WDSP mode set to " << mode;
	SetRXAMode(m_rx, mode);

}

void QWDSPEngine::setAGCMode(AGCMode agc) {
		SetRXAAGCMode(m_rx, agc);
		//SetRXAAGCThresh(rx->id, agc_thresh_point, 4096.0, rx->sample_rate);
		SetRXAAGCSlope(m_rx,m_agcSlope);
	//	SetRXAAGCTop(m_rx,m_agcMaximumGain);
		switch(agc) {
			case agcOFF:
				break;
			case agcLONG:
				SetRXAAGCAttack(m_rx,2);
				SetRXAAGCHang(m_rx,2000);
				SetRXAAGCDecay(m_rx,2000);
				SetRXAAGCHangThreshold(m_rx, m_agcHangThreshold);
				break;
			case agcSLOW:
				SetRXAAGCAttack(m_rx,2);
				SetRXAAGCHang(m_rx,1000);
				SetRXAAGCDecay(m_rx,500);
				SetRXAAGCHangThreshold(m_rx,m_agcHangThreshold);
				break;
			case agcMED:
				SetRXAAGCAttack(m_rx,2);
				SetRXAAGCHang(m_rx,0);
				SetRXAAGCDecay(m_rx,250);
				SetRXAAGCHangThreshold(m_rx,100);
				break;
			case agcFAST:
				SetRXAAGCAttack(m_rx,2);
				SetRXAAGCHang(m_rx,0);
				SetRXAAGCDecay(m_rx,50);
				SetRXAAGCHangThreshold(m_rx,100);
				break;

			case agcUser:
				SetRXAAGCAttack(m_rx,m_agcAttackTime);
				SetRXAAGCHang(m_rx,0);
				SetRXAAGCDecay(m_rx,m_agcDecayTime);
				SetRXAAGCHangThreshold(m_rx,m_agcHangThreshold);
				break;
		}
	emit setAGCLineValues(m_rx);
	WDSP_ENGINE_DEBUG << "Set AGC Mode " << agc;

}

void QWDSPEngine::setAGCAttackTime(int rx, int value) {
    if (m_rx != rx) return;
	m_agcAttackTime = value;
}

void QWDSPEngine::setAGCDecayTime(int rx, int value) {
    if (m_rx != rx) return;
	m_agcDecayTime = value;
}

void QWDSPEngine::setAGCSlope(int rx, int value) {
    if (m_rx != rx) return;
	m_agcSlope = value;
}


void QWDSPEngine::setAGCMaximumGain(qreal value) {
	SetRXAAGCTop(m_rx, (double)value);
	m_agcMaximumGain = value;
    WDSP_ENGINE_DEBUG << "Set AGCMaximum gain " << value;
	emit setAGCLineValues(m_rx);
}

void QWDSPEngine::setAGCHangThreshold(int rx, double value) {
    if (m_rx != rx) return;
	m_agcHangLevel = value;
   	WDSP_ENGINE_DEBUG << "Set AGC Hang Threshold " << value;
}

void QWDSPEngine::setAGCLineValues(int rx) {
    if (m_rx != rx) return;
    double hang;
    double thresh;

    GetRXAAGCHangLevel(m_rx, &hang);
    GetRXAAGCThresh(m_rx, &thresh, 2048, (double)m_samplerate);

    if ((hang != m_agcHangLevel) || (thresh != m_agcHangThreshold))
	{
		m_agcHangLevel = hang;
		m_agcThreshold = thresh;
		emit set->agcLineLevelsChanged(this,m_rx,thresh,hang);
		WDSP_ENGINE_DEBUG << "Set AGC line value" << hang;

	}

//    qreal noiseOffset = 10.0 * log10(qAbs(filter->filterHi() - filter->filterLo()) * 2 * m_size / m_samplerate);
//    qreal threshold = 20.0 * log10(thresh) - noiseOffset + AGCOFFSET;


}


void QWDSPEngine::setAGCHangLevel(double level) {

	SetRXAAGCHangLevel(m_rx,level);
	WDSP_ENGINE_DEBUG << "Set AGC line value" << level;

}


void QWDSPEngine::setAGCThreshold(double threshold) {

	SetRXAAGCThresh(m_rx,threshold,2048,this->m_samplerate);
	emit setAGCLineValues(m_rx);
	WDSP_ENGINE_DEBUG << "Set AGC threshold " << threshold;
}

void QWDSPEngine::setAGCHangTime(int value) {

	SetRXAAGCHang(m_rx,value);
	WDSP_ENGINE_DEBUG << "Set AGC Hang time" << value;

}


void QWDSPEngine::setSampleRate(QObject *sender, int value) {

	Q_UNUSED(sender)

	if (m_samplerate == value) return;

	switch (value) {

		case 48000:
			m_samplerate = value;
			break;

		case 96000:
			m_samplerate = value;
			break;

		case 192000:
			m_samplerate = value;
			break;

		case 384000:
			m_samplerate = value;
			break;

		default:
			WDSP_ENGINE_DEBUG << "invalid sample rate (possible values are: 48, 96, 192, or 384 kHz)!\n";
			break;
	}
 	SetChannelState(m_rx,0,1);
	SetInputSamplerate(m_rx,m_samplerate);

	init_analyzer(m_refreshrate);
    SetEXTANBSamplerate (m_rx, m_samplerate);
    SetEXTNOBSamplerate (m_rx, m_samplerate);
	SetChannelState(m_rx,1,0);
	WDSP_ENGINE_DEBUG << "sample rate set to " << m_samplerate ;

}


void QWDSPEngine:: setFilter(double low,double high) {


	if(m_dspmode == FMN) {
		SetRXAFMDeviation(m_rx, (double)8000.0);
		}
	RXASetPassband(m_rx,low,high);
	emit setAGCLineValues(m_rx);
    WDSP_ENGINE_DEBUG << "Set Filter:Low  " <<  low << "High " << high;
}


void QWDSPEngine::setNCOFrequency(int rx, long ncoFreq) {

	if (getQtDSPStatus() == 0 ) return;

	if (m_rx != rx) return;

	if(ncoFreq==0) {
		SetRXAShiftFreq(m_rx, (double)ncoFreq);
//		RXANBPSetShiftFrequency(m_rx, (double)ncoFreq);
		SetRXAShiftRun(m_rx, 0);
	} else {
		SetRXAShiftFreq(m_rx, (double)ncoFreq);
//		RXANBPSetShiftFrequency(m_rx, (double)ncoFreq);
		SetRXAShiftRun(m_rx, 1);
	}
}

void QWDSPEngine::setSampleSize(int rx, int size) {

	if (m_rx == rx) {

		m_mutex.lock();
		m_spectrumSize = size;
		WDSP_ENGINE_DEBUG <<  "Set sample size" <<  size;
		m_mutex.unlock();
	}
}

void QWDSPEngine::ProcessFrequencyShift(CPX &in, CPX &out) {


}

void QWDSPEngine::init_analyzer(int refreshrate) {
	int flp[] = {0};
	double keep_time = 0.1;
	int n_pixout = 1;
	int spur_elimination_ffts = 1;
	int data_type = 1;
	int fft_size = m_fftSize;
	int window_type = 6;
	double kaiser_pi = 14.0;
	int overlap;
	int clip = 0;
	int span_clip_l = 0;
	int span_clip_h = 0;
	int pixels = 4096;
	int stitches = 1;
	int calibration_data_set = 0;
	double span_min_freq = 0.0;
	double span_max_freq = 0.0;

	int max_w = fft_size + (int) min(keep_time * refreshrate, keep_time * (double) fft_size * (double) refreshrate);

	overlap = (int) max(0.0, ceil(fft_size - (double) m_samplerate / (double) refreshrate));

	fprintf(stderr, "SetAnalyzer id=%d buffer_size=%d overlap=%d\n fft%d\n", m_rx, m_size, overlap,m_fftSize);


	SetAnalyzer(m_rx,
				n_pixout,
				spur_elimination_ffts, //number of LO frequencies = number of ffts used in elimination
				data_type, //0 for real input data (I only); 1 for complex input data (I & Q)
				flp, //vector with one elt for each LO frequency, 1 if high-side LO, 0 otherwise
				fft_size, //size of the fft, i.e., number of input samples
				1024, //number of samples transferred for each OpenBuffer()/CloseBuffer()
				window_type, //integer specifying which window function to use
				kaiser_pi, //PiAlpha parameter for Kaiser window
				overlap, //number of samples each fft (other than the first) is to re-use from the previous
				clip, //number of fft output bins to be clipped from EACH side of each sub-span
				span_clip_l, //number of bins to clip from low end of entire span
				span_clip_h, //number of bins to clip from high end of entire span
				pixels, //number of pixel values to return.  may be either <= or > number of bins
				stitches, //number of sub-spans to concatenate to form a complete span
				calibration_data_set, //identifier of which set of calibration data to use
				span_min_freq, //frequency at first pixel value8192
				span_max_freq, //frequency at last pixel value
				max_w //max samples to hold in input ring buffers
	);
}



void QWDSPEngine::setFramesPerSecond(QObject* sender, int rx, int value){

	Q_UNUSED(sender)
	if (rx != m_rx) return;
	m_mutex.lock();
	m_refreshrate = value;
    init_analyzer(value);
    calcDisplayAveraging();
    SetDisplayAvBackmult(rx, 0, m_display_avb);
    SetDisplayNumAverage(rx, 0, m_display_average);
    m_mutex.unlock();
    WDSP_ENGINE_DEBUG <<  "SetFramesPerSecond" <<  value;
}


void QWDSPEngine::setPanAdaptorAveragingMode(int rx, int mode) {
    if (rx != m_rx) return;
    WDSP_ENGINE_DEBUG <<  "Setpan av mode" <<  mode;
    SetDisplayAverageMode(m_rx,0,mode);
    calcDisplayAveraging();
    SetDisplayAvBackmult(rx, 0, m_display_avb);
    SetDisplayNumAverage(rx, 0, m_display_average);
}


void QWDSPEngine::setPanAdaptorDetectorMode(int rx, int mode) {
    if (rx != m_rx) return;
    WDSP_ENGINE_DEBUG <<  "Setpan av det  mode" <<  mode;
    SetDisplayDetectorMode(rx,0,mode);

}

void QWDSPEngine::setPanAdaptorAveragingCnt(QObject* sender, int rx, int count){
    Q_UNUSED(sender);
    if (rx != m_rx) return;
    m_averageCount = count;
    calcDisplayAveraging();
    SetDisplayAvBackmult(rx, 0, m_display_avb);
    SetDisplayNumAverage(rx, 0, m_display_average);
    WDSP_ENGINE_DEBUG <<  "Setpan av count mode" <<  m_display_avb << " " << m_display_average;
}

void QWDSPEngine::calcDisplayAveraging() {
    double t=0.001*m_averageCount;
    m_display_avb = exp(-1.0 / ((double)m_refreshrate * t));
    m_display_average = max(2, (int)min(60, (double)m_refreshrate * t));
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


void QWDSPEngine::setfftSize(int rx, int value) {
	if (rx != m_rx) return;
	m_fftSize = getfftVal(value);
	WDSP_ENGINE_DEBUG <<  "mfftsize set" <<  m_fftSize;
	m_mutex.lock();
	init_analyzer(value);
	calcDisplayAveraging();
	SetDisplayAvBackmult(rx, 0, m_display_avb);
	SetDisplayNumAverage(rx, 0, m_display_average);
	m_mutex.unlock();

}


void QWDSPEngine::setfmsqLevel(int rx, int value) {
	if (rx != m_rx) return;
	double threshold = pow(10.0,-2.0 * value/100.0);
	WDSP_ENGINE_DEBUG <<  "fmSqLevel set" <<  value;
	SetRXAFMSQThreshold(m_rx, threshold);

}

void QWDSPEngine::setFilterMode(int rx) {
    if (rx != m_rx) return;
	switch (m_nbMode) {
		case 0:
			m_nb = m_nb2 = 0;
			break;
		case 1:
			m_nb = 1;
			m_nb2 = 0;
			break;
		case 2:
			m_nb = 0;
			m_nb2 = 1;
			break;

		default:
			WDSP_ENGINE_DEBUG << "invalid nb mode" << m_nbMode;
			break;
	}

	switch (m_nrMode) {

		case 0:
			m_nr = m_nr2 = 0;
			break;
		case 1:
			m_nr = 1;
			m_nr2 = 0;
			break;
		case 2:
			m_nr = 0;
			m_nr2 = 1;
			break;

		default:
			WDSP_ENGINE_DEBUG <<  "invalid nr mode" <<  m_nrMode;
			break;
	}

	SetRXAEMNRPosition(m_rx,m_nr_agc);
	SetRXAEMNRaeRun(m_rx, m_nr2_ae);
	SetRXAEMNRnpeMethod(m_rx,m_nr2_npe_method);
	SetRXAEMNRgainMethod(m_rx,m_nr2_gain_method);
	SetEXTANBRun(rx, m_nb);
 	SetEXTNOBRun(rx, m_nb2);
  	SetRXAANRRun(rx, m_nr);
  	SetRXAEMNRRun(rx, m_nr2);
  	SetRXAANFRun(rx, m_anf);
  	SetRXASNBARun(rx, m_snb);
    WDSP_ENGINE_DEBUG <<  "nb mode" <<  m_nb;
    WDSP_ENGINE_DEBUG <<  "nb2mode" <<  m_nb2;
    WDSP_ENGINE_DEBUG <<  "nf mode" <<  m_nr;
    WDSP_ENGINE_DEBUG <<  "nr2 mode" <<  m_nr2;
    WDSP_ENGINE_DEBUG <<  "anf mode" <<  m_anf;
    WDSP_ENGINE_DEBUG <<  "snb mode" <<  m_anf;
}

void QWDSPEngine::setNoiseBlankerMode(int rx, int nb) {
	if (rx != m_rx) return;
	m_nbMode = nb;
	WDSP_ENGINE_DEBUG << "nb mode" << nb;
	setFilterMode(rx);
}


void QWDSPEngine::setNoiseFilterMode(int rx, int nr) {
	m_nrMode = nr;
	setFilterMode(rx);
}

void QWDSPEngine::setNr2Ae(int rx, bool value) {
    if (rx != m_rx) return;
    m_nr2_ae = value;
    SetRXAEMNRaeRun(m_rx, m_nr2_ae);
}

void QWDSPEngine::setNr2GainMethod(int rx, int value) {
    if (rx != m_rx) return;
    m_nr2_gain_method = value;
    SetRXAEMNRgainMethod(m_rx,m_nr2_gain_method);
}

void QWDSPEngine::setNr2NpeMethod(int rx, int value) {
    if (rx != m_rx) return;
    m_nr2_npe_method = value;
    SetRXAEMNRnpeMethod(m_rx,m_nr2_npe_method);
}

void QWDSPEngine::setNrAGC(int rx, int value) {
    if (rx != m_rx) return;
    m_nr_agc = value;
    SetRXAEMNRPosition(m_rx,m_nr_agc);
}


void QWDSPEngine::setanf(int rx, bool value) {
	if (rx != m_rx) return;
	m_anf = value;
	WDSP_ENGINE_DEBUG <<  "anf mode" <<  value;
	SetRXAANFRun(rx, m_anf);
}

void QWDSPEngine::setsnb(int rx, bool value) {
	m_snb = value;
	WDSP_ENGINE_DEBUG <<  "	snb mode" <<  value;
	SetRXASNBARun(rx, m_snb);
}

//#define POSTGEN

void QWDSPEngine::set_txrx(RadioState state) {
    bool tone = false;
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
            SetTXAPreGenToneMag(TX_ID,0.5);
            SetTXAPreGenMode(TX_ID,0);

//            SetChannelState(0,0,1);

            SetTXAPreGenRun(TX_ID,1);

#endif
            }
            SetTXAMode(TX_ID,6);

            SetTXABandpassFreqs(TX_ID, 100,1000);
            SetTXAPanelGain1(TX_ID,pow(10.0, 20/20.0));
            SetTXAPanelRun(TX_ID, 1);
            SetChannelState(TX_ID,1,0);
            qDebug() << "TX";

        case RadioState::DUPLEX:
        default:
         break;
    }
}
