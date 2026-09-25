/**
* @file  cusdr_hamDatabase.cpp
* @brief Ham database implementation for cuSDR
* @author Hermann von Hasseln, DL3HVH
* @version 0.1
* @date 2012-01-27
*/

#include "cusdr_hamDatabase.h"

QList<THamBandFrequencies> getHamBandFrequencies(IARURegion region) {

	QList<THamBandFrequencies> hamBandFreqList;

	THamBandFrequencies hamBandFreq;

    hamBandFreq.frequencyLo = 135700;
    hamBandFreq.frequencyHi = 137800;
    hamBandFreq.hamBand = (HamBand) m2200;
    hamBandFreq.bandString = "2200m";
    hamBandFreq.region = region;
    hamBandFreqList << hamBandFreq;

    hamBandFreq.frequencyLo = 472000;
    hamBandFreq.frequencyHi = 479000;
    hamBandFreq.hamBand = (HamBand) m630;
    hamBandFreq.bandString = "630m";
    hamBandFreq.region = region;
    hamBandFreqList << hamBandFreq;

    hamBandFreq.frequencyLo = (region == region1) ? 1810000 : 1800000;
	hamBandFreq.frequencyHi = 2000000;
	hamBandFreq.hamBand = (HamBand) m160;
	hamBandFreq.bandString = "160m";
	hamBandFreq.region = region;
	hamBandFreqList << hamBandFreq;

	hamBandFreq.frequencyLo = 3500000;
	hamBandFreq.frequencyHi = (region == region2) ? 4000000 : (region == region3 ? 3900000 : 3800000);
	hamBandFreq.hamBand = (HamBand) m80;
	hamBandFreq.bandString = "80m";
	hamBandFreq.region = region;
	hamBandFreqList << hamBandFreq;

	hamBandFreq.frequencyLo = 5260000;
	hamBandFreq.frequencyHi = 5410000;
	hamBandFreq.hamBand = (HamBand) m60;
	hamBandFreq.bandString = "60m";
	hamBandFreq.region = region;
	hamBandFreqList << hamBandFreq;

	hamBandFreq.frequencyLo = 7000000;
	hamBandFreq.frequencyHi = (region == region1) ? 7200000 : 7300000;
	hamBandFreq.hamBand = (HamBand) m40;
	hamBandFreq.bandString = "40m";
	hamBandFreq.region = region;
	hamBandFreqList << hamBandFreq;

	hamBandFreq.frequencyLo = 10100000;
	hamBandFreq.frequencyHi = 10150000;
	hamBandFreq.hamBand = (HamBand) m30;
	hamBandFreq.bandString = "30m";
	hamBandFreq.region = region;
	hamBandFreqList << hamBandFreq;

	hamBandFreq.frequencyLo = 14000000;
	hamBandFreq.frequencyHi = 14350000;
	hamBandFreq.hamBand = (HamBand) m20;
	hamBandFreq.bandString = "20m";
	hamBandFreq.region = region;
	hamBandFreqList << hamBandFreq;

	hamBandFreq.frequencyLo = 18068000;
	hamBandFreq.frequencyHi = 18168000;
	hamBandFreq.hamBand = (HamBand) m17;
	hamBandFreq.bandString = "17m";
	hamBandFreq.region = region;
	hamBandFreqList << hamBandFreq;

	hamBandFreq.frequencyLo = 21000000;
	hamBandFreq.frequencyHi = 21450000;
	hamBandFreq.hamBand = (HamBand) m15;
	hamBandFreq.bandString = "15m";
	hamBandFreq.region = region;
	hamBandFreqList << hamBandFreq;

	hamBandFreq.frequencyLo = 24890000;
	hamBandFreq.frequencyHi = 24990000;
	hamBandFreq.hamBand = (HamBand) m12;
	hamBandFreq.bandString = "12m";
	hamBandFreq.region = region;
	hamBandFreqList << hamBandFreq;

	hamBandFreq.frequencyLo = 28000000;
	hamBandFreq.frequencyHi = 29700000;
	hamBandFreq.hamBand = (HamBand) m10;
	hamBandFreq.bandString = "10m";
	hamBandFreq.region = region;
	hamBandFreqList << hamBandFreq;

	hamBandFreq.frequencyLo = 50000000;
	hamBandFreq.frequencyHi = (region == region1) ? 52000000 : 54000000;
	hamBandFreq.hamBand = (HamBand) m6;
	hamBandFreq.bandString = "6m";
	hamBandFreq.region = region;
	hamBandFreqList << hamBandFreq;

	hamBandFreq.frequencyLo = 144000000;
	hamBandFreq.frequencyHi = (region == region1) ? 146000000 : 148000000;
	hamBandFreq.hamBand = (HamBand) m2;
	hamBandFreq.bandString = "2m";
	hamBandFreq.region = region;
	hamBandFreqList << hamBandFreq;

	if (region == region2) {
		hamBandFreq.frequencyLo = 222000000;
		hamBandFreq.frequencyHi = 225000000;
	} else {
		hamBandFreq.frequencyLo = -1;
		hamBandFreq.frequencyHi = -1;
	}
	hamBandFreq.hamBand = (HamBand) cm125;
	hamBandFreq.bandString = "125cm";
	hamBandFreq.region = region;
	hamBandFreqList << hamBandFreq;

	hamBandFreq.frequencyLo = (region == region2) ? 420000000 : 430000000;
	hamBandFreq.frequencyHi = (region == region1) ? 440000000 : 450000000;
	hamBandFreq.hamBand = (HamBand) cm70;
	hamBandFreq.bandString = "70cm";
	hamBandFreq.region = region;
	hamBandFreqList << hamBandFreq;

	if (region == region2) {
		hamBandFreq.frequencyLo = 902000000;
		hamBandFreq.frequencyHi = 928000000;
	} else {
		hamBandFreq.frequencyLo = -1;
		hamBandFreq.frequencyHi = -1;
	}
	hamBandFreq.hamBand = (HamBand) cm33;
	hamBandFreq.bandString = "33cm";
	hamBandFreq.region = region;
	hamBandFreqList << hamBandFreq;

	hamBandFreq.frequencyLo = 1240000000LL;
	hamBandFreq.frequencyHi = 1300000000LL;
	hamBandFreq.hamBand = (HamBand) cm23;
	hamBandFreq.bandString = "23cm";
	hamBandFreq.region = region;
	hamBandFreqList << hamBandFreq;

	hamBandFreq.frequencyLo = 2300000000LL;
	hamBandFreq.frequencyHi = 2450000000LL;
	hamBandFreq.hamBand = (HamBand) cm13;
	hamBandFreq.bandString = "13cm";
	hamBandFreq.region = region;
	hamBandFreqList << hamBandFreq;

	if (region == region1) {
		hamBandFreq.frequencyLo = 3400000000LL;
		hamBandFreq.frequencyHi = 3475000000LL;
	} else {
		hamBandFreq.frequencyLo = 3300000000LL;
		hamBandFreq.frequencyHi = 3500000000LL;
	}
	hamBandFreq.hamBand = (HamBand) cm10;
	hamBandFreq.bandString = "10cm";
	hamBandFreq.region = region;
	hamBandFreqList << hamBandFreq;

	hamBandFreq.frequencyLo = 5650000000LL;
	hamBandFreq.frequencyHi = 5925000000LL;
	hamBandFreq.hamBand = (HamBand) cm5;
	hamBandFreq.bandString = "5cm";
	hamBandFreq.region = region;
	hamBandFreqList << hamBandFreq;

	hamBandFreq.frequencyLo = 0;
	hamBandFreq.frequencyHi = 61440000;
	hamBandFreq.hamBand = (HamBand) gen;
	hamBandFreq.bandString = "Gen";
	hamBandFreq.region = region;
	hamBandFreqList << hamBandFreq;

	return hamBandFreqList;
}

QList<THamBandText> getHamBandText(IARURegion region) {

	QList<THamBandText> hamBandTextList;

	THamBandText hamBandText;

    hamBandText.frequencyLo = 135700;
    hamBandText.frequencyHi = 135800;
    hamBandText.hamBand = (HamBand) m2200;
    hamBandText.region = region;
    hamBandText.maxBandwith = 200;
    hamBandText.text = "CW only, International DX window";
    hamBandText.shortText = "CW";

    hamBandTextList << hamBandText;

    hamBandText.frequencyLo = 135800;
    hamBandText.frequencyHi = 136000;
    hamBandText.hamBand = (HamBand) m2200;
    hamBandText.region = region;
    hamBandText.maxBandwith = 200;
    hamBandText.text = "CW only, Test transmissions and beacons";
    hamBandText.shortText = "CW";

    hamBandTextList << hamBandText;

    hamBandText.frequencyLo = 136000;
    hamBandText.frequencyHi = 137400;
    hamBandText.hamBand = (HamBand) m2200;
    hamBandText.region = region;
    hamBandText.maxBandwith = 200;
    hamBandText.text = "CW only";
    hamBandText.shortText = "CW";

    hamBandTextList << hamBandText;

    hamBandText.frequencyLo = 137400;
    hamBandText.frequencyHi = 137600;
    hamBandText.hamBand = (HamBand) m2200;
    hamBandText.region = region;
    hamBandText.maxBandwith = 200;
    hamBandText.text = "Narrow band digital modes";
    hamBandText.shortText = "Digital Modes";

    hamBandTextList << hamBandText;

    hamBandText.frequencyLo = 137600;
    hamBandText.frequencyHi = 137800;
    hamBandText.hamBand = (HamBand) m2200;
    hamBandText.region = region;
    hamBandText.maxBandwith = 200;
    hamBandText.text = "Slow CW, QRSS etc.";
    hamBandText.shortText = "Slow CW";

    hamBandTextList << hamBandText;

    hamBandText.frequencyLo = 472000;
    hamBandText.frequencyHi = 479000;
    hamBandText.hamBand = (HamBand) m630;
    hamBandText.region = region;
    hamBandText.maxBandwith = 2100;
    hamBandText.text = "CW, Digi and SSB narrow band modes";
    hamBandText.shortText = "Narrow band modes";

    hamBandTextList << hamBandText;

    if (region != region1) {
        hamBandText.frequencyLo = 1800000;
        hamBandText.frequencyHi = 1810000;
        hamBandText.hamBand = (HamBand) m160;
        hamBandText.region = region;
        hamBandText.maxBandwith = 200;
        hamBandText.text = "CW, Digital modes";
        hamBandText.shortText = "CW/Digital";
        hamBandText.freqTextList.clear();
        hamBandTextList << hamBandText;
    }

    hamBandText.frequencyLo = 1810000;
	hamBandText.frequencyHi = 1838000;
	hamBandText.hamBand = (HamBand) m160;
	hamBandText.region = region;
	hamBandText.maxBandwith = 200;
	hamBandText.text = "CW";
	hamBandText.shortText = "CW";
	hamBandText.freqTextList << "1836 kHz: QRP Centre of Activity";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 1838000;
	hamBandText.frequencyHi = 1840000;
	hamBandText.hamBand = (HamBand) m160;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 500;
	hamBandText.text = "Narrow band modes";
	hamBandText.shortText = "Narrow band modes";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 1840000;
	hamBandText.frequencyHi = 1843000;
	hamBandText.hamBand = (HamBand) m160;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 2700;
	hamBandText.text = "All modes, digimodes, Lowest dial setting for LSB Voice mode: 1843, 3603 and 7053 kHz";
	hamBandText.shortText = "All modes";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 1843000;
	hamBandText.frequencyHi = 2000000;
	hamBandText.hamBand = (HamBand) m160;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 2700;
	hamBandText.text = "All modes, Lowest dial setting for LSB Voice mode: 1843, 3603 and 7053 kHz";
	hamBandText.shortText = "All modes";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 3500000;
	hamBandText.frequencyHi = 3510000;
	hamBandText.hamBand = (HamBand) m80;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 200;
	hamBandText.text = "CW, priority for intercontinental operation";
	hamBandText.shortText = "CW";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 3510000;
	hamBandText.frequencyHi = 3560000;
	hamBandText.hamBand = (HamBand) m80;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 200;
	hamBandText.text = "CW, contest preferred";
	hamBandText.shortText = "CW";
	hamBandText.freqTextList << "3555 kHz: QRS Centre of Activity";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 3560000;
	hamBandText.frequencyHi = 3580000;
	hamBandText.hamBand = (HamBand) m80;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 200;
	hamBandText.text = "CW";
	hamBandText.shortText = "CW";
	hamBandText.freqTextList << "3560 kHz: QRP Centre of Activity";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 3580000;
	hamBandText.frequencyHi = 3590000;
	hamBandText.hamBand = (HamBand) m80;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 500;
	hamBandText.text = "Narrow band modes, digimodes";
	hamBandText.shortText = "Narrow band modes";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 3590000;
	hamBandText.frequencyHi = 3600000;
	hamBandText.hamBand = (HamBand) m80;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 500;
	hamBandText.text = "Narrow band modes, digimodes, automatically controlled data stations (unattended)";
	hamBandText.shortText = "Narrow band modes";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 3600000;
	hamBandText.frequencyHi = 3620000;
	hamBandText.hamBand = (HamBand) m80;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 200;
	hamBandText.text = "All modes, digimodes, automatically controlled data station (unattended)";
	hamBandText.shortText = "All modes";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 3620000;
	hamBandText.frequencyHi = 3650000;
	hamBandText.hamBand = (HamBand) m80;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 2700;
	hamBandText.text = "All modes";
	hamBandText.shortText = "All modes";
	hamBandText.freqTextList << "3630 kHz: Digital Voice Centre of Activity, SSB contest preferred";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 3650000;
	hamBandText.frequencyHi = 3700000;
	hamBandText.hamBand = (HamBand) m80;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 2700;
	hamBandText.text = "All modes";
	hamBandText.shortText = "All modes";
	hamBandText.freqTextList << "3690 kHz: SSB QRP Centre of Activity";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 3700000;
	hamBandText.frequencyHi = 3775000;
	hamBandText.hamBand = (HamBand) m80;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 2700;
	hamBandText.text = "All modes, SSB contest preferred";
	hamBandText.shortText = "All modes";
	hamBandText.freqTextList << "3735 kHz: Image Centre of Activity";
	hamBandText.freqTextList << "3760 kHz: Region 1 Emergency Centre of Activity";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 3775000;
	hamBandText.frequencyHi = 3800000;
	hamBandText.hamBand = (HamBand) m80;
	hamBandText.region = region;
	hamBandText.maxBandwith = 2700;
	hamBandText.text = "All modes, priority for intercontinental operation";
	hamBandText.shortText = "All modes";

	hamBandTextList << hamBandText;

	if (region == region2) {
		hamBandText.frequencyLo = 3800000;
		hamBandText.frequencyHi = 4000000;
		hamBandText.hamBand = (HamBand) m80;
		hamBandText.region = region;
		hamBandText.maxBandwith = 2700;
		hamBandText.text = "All modes, Phone (75m)";
		hamBandText.shortText = "Phone (75m)";
		hamBandText.freqTextList.clear();
		hamBandTextList << hamBandText;
	} else if (region == region3) {
		hamBandText.frequencyLo = 3800000;
		hamBandText.frequencyHi = 3900000;
		hamBandText.hamBand = (HamBand) m80;
		hamBandText.region = region;
		hamBandText.maxBandwith = 2700;
		hamBandText.text = "All modes, Phone (80m)";
		hamBandText.shortText = "Phone (80m)";
		hamBandText.freqTextList.clear();
		hamBandTextList << hamBandText;
	}

	hamBandText.frequencyLo = 7000000;
	hamBandText.frequencyHi = 7040000;
	hamBandText.hamBand = (HamBand) m40;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 200;
	hamBandText.text = "CW";
	hamBandText.shortText = "CW";
	hamBandText.freqTextList << "7030 kHz: QRP Centre of Activity";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 7040000;
	hamBandText.frequencyHi = 7047000;
	hamBandText.hamBand = (HamBand) m40;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 500;
	hamBandText.text = "Narrow band modes, digimodes";
	hamBandText.shortText = "Narrow band modes";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 7047000;
	hamBandText.frequencyHi = 7050000;
	hamBandText.hamBand = (HamBand) m40;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 500;
	hamBandText.text = "Narrow band modes, digimodes, automatically controlled data stations (unattended)";
	hamBandText.shortText = "Narrow band modes";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 7050000;
	hamBandText.frequencyHi = 7053000;
	hamBandText.hamBand = (HamBand) m40;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 2700;
	hamBandText.text = "All modes, digimodes, automatically controlled data stations (unattended)";
	hamBandText.shortText = "All modes";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 7053000;
	hamBandText.frequencyHi = 7060000;
	hamBandText.hamBand = (HamBand) m40;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 2700;
	hamBandText.text = "All modes, digimodes";
	hamBandText.shortText = "All modes";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 7060000;
	hamBandText.frequencyHi = 7100000;
	hamBandText.hamBand = (HamBand) m40;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 2700;
	hamBandText.text = "All modes, SSB contest preferred";
	hamBandText.shortText = "All modes";
	hamBandText.freqTextList << "7070 kHz: Digital Voice Centre of Activity";
	hamBandText.freqTextList << "7090 kHz: SSB QRP Centre of Activity";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 7100000;
	hamBandText.frequencyHi = 7130000;
	hamBandText.hamBand = (HamBand) m40;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 2700;
	hamBandText.text = "All modes";
	hamBandText.shortText = "All modes";
	hamBandText.freqTextList << "7110 kHz: Region 1 Emergency Centre of Activity";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 7130000;
	hamBandText.frequencyHi = 7175000;
	hamBandText.hamBand = (HamBand) m40;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 2700;
	hamBandText.text = "All modes, SSB contest preferred";
	hamBandText.shortText = "All modes";
	hamBandText.freqTextList << "7165 kHz: Image Centre of Activity";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 7175000;
	hamBandText.frequencyHi = 7200000;
	hamBandText.hamBand = (HamBand) m40;
	hamBandText.region = region;
	hamBandText.maxBandwith = 2700;
	hamBandText.text = "All modes, priority for intercontinental operation";
	hamBandText.shortText = "All modes";

	hamBandTextList << hamBandText;

	if (region != region1) {
		hamBandText.frequencyLo = 7200000;
		hamBandText.frequencyHi = 7300000;
		hamBandText.hamBand = (HamBand) m40;
		hamBandText.region = region;
		hamBandText.maxBandwith = 2700;
		hamBandText.text = "All modes, Phone";
		hamBandText.shortText = "Phone/All";
		hamBandText.freqTextList.clear();
		hamBandTextList << hamBandText;
	}

	hamBandText.frequencyLo = 10100000;
	hamBandText.frequencyHi = 10140000;
	hamBandText.hamBand = (HamBand) m30;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 200;
	hamBandText.text = "CW";
	hamBandText.shortText = "CW";
	hamBandText.freqTextList << "10116 kHz: QRP Centre of Activity";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 10140000;
	hamBandText.frequencyHi = 10150000;
	hamBandText.hamBand = (HamBand) m30;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 500;
	hamBandText.text = "Narrow band modes, digimodes";
	hamBandText.shortText = "Narrow band modes";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 14000000;
	hamBandText.frequencyHi = 14060000;
	hamBandText.hamBand = (HamBand) m20;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 200;
	hamBandText.text = "CW, contest preferred;";
	hamBandText.shortText = "CW";
	hamBandText.freqTextList << "14055 kHz: QRS Centre of Activity";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 14060000;
	hamBandText.frequencyHi = 14070000;
	hamBandText.hamBand = (HamBand) m20;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 200;
	hamBandText.text = "CW, 14060 kHz, QRP Centre of Activity";
	hamBandText.shortText = "CW";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 14070000;
	hamBandText.frequencyHi = 14089000;
	hamBandText.hamBand = (HamBand) m20;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 500;
	hamBandText.text = "Narrow band modes, digimodes";
	hamBandText.shortText = "Narrow band modes";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 14089000;
	hamBandText.frequencyHi = 14099000;
	hamBandText.hamBand = (HamBand) m20;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 500;
	hamBandText.text = "Narrow band modes, digimodes, automatically controlled data stations (unattended)";
	hamBandText.shortText = "Narrow band modes";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 14099000;
	hamBandText.frequencyHi = 14101000;
	hamBandText.hamBand = (HamBand) m20;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 0;
	hamBandText.text = "IBP, exclusively for beacons";
	hamBandText.shortText = "IBP";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 14101000;
	hamBandText.frequencyHi = 14112000;
	hamBandText.hamBand = (HamBand) m20;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 2700;
	hamBandText.text = "All modes, digimodes, automatically controlled data stations (unattended)";
	hamBandText.shortText = "All modes";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 14112000;
	hamBandText.frequencyHi = 14125000;
	hamBandText.hamBand = (HamBand) m20;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 2700;
	hamBandText.text = "All modes";
	hamBandText.shortText = "All modes";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 14125000;
	hamBandText.frequencyHi = 14300000;
	hamBandText.hamBand = (HamBand) m20;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 2700;
	hamBandText.text = "All modes, SSB contest preferred";
	hamBandText.shortText = "All modes";
	hamBandText.freqTextList << "14130 kHz: Digital Voice Centre of Activity";
	hamBandText.freqTextList << "14195 kHz � 5 kHz: Priority for Dxpeditions";
	hamBandText.freqTextList << "14230 kHz: Image Centre of Activity";
	hamBandText.freqTextList << "14285 kHz: SSB QRP Centre of Activity";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 14300000;
	hamBandText.frequencyHi = 14350000;
	hamBandText.hamBand = (HamBand) m20;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 2700;
	hamBandText.text = "All modes";
	hamBandText.shortText = "All modes";
	hamBandText.freqTextList << "14300 kHz: Global Emergency centre of activity";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 18068000;
	hamBandText.frequencyHi = 18095000;
	hamBandText.hamBand = (HamBand) m17;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 200;
	hamBandText.text = "CW";
	hamBandText.shortText = "CW";
	hamBandText.freqTextList << "18086 kHz: QRP Centre of Activity";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 18095000;
	hamBandText.frequencyHi = 18105000;
	hamBandText.hamBand = (HamBand) m17;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 500;
	hamBandText.text = "Narrow band modes, digimodes";
	hamBandText.shortText = "Narrow band modes";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 18105000;
	hamBandText.frequencyHi = 18109000;
	hamBandText.hamBand = (HamBand) m17;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 500;
	hamBandText.text = "Narrow band modes, digimodes, automatically controlled data stations (unattended)";
	hamBandText.shortText = "Narrow band modes";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 18109000;
	hamBandText.frequencyHi = 18111000;
	hamBandText.hamBand = (HamBand) m17;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 0;
	hamBandText.text = "IBP, exclusively for beacons";
	hamBandText.shortText = "IBP";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 18111000;
	hamBandText.frequencyHi = 18120000;
	hamBandText.hamBand = (HamBand) m17;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 2700;
	hamBandText.text = "IBP, exclusively for beacons";
	hamBandText.shortText = "IBP";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 18120000;
	hamBandText.frequencyHi = 18168000;
	hamBandText.hamBand = (HamBand) m17;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 2700;
	hamBandText.text = "All modes";
	hamBandText.shortText = "All modes";
	hamBandText.freqTextList << "18130 kHz: SSB QRP Centre of Activity";
	hamBandText.freqTextList << "18150 kHz: Digital Voice Centre of Activity";
	hamBandText.freqTextList << "18160 kHz: Global Emergency Centre of Activity";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 21000000;
	hamBandText.frequencyHi = 21070000;
	hamBandText.hamBand = (HamBand) m15;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 200;
	hamBandText.text = "CW";
	hamBandText.shortText = "CW";
	hamBandText.freqTextList << "21055 kHz: QRS Centre of Activity";
	hamBandText.freqTextList << "21060 kHz: QRP Centre of Activity";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 21070000;
	hamBandText.frequencyHi = 21090000;
	hamBandText.hamBand = (HamBand) m15;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 500;
	hamBandText.text = "Narrow band modes, digimodes";
	hamBandText.shortText = "Narrow band modes";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 21090000;
	hamBandText.frequencyHi = 21110000;
	hamBandText.hamBand = (HamBand) m15;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 500;
	hamBandText.text = "Narrow band modes, digimodes, automatically controlled data stations (unattended)";
	hamBandText.shortText = "Narrow band modes";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 21110000;
	hamBandText.frequencyHi = 21120000;
	hamBandText.hamBand = (HamBand) m15;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 2700;
	hamBandText.text = "All modes (excluding SSB), digimodes, automatically controlled data stations (unattended)";
	hamBandText.shortText = "All modes (excluding SSB)";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 21120000;
	hamBandText.frequencyHi = 21149000;
	hamBandText.hamBand = (HamBand) m15;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 500;
	hamBandText.text = "Narrow band modes";
	hamBandText.shortText = "Narrow band modes";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 21149000;
	hamBandText.frequencyHi = 21151000;
	hamBandText.hamBand = (HamBand) m15;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 0;
	hamBandText.text = "IBP, exclusively for beacons";
	hamBandText.shortText = "IBP";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 21151000;
	hamBandText.frequencyHi = 21450000;
	hamBandText.hamBand = (HamBand) m15;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 2700;
	hamBandText.text = "All modes";
	hamBandText.shortText = "All modes";
	hamBandText.freqTextList << "21180 kHz: Digital Voice Centre of Activity";
	hamBandText.freqTextList << "21285 kHz: SSB QRP Centre of Activity";
	hamBandText.freqTextList << "21340 kHz: Image Centre of Activity";
	hamBandText.freqTextList << "21360 kHz: Global Emergency Centre of Activity";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 24890000;
	hamBandText.frequencyHi = 24915000;
	hamBandText.hamBand = (HamBand) m12;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 200;
	hamBandText.text = "CW, 24906 kHz, QRP centre of activity";
	hamBandText.shortText = "CW";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 24915000;
	hamBandText.frequencyHi = 24925000;
	hamBandText.hamBand = (HamBand) m12;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 500;
	hamBandText.text = "Narrow band modes, digimodes";
	hamBandText.shortText = "Narrow band modes";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 24925000;
	hamBandText.frequencyHi = 24929000;
	hamBandText.hamBand = (HamBand) m12;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 500;
	hamBandText.text = "Narrow band modes, digimodes, automatically controlled data stations (unattended)";
	hamBandText.shortText = "Narrow band modes";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 24929000;
	hamBandText.frequencyHi = 24931000;
	hamBandText.hamBand = (HamBand) m12;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 0;
	hamBandText.text = "IBP, exclusively for beacons";
	hamBandText.shortText = "IBP";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 24931000;
	hamBandText.frequencyHi = 24940000;
	hamBandText.hamBand = (HamBand) m12;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 2700;
	hamBandText.text = "All modes, digimodes, automatically controlled data stations (unattended)";
	hamBandText.shortText = "All modes";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 24940000;
	hamBandText.frequencyHi = 24990000;
	hamBandText.hamBand = (HamBand) m12;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 2700;
	hamBandText.text = "All modes, 24960 kHz: Digital Voice Centre of Activity";
	hamBandText.shortText = "All modes";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 28000000;
	hamBandText.frequencyHi = 28070000;
	hamBandText.hamBand = (HamBand) m10;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 200;
	hamBandText.text = "CW, 28055 kHz: QRS Centre of Activity";
	hamBandText.shortText = "CW";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 28070000;
	hamBandText.frequencyHi = 28120000;
	hamBandText.hamBand = (HamBand) m10;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 500;
	hamBandText.text = "Narrow band modes, digimodes";
	hamBandText.shortText = "Narrow band modes";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 28120000;
	hamBandText.frequencyHi = 28150000;
	hamBandText.hamBand = (HamBand) m10;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 500;
	hamBandText.text = "Narrow band modes, digimodes, automatically controlled data stations (unattended)";
	hamBandText.shortText = "Narrow band modes";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 28150000;
	hamBandText.frequencyHi = 28190000;
	hamBandText.hamBand = (HamBand) m10;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 500;
	hamBandText.text = "Narrow band modes";
	hamBandText.shortText = "Narrow band modes";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 28190000;
	hamBandText.frequencyHi = 28199000;
	hamBandText.hamBand = (HamBand) m10;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 0;
	hamBandText.text = "IBP, regional time shared beacons";
	hamBandText.shortText = "IBP";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 28199000;
	hamBandText.frequencyHi = 28201000;
	hamBandText.hamBand = (HamBand) m10;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 0;
	hamBandText.text = "IBP, worldwide time shared beacons";
	hamBandText.shortText = "IBP";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 28201000;
	hamBandText.frequencyHi = 28225000;
	hamBandText.hamBand = (HamBand) m10;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 0;
	hamBandText.text = "IBP, continuous duty beacons";
	hamBandText.shortText = "IBP";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 28225000;
	hamBandText.frequencyHi = 28300000;
	hamBandText.hamBand = (HamBand) m10;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 2700;
	hamBandText.text = "All modes, beacons";
	hamBandText.shortText = "All modes, beacons";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 28300000;
	hamBandText.frequencyHi = 28320000;
	hamBandText.hamBand = (HamBand) m10;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 2700;
	hamBandText.text = "All modes, digimodes, automatically controlled data stations (unattended)";
	hamBandText.shortText = "All modes";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 28320000;
	hamBandText.frequencyHi = 29100000;
	hamBandText.hamBand = (HamBand) m10;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 2700;
	hamBandText.text = "All modes";
	hamBandText.shortText = "All modes";
	hamBandText.freqTextList << "28330 kHz: Digital Voice Centre of Activity";
	hamBandText.freqTextList << "28360 kHz: SSB QRP Centre of Activity";
	hamBandText.freqTextList << "28680 kHz: Image Centre of Activity";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 29100000;
	hamBandText.frequencyHi = 29200000;
	hamBandText.hamBand = (HamBand) m10;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 6000;
	hamBandText.text = "All modes, FM simplex: 10 kHz channels";
	hamBandText.shortText = "All modes";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 29200000;
	hamBandText.frequencyHi = 29300000;
	hamBandText.hamBand = (HamBand) m10;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 6000;
	hamBandText.text = "All modes, digimodes, automatically controlled data stations (unattended)";
	hamBandText.shortText = "All modes";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 29300000;
	hamBandText.frequencyHi = 29510000;
	hamBandText.hamBand = (HamBand) m10;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 6000;
	hamBandText.text = "Satellite-downlink";
	hamBandText.shortText = "Satellite-downlink";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 29510000;
	hamBandText.frequencyHi = 29520000;
	hamBandText.hamBand = (HamBand) m10;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 0;
	hamBandText.text = "Guard channel";
	hamBandText.shortText = "Guard channel";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 29520000;
	hamBandText.frequencyHi = 29590000;
	hamBandText.hamBand = (HamBand) m10;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 6000;
	hamBandText.text = "All modes, FM repeater input (RH1 to RH8)";
	hamBandText.shortText = "All modes";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 29590000;
	hamBandText.frequencyHi = 29600000;
	hamBandText.hamBand = (HamBand) m10;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 6000;
	hamBandText.text = "All modes, FM calling channel";
	hamBandText.shortText = "All modes";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 29600000;
	hamBandText.frequencyHi = 29610000;
	hamBandText.hamBand = (HamBand) m10;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 6000;
	hamBandText.text = "All modes, FM simplex repeater (parrot, input and output)";
	hamBandText.shortText = "All modes";

	hamBandTextList << hamBandText;

	hamBandText.frequencyLo = 29610000;
	hamBandText.frequencyHi = 29700000;
	hamBandText.hamBand = (HamBand) m10;
	hamBandText.region = (IARURegion) region1;
	hamBandText.maxBandwith = 6000;
	hamBandText.text = "All modes, FM repeater outputs (RH1 to RH8)";
	hamBandText.shortText = "All modes";

    hamBandTextList << hamBandText;

    hamBandText.frequencyLo = 50000000;
    hamBandText.frequencyHi = 50020000;
    hamBandText.hamBand = (HamBand) m6;
    hamBandText.region = (IARURegion) region1;
    hamBandText.maxBandwith = 2700;
    hamBandText.text = "CW";
    hamBandText.shortText = "All modes";

    hamBandTextList << hamBandText;

    hamBandText.frequencyLo = 50020000;
    hamBandText.frequencyHi = 50080000;
    hamBandText.hamBand = (HamBand) m6;
    hamBandText.region = (IARURegion) region1;
    hamBandText.maxBandwith = 2700;
    hamBandText.text = "Beacons";
    hamBandText.shortText = "All modes";
    hamBandText.freqTextList << "50.2937  kHz: WSPR";
    hamBandText.freqTextList << "50.2760 JT65";
    hamBandText.freqTextList << "50.3130 FT8";

	hamBandTextList << hamBandText;

    hamBandText.frequencyLo = 50080000;
    hamBandText.frequencyHi = 52000000;
    hamBandText.hamBand = (HamBand) m6;
    hamBandText.region = region;
    hamBandText.maxBandwith = 2700;
    hamBandText.text = "All Modes";
    hamBandText.shortText = "All modes";
    hamBandText.freqTextList.clear();
    hamBandTextList << hamBandText;

    if (region != region1) {
        hamBandText.frequencyLo = 52000000;
        hamBandText.frequencyHi = 53000000;
        hamBandText.hamBand = (HamBand) m6;
        hamBandText.region = region;
        hamBandText.maxBandwith = 2700;
        hamBandText.text = "All Modes, Simplex, Digital";
        hamBandText.shortText = "All modes";
        hamBandText.freqTextList.clear();
        hamBandTextList << hamBandText;

        hamBandText.frequencyLo = 53000000;
        hamBandText.frequencyHi = 54000000;
        hamBandText.hamBand = (HamBand) m6;
        hamBandText.region = region;
        hamBandText.maxBandwith = 5000;
        hamBandText.text = "FM Repeaters";
        hamBandText.shortText = "FM Repeaters";
        hamBandText.freqTextList.clear();
        hamBandTextList << hamBandText;
    }

    // 2m
    hamBandText.frequencyLo = 144000000;
    hamBandText.frequencyHi = 144100000;
    hamBandText.hamBand = (HamBand) m2;
    hamBandText.region = region;
    hamBandText.maxBandwith = 500;
    hamBandText.text = "CW, Telegraphy, EME";
    hamBandText.shortText = "CW/EME";
    hamBandText.freqTextList.clear();
    hamBandTextList << hamBandText;

    hamBandText.frequencyLo = 144100000;
    hamBandText.frequencyHi = 144400000;
    hamBandText.hamBand = (HamBand) m2;
    hamBandText.region = region;
    hamBandText.maxBandwith = 2700;
    hamBandText.text = "SSB, CW, Telegraphy";
    hamBandText.shortText = "SSB/CW";
    hamBandText.freqTextList.clear();
    hamBandTextList << hamBandText;

    hamBandText.frequencyLo = 144400000;
    hamBandText.frequencyHi = 144990000;
    hamBandText.hamBand = (HamBand) m2;
    hamBandText.region = region;
    hamBandText.maxBandwith = 500;
    hamBandText.text = "Propagation Beacons, Digital";
    hamBandText.shortText = "Beacons/Digi";
    hamBandText.freqTextList.clear();
    hamBandTextList << hamBandText;

    hamBandText.frequencyLo = 145000000;
    hamBandText.frequencyHi = 145800000;
    hamBandText.hamBand = (HamBand) m2;
    hamBandText.region = region;
    hamBandText.maxBandwith = 12500;
    hamBandText.text = "FM Repeaters, Simplex";
    hamBandText.shortText = "FM/Repeaters";
    hamBandText.freqTextList.clear();
    hamBandTextList << hamBandText;

    hamBandText.frequencyLo = 145800000;
    hamBandText.frequencyHi = 146000000;
    hamBandText.hamBand = (HamBand) m2;
    hamBandText.region = region;
    hamBandText.maxBandwith = 12500;
    hamBandText.text = "Amateur Satellite Service";
    hamBandText.shortText = "Satellite";
    hamBandText.freqTextList.clear();
    hamBandTextList << hamBandText;

    if (region != region1) {
        hamBandText.frequencyLo = 146000000;
        hamBandText.frequencyHi = 148000000;
        hamBandText.hamBand = (HamBand) m2;
        hamBandText.region = region;
        hamBandText.maxBandwith = 20000;
        hamBandText.text = "FM Repeaters, Simplex, Digital";
        hamBandText.shortText = "FM/Repeaters";
        hamBandText.freqTextList.clear();
        hamBandTextList << hamBandText;
    }

    // 1.25m (Region 2 only)
    if (region == region2) {
        hamBandText.frequencyLo = 222000000;
        hamBandText.frequencyHi = 225000000;
        hamBandText.hamBand = (HamBand) cm125;
        hamBandText.region = region;
        hamBandText.maxBandwith = 20000;
        hamBandText.text = "All modes, FM repeaters, Simplex";
        hamBandText.shortText = "FM/All";
        hamBandText.freqTextList.clear();
        hamBandTextList << hamBandText;
    }

    // 70cm
    if (region == region2) {
        hamBandText.frequencyLo = 420000000;
        hamBandText.frequencyHi = 430000000;
        hamBandText.hamBand = (HamBand) cm70;
        hamBandText.region = region;
        hamBandText.maxBandwith = 20000;
        hamBandText.text = "ATV, Repeaters, Experimental";
        hamBandText.shortText = "ATV/Exp";
        hamBandText.freqTextList.clear();
        hamBandTextList << hamBandText;
    }

    hamBandText.frequencyLo = 430000000;
    hamBandText.frequencyHi = 440000000;
    hamBandText.hamBand = (HamBand) cm70;
    hamBandText.region = region;
    hamBandText.maxBandwith = 20000;
    hamBandText.text = "All modes, Simplex, Repeaters, Satellites";
    hamBandText.shortText = "All modes";
    hamBandText.freqTextList.clear();
    hamBandTextList << hamBandText;

    if (region != region1) {
        hamBandText.frequencyLo = 440000000;
        hamBandText.frequencyHi = 450000000;
        hamBandText.hamBand = (HamBand) cm70;
        hamBandText.region = region;
        hamBandText.maxBandwith = 20000;
        hamBandText.text = "FM Repeaters, Simplex, Digital links";
        hamBandText.shortText = "FM/Repeaters";
        hamBandText.freqTextList.clear();
        hamBandTextList << hamBandText;
    }

    // 33cm (Region 2 only)
    if (region == region2) {
        hamBandText.frequencyLo = 902000000;
        hamBandText.frequencyHi = 928000000;
        hamBandText.hamBand = (HamBand) cm33;
        hamBandText.region = region;
        hamBandText.maxBandwith = 20000;
        hamBandText.text = "All modes, Weak signal, FM repeaters";
        hamBandText.shortText = "Weak signal/FM";
        hamBandText.freqTextList.clear();
        hamBandTextList << hamBandText;
    }

    // 23cm
    hamBandText.frequencyLo = 1240000000LL;
    hamBandText.frequencyHi = 1300000000LL;
    hamBandText.hamBand = (HamBand) cm23;
    hamBandText.region = region;
    hamBandText.maxBandwith = 20000;
    hamBandText.text = "All modes, ATV, Weak signal, Satellites";
    hamBandText.shortText = "All modes";
    hamBandText.freqTextList.clear();
    hamBandTextList << hamBandText;

    // 13cm
    hamBandText.frequencyLo = 2300000000LL;
    hamBandText.frequencyHi = 2450000000LL;
    hamBandText.hamBand = (HamBand) cm13;
    hamBandText.region = region;
    hamBandText.maxBandwith = 20000;
    hamBandText.text = "All modes, Amateur satellite, Broadband";
    hamBandText.shortText = "All modes";
    hamBandText.freqTextList.clear();
    hamBandTextList << hamBandText;

    // 10cm
    hamBandText.frequencyLo = (region == region1) ? 3400000000LL : 3300000000LL;
    hamBandText.frequencyHi = (region == region1) ? 3475000000LL : 3500000000LL;
    hamBandText.hamBand = (HamBand) cm10;
    hamBandText.region = region;
    hamBandText.maxBandwith = 20000;
    hamBandText.text = "All modes, Weak signal, Satellites";
    hamBandText.shortText = "All modes";
    hamBandText.freqTextList.clear();
    hamBandTextList << hamBandText;

    // 5cm
    hamBandText.frequencyLo = 5650000000LL;
    hamBandText.frequencyHi = 5925000000LL;
    hamBandText.hamBand = (HamBand) cm5;
    hamBandText.region = region;
    hamBandText.maxBandwith = 20000;
    hamBandText.text = "All modes, Amateur satellite, Broadband";
    hamBandText.shortText = "All modes";
    hamBandText.freqTextList.clear();
    hamBandTextList << hamBandText;

    for (int i = 0; i < hamBandTextList.size(); ++i) {
        hamBandTextList[i].region = region;
    }

    return hamBandTextList;
}

QList<TDefaultFilter> getDefaultFilterFrequencies() {

	QList<TDefaultFilter> defaultFilters;

	TDefaultFilter defaultFilter;

	defaultFilter.dspMode = (DSPMode) LSB;
	defaultFilter.defaultFilterMode = filterLSB;
	defaultFilter.filterLo = -3050.0f;
	defaultFilter.filterHi = -150.0f;

	defaultFilters << defaultFilter;

	defaultFilter.dspMode = (DSPMode) USB;
	defaultFilter.defaultFilterMode = filterUSB;
	defaultFilter.filterLo = 150.0f;
	defaultFilter.filterHi = 3050.0f;

	defaultFilters << defaultFilter;

	defaultFilter.dspMode = (DSPMode) DSB;
	defaultFilter.defaultFilterMode = filterDSB;
	defaultFilter.filterLo = -3300.0f;
	defaultFilter.filterHi = 3300.0f;

	defaultFilters << defaultFilter;

	defaultFilter.dspMode = (DSPMode) CWL;
	defaultFilter.defaultFilterMode = filterCWL;
	defaultFilter.filterLo = -1100.0f;
	defaultFilter.filterHi = -100.0f;

	defaultFilters << defaultFilter;

	defaultFilter.dspMode = (DSPMode) CWU;
	defaultFilter.defaultFilterMode = filterCWU;
	defaultFilter.filterLo = 100.0f;
	defaultFilter.filterHi = 1100.0f;

	defaultFilters << defaultFilter;

	defaultFilter.dspMode = (DSPMode) FMN;
	defaultFilter.defaultFilterMode = filterFMN;
	defaultFilter.filterLo = -2000.0f;
	defaultFilter.filterHi = 2000.0f;

	defaultFilters << defaultFilter;

	defaultFilter.dspMode = (DSPMode) AM;
	defaultFilter.defaultFilterMode = filterAM;
	defaultFilter.filterLo = -4000.0f;
	defaultFilter.filterHi = 4000.0f;

	defaultFilters << defaultFilter;

	defaultFilter.dspMode = (DSPMode) DIGU;
	defaultFilter.defaultFilterMode = filterDIGU;
	defaultFilter.filterLo = 150.0f;
	defaultFilter.filterHi = 3050.0f;

	defaultFilters << defaultFilter;

	defaultFilter.dspMode = (DSPMode) SPEC;
	defaultFilter.defaultFilterMode = filterSPEC;
	defaultFilter.filterLo = -6000.0f;
	defaultFilter.filterHi = 6000.0f;

	defaultFilters << defaultFilter;

	defaultFilter.dspMode = (DSPMode) DIGL;
	defaultFilter.defaultFilterMode = filterDIGL;
	defaultFilter.filterLo = -3050.0f;
	defaultFilter.filterHi = -150.0f;

	defaultFilters << defaultFilter;

	defaultFilter.dspMode = (DSPMode) SAM;
	defaultFilter.defaultFilterMode = filterSAM;
	defaultFilter.filterLo = -3300.0f;
	defaultFilter.filterHi = 3300.0f;

	defaultFilters << defaultFilter;

	defaultFilter.dspMode = (DSPMode) FDV;
	defaultFilter.defaultFilterMode = filterFDV;
	defaultFilter.filterLo = -6000.0f;
	defaultFilter.filterHi = 6000.0f;

	defaultFilters << defaultFilter;

	return defaultFilters;
}

QList<QList<THamBandDefaults> > getHamBandDefaults(IARURegion region) {

	QList<QList<THamBandDefaults> > hamBandDefaults;

	QList<THamBandDefaults> hamBandDefault;

	THamBandDefaults defaults;

	defaults.hamBand = (HamBand) m160;
	defaults.dspMode = (DSPMode) CWL;
	defaults.frequencyLo = 1810000;
	hamBandDefault << defaults;

	defaults.hamBand = (HamBand) m160;
	defaults.dspMode = (DSPMode) CWU;
	defaults.frequencyLo = 1835000;
	hamBandDefault << defaults;

	defaults.hamBand = (HamBand) m160;
	defaults.dspMode = (DSPMode) USB;
	defaults.frequencyLo = 1845000;
	hamBandDefault << defaults;

	hamBandDefaults << hamBandDefault;

	hamBandDefault.clear();
	defaults.hamBand = (HamBand) m80;
	defaults.dspMode = (DSPMode) CWL;
	defaults.frequencyLo = 3501000;
	hamBandDefault << defaults;

	defaults.hamBand = (HamBand) m80;
	defaults.dspMode = (DSPMode) LSB;
	defaults.frequencyLo = (region == region1) ? 3650000 : 3751000;
	hamBandDefault << defaults;

	defaults.hamBand = (HamBand) m80;
	defaults.dspMode = (DSPMode) LSB;
	defaults.frequencyLo = (region == region1) ? 3751000 : 3850000;
	hamBandDefault << defaults;

	hamBandDefaults << hamBandDefault;

	hamBandDefault.clear();
	defaults.hamBand = (HamBand) m60;
	defaults.dspMode = (DSPMode) USB;
	defaults.frequencyLo = 5258500;
	hamBandDefault << defaults;

	hamBandDefaults << hamBandDefault;

	hamBandDefault.clear();
	defaults.hamBand = (HamBand) m40;
	defaults.dspMode = (DSPMode) CWL;
	defaults.frequencyLo = 7001000;
	hamBandDefault << defaults;

	defaults.hamBand = (HamBand) m40;
	defaults.dspMode = (DSPMode) LSB;
	defaults.frequencyLo = 7152000;
	hamBandDefault << defaults;

	if (region != region1) {
		defaults.hamBand = (HamBand) m40;
		defaults.dspMode = (DSPMode) LSB;
		defaults.frequencyLo = 7250000;
		hamBandDefault << defaults;
	}

	hamBandDefaults << hamBandDefault;

	defaults.hamBand = (HamBand) m30;
	defaults.dspMode = (DSPMode) CWU;
	defaults.frequencyLo = 10120000;
	hamBandDefault << defaults;

	hamBandDefaults << hamBandDefault;

	defaults.hamBand = (HamBand) m20;
	defaults.dspMode = (DSPMode) CWU;
	defaults.frequencyLo = 14010000;
	hamBandDefault << defaults;

	defaults.hamBand = (HamBand) m20;
	defaults.dspMode = (DSPMode) USB;
	defaults.frequencyLo = 14230000;
	hamBandDefault << defaults;

	hamBandDefaults << hamBandDefault;

	defaults.hamBand = (HamBand) m17;
	defaults.dspMode = (DSPMode) CWU;
	defaults.frequencyLo = 18090000;
	hamBandDefault << defaults;

	defaults.hamBand = (HamBand) m17;
	defaults.dspMode = (DSPMode) USB;
	defaults.frequencyLo = 18125000;
	hamBandDefault << defaults;

	hamBandDefaults << hamBandDefault;

	defaults.hamBand = (HamBand) m15;
	defaults.dspMode = (DSPMode) CWU;
	defaults.frequencyLo = 21001000;
	hamBandDefault << defaults;

	defaults.hamBand = (HamBand) m15;
	defaults.dspMode = (DSPMode) USB;
	defaults.frequencyLo = 21255000;
	hamBandDefault << defaults;

	hamBandDefaults << hamBandDefault;

	defaults.hamBand = (HamBand) m12;
	defaults.dspMode = (DSPMode) CWU;
	defaults.frequencyLo = 24895000;
	hamBandDefault << defaults;

	defaults.hamBand = (HamBand) m12;
	defaults.dspMode = (DSPMode) USB;
	defaults.frequencyLo = 24900000;
	hamBandDefault << defaults;

	hamBandDefaults << hamBandDefault;

	defaults.hamBand = (HamBand) m10;
	defaults.dspMode = (DSPMode) CWU;
	defaults.frequencyLo = 28010000;
	hamBandDefault << defaults;

	defaults.hamBand = (HamBand) m10;
	defaults.dspMode = (DSPMode) USB;
	defaults.frequencyLo = 28300000;
	hamBandDefault << defaults;

	hamBandDefaults << hamBandDefault;

	defaults.hamBand = (HamBand) m6;
	defaults.dspMode = (DSPMode) CWU;
	defaults.frequencyLo = 50010000;
	hamBandDefault << defaults;

	defaults.hamBand = (HamBand) m6;
	defaults.dspMode = (DSPMode) USB;
	defaults.frequencyLo = 50125000;
	hamBandDefault << defaults;

    defaults.hamBand = (HamBand) m6;
    defaults.dspMode = (DSPMode) FMN;
    defaults.frequencyLo = (region == region1) ? 51500000 : 53000000;
    hamBandDefault << defaults;

    hamBandDefaults << hamBandDefault;

	// 2m
	hamBandDefault.clear();
	defaults.hamBand = (HamBand) m2;
	defaults.dspMode = (DSPMode) CWU;
	defaults.frequencyLo = 144050000;
	hamBandDefault << defaults;

	defaults.hamBand = (HamBand) m2;
	defaults.dspMode = (DSPMode) USB;
	defaults.frequencyLo = 144200000;
	hamBandDefault << defaults;

	defaults.hamBand = (HamBand) m2;
	defaults.dspMode = (DSPMode) FMN;
	defaults.frequencyLo = (region == region1) ? 145500000 : 146520000;
	hamBandDefault << defaults;
	hamBandDefaults << hamBandDefault;

	// 1.25m (Region 2 only)
	if (region == region2) {
		hamBandDefault.clear();
		defaults.hamBand = (HamBand) cm125;
		defaults.dspMode = (DSPMode) CWU;
		defaults.frequencyLo = 222100000;
		hamBandDefault << defaults;

		defaults.hamBand = (HamBand) cm125;
		defaults.dspMode = (DSPMode) USB;
		defaults.frequencyLo = 222300000;
		hamBandDefault << defaults;

		defaults.hamBand = (HamBand) cm125;
		defaults.dspMode = (DSPMode) FMN;
		defaults.frequencyLo = 223500000;
		hamBandDefault << defaults;
		hamBandDefaults << hamBandDefault;
	}

	// 70cm
	hamBandDefault.clear();
	defaults.hamBand = (HamBand) cm70;
	defaults.dspMode = (DSPMode) USB;
	defaults.frequencyLo = 432100000;
	hamBandDefault << defaults;

	defaults.hamBand = (HamBand) cm70;
	defaults.dspMode = (DSPMode) FMN;
	defaults.frequencyLo = (region == region1) ? 433500000 : 446000000;
	hamBandDefault << defaults;
	hamBandDefaults << hamBandDefault;

	// 33cm (Region 2 only)
	if (region == region2) {
		hamBandDefault.clear();
		defaults.hamBand = (HamBand) cm33;
		defaults.dspMode = (DSPMode) USB;
		defaults.frequencyLo = 902100000;
		hamBandDefault << defaults;

		defaults.hamBand = (HamBand) cm33;
		defaults.dspMode = (DSPMode) FMN;
		defaults.frequencyLo = 927500000;
		hamBandDefault << defaults;
		hamBandDefaults << hamBandDefault;
	}

	// 23cm
	hamBandDefault.clear();
	defaults.hamBand = (HamBand) cm23;
	defaults.dspMode = (DSPMode) USB;
	defaults.frequencyLo = 1296100000LL;
	hamBandDefault << defaults;

	defaults.hamBand = (HamBand) cm23;
	defaults.dspMode = (DSPMode) FMN;
	defaults.frequencyLo = 1294500000LL;
	hamBandDefault << defaults;
	hamBandDefaults << hamBandDefault;

	hamBandDefault.clear();

	defaults.hamBand = (HamBand) gen;
	defaults.dspMode = (DSPMode) SAM;
	defaults.frequencyLo = 590000;
	hamBandDefault << defaults;

	defaults.hamBand = (HamBand) gen;
	defaults.dspMode = (DSPMode) SAM;
	defaults.frequencyLo = 3850000;
	hamBandDefault << defaults;

	defaults.hamBand = (HamBand) gen;
	defaults.dspMode = (DSPMode) SAM;
	defaults.frequencyLo = 5975000;
	hamBandDefault << defaults;

	defaults.hamBand = (HamBand) gen;
	defaults.dspMode = (DSPMode) SAM;
	defaults.frequencyLo = 9550000;
	hamBandDefault << defaults;

	defaults.hamBand = (HamBand) gen;
	defaults.dspMode = (DSPMode) SAM;
	defaults.frequencyLo = 13845000;
	hamBandDefault << defaults;

	hamBandDefaults << hamBandDefault;

	return hamBandDefaults;
}

HamBand getBandFromFrequency(const QList<THamBandFrequencies> &bandList, qint64 frequency) {

	HamBand band;

	for (int i = 0; i < bandList.size(); ++i) {
		
		if (bandList.at(i).frequencyLo <= frequency && bandList.at(i).frequencyHi >= frequency) {
			band = bandList.at(i).hamBand;
			return band;
		}
	}
	return (HamBand) gen;
	
}

TDefaultFilter getFilterFromDSPMode(const QList<TDefaultFilter> &filterList, DSPMode mode) {

	TDefaultFilter filter;

	for (int i = 0; i < filterList.size(); ++i) {

		if (filterList.at(i).dspMode == mode) {

			filter = filterList.at(i);
			return filter;
		}
	}
	return filterList.at(0);
}

TDefaultFilter resolvedTxPassband(const QList<TDefaultFilter> &filterList,
                                        DSPMode mode, double lo, double hi)
{
	double a = lo;
	double b = hi;
	if (b > a) {
		if (isUpperSidebandMode(mode) && b <= 0.0) {
			a = -hi;
			b = -lo;
		} else if (isLowerSidebandMode(mode) && a >= 0.0) {
			a = -hi;
			b = -lo;
		}
	}
	if (b > a) {
		TDefaultFilter live;
		live.dspMode = mode;
		live.filterLo = a;
		live.filterHi = b;
		return live;
	}
	return getFilterFromDSPMode(filterList, mode);
}

QString getHamBandTextString(const QList<THamBandText> &textList, bool shortText, qint64 frequency) {

	QString str = "";

	for (int i = 0; i < textList.size(); ++i) {

		if (textList.at(i).frequencyLo <= frequency && textList.at(i).frequencyHi >= frequency) {

			if (shortText)
				str = textList.at(i).shortText;
			else
				str = textList.at(i).text;
			
			return str;
		}
		else
			str = "Out of Band";
	}
	return str;
}

