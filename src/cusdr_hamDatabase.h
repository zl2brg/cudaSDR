/**
* @file  cusdr_hamDatabase.h
* @brief Ham database header file for cuSDR
* @author Hermann von Hasseln, DL3HVH
* @version 0.1
* @date 2012-01-27
*/

/*   
 *   Copyright 2011, 2012 Hermann von Hasseln, DL3HVH
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
 
#ifndef CUSDR_HAMDATABASE_H
#define CUSDR_HAMDATABASE_H

#include <QList>



typedef enum _iaruRegion {

  region1,
  region2,
  region3

} IARURegion;

#include <QDebug>
Q_DECLARE_METATYPE (_iaruRegion)

typedef enum _hamBand {

  m2200,        //  0
  m630,         //  1
  m160,			//  2
  m80,			//  3
  m60,			//  4
  m40,			//  5
  m30,			//  6
  m20,			//  7
  m17,			//  8
  m15,			//  9
  m12,			// 10
  m10,			// 11
  m6,			// 12
  m2,			// 13
  cm125,		// 14
  cm70,			// 15
  cm33,			// 16
  cm23,			// 17
  cm13,			// 18
  cm10,			// 19
  cm5,			// 20
  gen			// 21

} HamBand;

typedef enum _dspMode {

  LSB,			//  0
  USB,			//  1
  DSB,			//  2
  CWL,			//  3
  CWU,			//  4
  FMN,			//  5
  AM,			//  6
  DIGU,			//  7
  SPEC,			//  8
  DIGL,			//  9
  SAM,			// 10
  FDV			// 11  (app FreeDV; resolveWDSPMode remaps to LSB/USB)

} DSPMode;

// Resolve the logical app mode to the actual WDSP mode.
// FDV (FreeDV) uses LSB below 10 MHz and USB above; all other modes pass through unchanged.
inline DSPMode resolveWDSPMode(DSPMode appMode, qint64 frequency) {
    return (appMode == FDV)
        ? (frequency < 10000000L ? LSB : USB)
        : appMode;
}

typedef enum _adcMode {

	adc1, 
	adc2

} ADCMode;

typedef enum _agcMode {

	agcOFF, 
	agcLONG, 
	agcSLOW, 
	agcMED, 
	agcFAST,
	agcUser

} AGCMode;

typedef enum _defaultFilterMode {

	filterLSB,
	filterUSB,
	filterDSB,
	filterCWL,
	filterCWU,
	filterFMN,
	filterAM,
	filterDIGU,
	filterSPEC,
	filterDIGL,
	filterSAM,
	filterFDV,
	filterFREEDV
	
} TDefaultFilterMode;

Q_DECLARE_METATYPE (HamBand)
Q_DECLARE_METATYPE (DSPMode)
Q_DECLARE_METATYPE (ADCMode)
Q_DECLARE_METATYPE (AGCMode)
Q_DECLARE_METATYPE (TDefaultFilterMode)

typedef struct _filter {

	DSPMode dspMode;
	//QRadio::_DSPMode dspMode;
	TDefaultFilterMode defaultFilterMode;
	qreal filterLo;
	qreal filterHi;

} TDefaultFilter;

typedef struct _hamBandFrequencies {

	HamBand		hamBand;
	IARURegion	region;
	
	QString		bandString;
	qint64		frequencyLo;
	qint64		frequencyHi;

} THamBandFrequencies;

typedef struct _hamBandText {

	HamBand		hamBand;
	IARURegion	region;

	qint64	frequencyLo;
	qint64	frequencyHi;
	int		maxBandwith;

	QString			text;
	QString			shortText;
	QStringList		freqTextList;

} THamBandText;

typedef struct _hamBandDefaults {

	HamBand	hamBand;
	DSPMode	dspMode;

	qint64	frequencyLo;

} THamBandDefaults;

//***********************************************************************


// Ham database query and table accessor functions
QList<THamBandFrequencies> getHamBandFrequencies();
QList<THamBandText> getHamBandText();
QList<TDefaultFilter> getDefaultFilterFrequencies();
QList<QList<THamBandDefaults> > getHamBandDefaults();

HamBand getBandFromFrequency(const QList<THamBandFrequencies> &bandList, qint64 frequency);
TDefaultFilter getFilterFromDSPMode(const QList<TDefaultFilter> &filterList, DSPMode mode);
TDefaultFilter resolvedTxPassband(const QList<TDefaultFilter> &filterList,
                                  DSPMode mode, double lo, double hi);
QString getHamBandTextString(const QList<THamBandText> &textList, bool shortText, qint64 frequency);

inline bool isLowerSidebandMode(DSPMode mode)
{
	return mode == LSB || mode == DIGL || mode == CWL;
}

inline bool isUpperSidebandMode(DSPMode mode)
{
	return mode == USB || mode == DIGU || mode == CWU;
}

#endif // CUSDR_HAMDATABASE_H
