
/**
* @file  qtwdsp_dspEngine.h
* @brief header file for QtDSP
* @author Hermann von Hasseln, DL3HVH
* @version 0.1
* @date 2012-04-07
*/

/*   
 *   Copyright (C) 2007, 2008, 2009, 2010 Philip A Covington, N8VB
 *
 *	 adapted for QtDSP by (C) 2012 Hermann von Hasseln, DL3HVH
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

#ifndef _QTWDSP_DSP_ENGINE_H
#define _QTWDSP_DSP_ENGINE_H

#define TX_ID 8
#define AGCOFFSET (-18.0)//-63.0

//#include <QObject>
//#include <QThread>
//#include <QMetaType>
//#include <QMutexLocker>
//#include <QMutex>
//#include <QWaitCondition>
//#include <QVariant>
//#include <QElapsedTimer>
extern "C" {
#include    <wdsp.h>
}

#include "../cusdr_settings.h"
#include "QtDSP/qtdsp_qComplex.h"


#ifdef LOG_WDSP_ENGINE
#define  WDSP_ENGINE_DEBUG qDebug().nospace() << "WDSPEngine::\t"
#else
#   define WDSP_ENGINE_DEBUG nullDebug()
#endif

#define min(X,Y) ((X) < (Y) ?  (X) : (Y))
#define max(X,Y) ((X) < (Y) ?  (Y) : (X))


class QWDSPEngine : public QObject {

	Q_OBJECT

public:
	explicit QWDSPEngine(QObject* parent = nullptr, int rx = 0, int size = 0);
    ~QWDSPEngine() override;


	void processDSP(CPX &in, CPX &out);

	double	getSMeterInstValue();
    void    init_analyzer(int refreshrate);
    void    calcDisplayAveraging();
	int  	getfftVal(int size);

	int     spectrumDataReady;
    qVectorFloat spectrumBuffer;

public slots:
	bool getQtDSPStatus() { return m_qtdspOn; }
	
	void    setNCOFrequency(int rx, long value);
	void    setSampleRate(QObject *sender, int value);
	void    setSampleSize(int rx, int size);
	void    setQtDSPStatus(bool value);
	void    setVolume(float value);
	void    setDSPMode(DSPMode mode);
	void    setAGCMode(AGCMode mode);
    void    setFilter(double  low,double high);
	void	setAGCMaximumGain(qreal);
	void    setAGCHangThreshold(int rx, double);
	//void	setAGCHangLeveldBLine(qreal value);
	//void	setAGCThresholdLine(QObject *sender, int rx, qreal value);
    void    setAGCLineValues(int rx);
	void    setAGCThreshold( double threshold);
	void    setAGCHangTime(int hang);
	void 	setAGCHangLevel(double level);
	void    setAGCAttackTime(int rx, int value);
    void    setAGCDecayTime(int rx, int value);
    void    setAGCSlope(int rx, int value);
    void 	setFramesPerSecond(QObject* sender, int rx, int value);
	void    setPanAdaptorAveragingMode( int rx, int value);
    void    setPanAdaptorDetectorMode( int rx, int value);
    void    setPanAdaptorAveragingCnt(QObject*, int rx , int value);
    void 	setfftSize(int rx, int value);
	void 	setfmsqLevel(int rx, int value);
	void 	setFilterMode(int rx);
	void 	setNoiseBlankerMode(int rx, int nb);
	void 	setNoiseFilterMode(int rx, int nr);
	void    setNrAGC(int rx , int value);
    void    setNr2GainMethod(int rx , int value);
    void    setNr2NpeMethod(int rx , int value);
    void    setNr2Ae(int rx , bool value);
    void    setanf(int rx, bool value);
    void    setsnb(int rx, bool value);
    void    set_txrx(RadioState state);




private:
	Settings*	set;
	TReceiver	m_rxData;
	AGCMode		m_agcMode;

	QMutex	m_mutex;

	bool	m_qtdspOn;

	int		m_rx;
	int		m_size;
	int		m_spectrumSize;
	int		m_samplerate;
	int		m_fftMultiplier;
	int 	m_fftSize;
	int     m_refreshrate;
	int     m_averageCount;
    double  m_display_avb;
    int     m_display_average;
    int     m_PanAvMode;
    int     m_PanDetMode;
   	float	m_volume;
   	int     m_agcSlope;
   	qreal   m_agcThreshold;
   	int     m_agcHangThreshold;
   	double  m_agcHangLevel;
   	double   m_agcMaximumGain;
   	int     m_agcAttackTime;
   	int     m_agcDecayTime;
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



    void ProcessFrequencyShift(CPX &in, CPX &out);
	void	setupConnections();


private slots:

};

#endif // _QTDSP_DSP_ENGINE_H
