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

/** Point marker (FT8 / WSPR / QRP CoA / EiBi SWL schedule, …) — frequency label. */
struct BandSpot {
	qint64 freqHz = 0;
	QString label;
	QString itu;
	QString lang;
	QString target;
	QString timeUtc;
	int startMin = -1;  // UTC minute of day (0..1440), -1 if 24/7 or unconstrained
	int endMin = -1;    // UTC minute of day (0..1440), -1 if 24/7 or unconstrained
	QString days;
	QString mode;
	int snr = 0;
	int wpm = 0;
	QString spotter;
	qint64 timestampSec = 0;
	int ttlSec = 0; // 0 = permanent/static spot; >0 = dynamic spot TTL in seconds

	bool isExpired(qint64 currentSec) const {
		if (ttlSec <= 0)
			return false;
		return (currentSec - timestampSec) > ttlSec;
	}

	bool isActiveAt(int utcMinOfDay, int dayOfWeek = 0) const {
		if (dayOfWeek >= 1 && dayOfWeek <= 7 && !days.isEmpty()) {
			if (days == QLatin1String("1-5") && dayOfWeek > 5)
				return false;
			if (days == QLatin1String("67") && dayOfWeek < 6)
				return false;
			if (days == QLatin1String("1-6") && dayOfWeek > 6)
				return false;
			if (days.size() == 1 && days.at(0).isDigit()) {
				const int d = days.at(0).digitValue();
				if (d >= 1 && d <= 7 && d != dayOfWeek)
					return false;
			}
			if (days.contains(QLatin1String("-"))) {
				const QStringList parts = days.split(QLatin1Char('-'));
				if (parts.size() == 2) {
					const int d1 = parts[0].toInt();
					const int d2 = parts[1].toInt();
					if (d1 >= 1 && d2 <= 7 && (dayOfWeek < d1 || dayOfWeek > d2))
						return false;
				}
			} else if (days.contains(QString::number(dayOfWeek))) {
				// Matched digit in set (e.g. "135")
			} else if (!days.contains(QLatin1Char('-')) && days.contains(QLatin1Char('1')) && days.contains(QLatin1Char('2'))) {
				// Other day string format
			}
		}

		if (startMin < 0 || endMin < 0)
			return true;
		if (utcMinOfDay < 0)
			return true;

		if (startMin <= endMin)
			return utcMinOfDay >= startMin && utcMinOfDay <= endMin;
		// Schedule crosses midnight (e.g. 2200-0200)
		return utcMinOfDay >= startMin || utcMinOfDay <= endMin;
	}
};

#endif
