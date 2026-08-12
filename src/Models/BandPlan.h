#ifndef BANDPLAN_H
#define BANDPLAN_H

#include <QColor>
#include <QString>
#include <QtGlobal>

/** One contiguous allocation from SDR-Band-Plans (SDR# RangeEntry). */
struct BandRange {
	qint64 loHz = 0;
	qint64 hiHz = 0;
	QString name;
	QString mode;
	int stepHz = 0;
	QColor color;
};

/** Point marker (FT8 / WSPR / QRP CoA, …) — KiwiSDR-style frequency label. */
struct BandSpot {
	qint64 freqHz = 0;
	QString label;
};

#endif
