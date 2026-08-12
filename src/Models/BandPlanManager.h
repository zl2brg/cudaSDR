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

	/** Append spots (e.g. digimode dial freqs) skipping near-duplicates. */
	void mergeSpots(const QVector<BandSpot> &extra, qint64 nearHz = 50);

	const QVector<BandRange> &ranges() const { return m_ranges; }
	const QVector<BandSpot> &spots() const { return m_spots; }
	bool isEmpty() const { return m_ranges.isEmpty() && m_spots.isEmpty(); }

	/** Ranges that overlap [loHz, hiHz], already sorted by loHz. */
	QVector<BandRange> rangesInSpan(qint64 loHz, qint64 hiHz) const;

	/** Spots with freq in [loHz, hiHz], sorted by frequency. */
	QVector<BandSpot> spotsInSpan(qint64 loHz, qint64 hiHz) const;

	/** Label for the range containing freqHz, or empty if none. */
	QString labelAt(qint64 freqHz) const;

	const BandRange *rangeAt(qint64 freqHz) const;

signals:
	void planChanged();

private:
	bool loadXml(const QByteArray &xml);
	bool loadSpotsJson(const QByteArray &json);
	bool loadKiwiDxJson(const QByteArray &json);
	static QColor colorFromSdrSharp(const QString &argbHex);
	static QString urlDecode(const QString &s);

	QVector<BandRange> m_ranges;
	QVector<BandSpot> m_spots;
};

#endif
