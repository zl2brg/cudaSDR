#ifndef BANDPLANMANAGER_H
#define BANDPLANMANAGER_H

#include "BandPlan.h"

#include <QObject>
#include <QVector>

/**
 * Loads SDR# / SDR-Band-Plans XML allocation bars plus digimode / Kiwi DX
 * spot markers. HamBand hardware paths are unchanged.
 */
class BandPlanManager : public QObject {
	Q_OBJECT

public:
	explicit BandPlanManager(QObject *parent = nullptr);

	bool loadFromResource(const QString &resourcePath);
	bool loadFromFile(const QString &filePath);
	bool loadFromData(const QByteArray &xml);

	/** Load AetherSDR-style {"spots":[{"freq":MHz,"label":"..."}, ...]} JSON. */
	bool loadSpotsFromResource(const QString &resourcePath);
	bool loadSpotsFromData(const QByteArray &json);

	/** Load KiwiSDR dist.dx.json: {"dx":[[freq_kHz, mode, ident, notes, flags], ...]} */
	bool loadKiwiDxFromResource(const QString &resourcePath);
	bool loadKiwiDxFromData(const QByteArray &json);

	/** Load EiBi shortwave schedule CSV: kHz;Time(UTC);Days;ITU;Station;Lng;Target;Remarks;... */
	bool loadEiBiCsvFromResource(const QString &resourcePath);
	bool loadEiBiCsvFromFile(const QString &filePath);
	bool loadEiBiCsvFromData(const QByteArray &csv);

	/** Append spots (e.g. digimode dial freqs) skipping near-duplicates. */
	void mergeSpots(const QVector<BandSpot> &extra, qint64 nearHz = 50);

	/** Add or update a dynamic live spot (e.g. from RBN or Telnet DX Cluster). */
	void addSpotMarker(qint64 freqHz, const QString &callsign, const QString &mode = QString(),
	                   int snr = 0, int wpm = 0, const QString &spotter = QString(),
	                   const QString &comment = QString(), int ttlSec = 900);

	/** Remove expired dynamic spots. Returns true if any were removed. */
	bool pruneExpiredSpots(qint64 currentSec = 0);

	/** Clear all dynamic spots. */
	void clearDynamicSpots();

	const QVector<BandRange> &ranges() const { return m_ranges; }
	const QVector<BandSpot> &spots() const { return m_spots; }
	bool isEmpty() const { return m_ranges.isEmpty() && m_spots.isEmpty(); }

	/** Ranges that overlap [loHz, hiHz], already sorted by loHz. */
	QVector<BandRange> rangesInSpan(qint64 loHz, qint64 hiHz) const;

	/** Spots with freq in [loHz, hiHz], optionally filtered by UTC minute of day (0..1440) and ISO dayOfWeek (1..7). */
	QVector<BandSpot> spotsInSpan(qint64 loHz, qint64 hiHz, int utcMinOfDay = -1, int dayOfWeek = 0) const;

	/** Label for the range containing freqHz, or empty if none. */
	QString labelAt(qint64 freqHz) const;

	const BandRange *rangeAt(qint64 freqHz) const;

signals:
	void planChanged();

private:
	bool loadXml(const QByteArray &xml);
	bool loadSpotsJson(const QByteArray &json);
	bool loadKiwiDxJson(const QByteArray &json);
	bool loadEiBiCsv(const QByteArray &csv);
	static QColor colorFromSdrSharp(const QString &argbHex);
	static QString urlDecode(const QString &s);

	QVector<BandRange> m_ranges;
	QVector<BandSpot> m_spots;
};

#endif
