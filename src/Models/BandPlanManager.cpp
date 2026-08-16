#include "BandPlanManager.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QXmlStreamReader>

#include <algorithm>

BandPlanManager::BandPlanManager(QObject *parent)
	: QObject(parent)
{
}

bool BandPlanManager::loadFromResource(const QString &resourcePath)
{
	QFile file(resourcePath);
	if (!file.open(QIODevice::ReadOnly))
		return false;
	return loadXml(file.readAll());
}

bool BandPlanManager::loadFromFile(const QString &filePath)
{
	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly))
		return false;
	return loadXml(file.readAll());
}

bool BandPlanManager::loadFromData(const QByteArray &xml)
{
	return loadXml(xml);
}

bool BandPlanManager::loadSpotsFromResource(const QString &resourcePath)
{
	QFile file(resourcePath);
	if (!file.open(QIODevice::ReadOnly))
		return false;
	return loadSpotsJson(file.readAll());
}

bool BandPlanManager::loadSpotsFromData(const QByteArray &json)
{
	return loadSpotsJson(json);
}

bool BandPlanManager::loadKiwiDxFromResource(const QString &resourcePath)
{
	QFile file(resourcePath);
	if (!file.open(QIODevice::ReadOnly))
		return false;
	return loadKiwiDxJson(file.readAll());
}

bool BandPlanManager::loadKiwiDxFromData(const QByteArray &json)
{
	return loadKiwiDxJson(json);
}

bool BandPlanManager::loadEiBiCsvFromResource(const QString &resourcePath)
{
	QFile file(resourcePath);
	if (!file.open(QIODevice::ReadOnly))
		return false;
	return loadEiBiCsv(file.readAll());
}

bool BandPlanManager::loadEiBiCsvFromFile(const QString &filePath)
{
	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly))
		return false;
	return loadEiBiCsv(file.readAll());
}

bool BandPlanManager::loadEiBiCsvFromData(const QByteArray &csv)
{
	return loadEiBiCsv(csv);
}

bool BandPlanManager::loadXml(const QByteArray &xml)
{
	QXmlStreamReader reader(xml);
	QVector<BandRange> loaded;

	while (!reader.atEnd()) {
		const auto token = reader.readNext();
		if (token != QXmlStreamReader::StartElement)
			continue;
		if (reader.name() != QLatin1String("RangeEntry"))
			continue;

		const QXmlStreamAttributes attrs = reader.attributes();
		BandRange range;
		range.loHz = attrs.value(QLatin1String("minFrequency")).toLongLong();
		range.hiHz = attrs.value(QLatin1String("maxFrequency")).toLongLong();
		range.mode = attrs.value(QLatin1String("mode")).toString();
		range.stepHz = attrs.value(QLatin1String("step")).toInt();
		range.color = colorFromSdrSharp(attrs.value(QLatin1String("color")).toString());
		range.name = reader.readElementText().trimmed();

		if (range.hiHz <= range.loHz)
			continue;
		loaded.append(range);
	}

	if (reader.hasError())
		return false;

	std::sort(loaded.begin(), loaded.end(),
	          [](const BandRange &a, const BandRange &b) { return a.loHz < b.loHz; });

	m_ranges = std::move(loaded);
	emit planChanged();
	return !m_ranges.isEmpty();
}

bool BandPlanManager::loadSpotsJson(const QByteArray &json)
{
	const QJsonDocument doc = QJsonDocument::fromJson(json);
	if (!doc.isObject())
		return false;

	const QJsonArray arr = doc.object().value(QLatin1String("spots")).toArray();
	QVector<BandSpot> loaded;
	loaded.reserve(arr.size());

	for (const QJsonValue &v : arr) {
		const QJsonObject o = v.toObject();
		BandSpot spot;
		spot.freqHz = qRound64(o.value(QLatin1String("freq")).toDouble() * 1e6);
		spot.label = o.value(QLatin1String("label")).toString().trimmed();
		if (spot.freqHz <= 0 || spot.label.isEmpty())
			continue;
		loaded.append(spot);
	}

	std::sort(loaded.begin(), loaded.end(),
	          [](const BandSpot &a, const BandSpot &b) { return a.freqHz < b.freqHz; });

	m_spots = std::move(loaded);
	emit planChanged();
	return !m_spots.isEmpty();
}

bool BandPlanManager::loadKiwiDxJson(const QByteArray &json)
{
	const QJsonDocument doc = QJsonDocument::fromJson(json);
	if (!doc.isObject())
		return false;

	const QJsonObject root = doc.object();
	// Default key is "dx"; custom DBs may rename the top-level array key.
	QJsonArray arr = root.value(QLatin1String("dx")).toArray();
	if (arr.isEmpty()) {
		for (auto it = root.begin(); it != root.end(); ++it) {
			if (it.value().isArray()) {
				arr = it.value().toArray();
				break;
			}
		}
	}

	QVector<BandSpot> loaded;
	loaded.reserve(arr.size());

	for (const QJsonValue &v : arr) {
		if (!v.isArray())
			continue;
		const QJsonArray e = v.toArray();
		if (e.size() < 3)
			continue;

		BandSpot spot;
		spot.freqHz = qRound64(e.at(0).toDouble() * 1000.0); // Kiwi stores kHz
		spot.label = urlDecode(e.at(2).toString()).trimmed();
		if (spot.freqHz <= 0 || spot.label.isEmpty())
			continue;

		// Prefer short digimode-style labels; otherwise keep the ident.
		loaded.append(spot);
	}

	std::sort(loaded.begin(), loaded.end(),
	          [](const BandSpot &a, const BandSpot &b) { return a.freqHz < b.freqHz; });

	m_spots = std::move(loaded);
	emit planChanged();
	return !m_spots.isEmpty();
}

bool BandPlanManager::loadEiBiCsv(const QByteArray &csv)
{
	if (csv.isEmpty())
		return false;

	QVector<BandSpot> loaded;
	loaded.reserve(10000);

	const QString text = QString::fromUtf8(csv);
	const QStringList lines = text.split(QLatin1Char('\n'));

	for (const QString &rawLine : lines) {
		const QString line = rawLine.trimmed();
		if (line.isEmpty() || line.startsWith(QLatin1Char('#')) || line.startsWith(QLatin1String("kHz"), Qt::CaseInsensitive))
			continue;

		const QStringList parts = line.split(QLatin1Char(';'));
		if (parts.size() < 5)
			continue;

		bool ok = false;
		const double kHz = parts.at(0).trimmed().toDouble(&ok);
		if (!ok || kHz <= 0.0)
			continue;

		const QString timeStr = parts.size() > 1 ? parts.at(1).trimmed() : QString();
		const QString days = parts.size() > 2 ? parts.at(2).trimmed() : QString();
		const QString itu = parts.size() > 3 ? parts.at(3).trimmed() : QString();
		const QString station = parts.size() > 4 ? parts.at(4).trimmed() : QString();
		const QString lang = parts.size() > 5 ? parts.at(5).trimmed() : QString();
		const QString target = parts.size() > 6 ? parts.at(6).trimmed() : QString();

		if (station.isEmpty())
			continue;

		BandSpot spot;
		spot.freqHz = qRound64(kHz * 1000.0);
		spot.itu = itu;
		spot.lang = lang;
		spot.target = target;
		spot.timeUtc = timeStr;
		spot.days = days;

		// Parse time range HHMM-HHMM
		if (timeStr.size() >= 9 && timeStr.contains(QLatin1Char('-'))) {
			const QStringList tParts = timeStr.split(QLatin1Char('-'));
			if (tParts.size() == 2) {
				const QString &s = tParts[0].trimmed();
				const QString &e = tParts[1].trimmed();
				if (s.size() == 4 && e.size() == 4) {
					bool sH_ok = false, sM_ok = false, eH_ok = false, eM_ok = false;
					const int sh = s.left(2).toInt(&sH_ok);
					const int sm = s.right(2).toInt(&sM_ok);
					const int eh = e.left(2).toInt(&eH_ok);
					const int em = e.right(2).toInt(&eM_ok);
					if (sH_ok && sM_ok && eH_ok && eM_ok) {
						if (s == QLatin1String("0000") && (e == QLatin1String("2400") || e == QLatin1String("0000"))) {
							spot.startMin = -1;
							spot.endMin = -1;
						} else {
							spot.startMin = sh * 60 + sm;
							spot.endMin = eh * 60 + em;
						}
					}
				}
			}
		}

		QString label = station;
		if (!lang.isEmpty()) {
			if (lang.startsWith(QLatin1Char('-')))
				label += QLatin1Char(' ') + lang;
			else
				label += QStringLiteral(" [") + lang + QStringLiteral("]");
		}
		spot.label = label;

		loaded.append(spot);
	}

	if (loaded.isEmpty())
		return false;

	std::sort(loaded.begin(), loaded.end(),
	          [](const BandSpot &a, const BandSpot &b) { return a.freqHz < b.freqHz; });

	m_spots = std::move(loaded);
	emit planChanged();
	return !m_spots.isEmpty();
}

void BandPlanManager::mergeSpots(const QVector<BandSpot> &extra, qint64 nearHz)
{
	if (extra.isEmpty())
		return;

	for (const BandSpot &s : extra) {
		bool exists = false;
		for (const BandSpot &existing : m_spots) {
			if (qAbs(existing.freqHz - s.freqHz) <= nearHz) {
				if (existing.label.compare(s.label, Qt::CaseInsensitive) == 0) {
					exists = true;
					break;
				}
			}
		}
		if (!exists)
			m_spots.append(s);
	}

	std::sort(m_spots.begin(), m_spots.end(),
	          [](const BandSpot &a, const BandSpot &b) { return a.freqHz < b.freqHz; });
	emit planChanged();
}

void BandPlanManager::addSpotMarker(qint64 freqHz, const QString &callsign, const QString &mode,
                                   int snr, int wpm, const QString &spotter,
                                   const QString &comment, int ttlSec)
{
	if (freqHz <= 0 || callsign.trimmed().isEmpty())
		return;

	const qint64 nowSec = QDateTime::currentSecsSinceEpoch();

	BandSpot spot;
	spot.freqHz = freqHz;
	spot.mode = mode.toUpper().trimmed();
	spot.snr = snr;
	spot.wpm = wpm;
	spot.spotter = spotter.trimmed();
	spot.target = comment.trimmed();
	spot.timestampSec = nowSec;
	spot.ttlSec = ttlSec > 0 ? ttlSec : 900; // default 15 minutes

	// Compose concise display label
	QString label = callsign.trimmed().toUpper();
	QStringList details;
	if (!spot.mode.isEmpty())
		details << spot.mode;
	if (wpm > 0)
		details << QStringLiteral("%1wpm").arg(wpm);
	if (snr != 0)
		details << QStringLiteral("%1%2dB").arg(snr > 0 ? "+" : "").arg(snr);

	if (!details.isEmpty())
		label += QStringLiteral(" [") + details.join(QLatin1Char(' ')) + QStringLiteral("]");

	spot.label = label;

	// Check if this spot / callsign already exists near this frequency (within 100 Hz)
	bool updated = false;
	for (BandSpot &existing : m_spots) {
		if (existing.ttlSec > 0 && qAbs(existing.freqHz - freqHz) <= 100) {
			if (existing.label.startsWith(callsign, Qt::CaseInsensitive)) {
				existing = spot;
				updated = true;
				break;
			}
		}
	}

	if (!updated) {
		m_spots.append(spot);
		std::sort(m_spots.begin(), m_spots.end(),
		          [](const BandSpot &a, const BandSpot &b) { return a.freqHz < b.freqHz; });
	}

	emit planChanged();
}

bool BandPlanManager::pruneExpiredSpots(qint64 currentSec)
{
	if (currentSec <= 0)
		currentSec = QDateTime::currentSecsSinceEpoch();

	const int initialSize = m_spots.size();
	m_spots.erase(
		std::remove_if(m_spots.begin(), m_spots.end(),
		               [currentSec](const BandSpot &s) { return s.isExpired(currentSec); }),
		m_spots.end());

	const bool removed = (m_spots.size() != initialSize);
	if (removed)
		emit planChanged();
	return removed;
}

void BandPlanManager::clearDynamicSpots()
{
	const int initialSize = m_spots.size();
	m_spots.erase(
		std::remove_if(m_spots.begin(), m_spots.end(),
		               [](const BandSpot &s) { return s.ttlSec > 0; }),
		m_spots.end());

	if (m_spots.size() != initialSize)
		emit planChanged();
}

QString BandPlanManager::urlDecode(const QString &s)
{
	return QUrl::fromPercentEncoding(s.toUtf8());
}

QColor BandPlanManager::colorFromSdrSharp(const QString &argbHex)
{
	QString hex = argbHex.trimmed();
	if (hex.startsWith(QLatin1Char('#')))
		hex.remove(0, 1);
	bool ok = false;
	const quint32 rgba = hex.toUInt(&ok, 16);
	if (!ok)
		return QColor(80, 80, 120, 128);
	// SDR# / Band-Plans use AARRGGBB.
	return QColor::fromRgba(rgba);
}

QVector<BandRange> BandPlanManager::rangesInSpan(qint64 loHz, qint64 hiHz) const
{
	QVector<BandRange> out;
	if (hiHz <= loHz || m_ranges.isEmpty())
		return out;

	for (const BandRange &r : m_ranges) {
		if (r.hiHz <= loHz)
			continue;
		if (r.loHz >= hiHz)
			break;
		out.append(r);
	}
	return out;
}

QVector<BandSpot> BandPlanManager::spotsInSpan(qint64 loHz, qint64 hiHz, int utcMinOfDay, int dayOfWeek) const
{
	QVector<BandSpot> out;
	if (hiHz <= loHz || m_spots.isEmpty())
		return out;

	const qint64 nowSec = QDateTime::currentSecsSinceEpoch();

	for (const BandSpot &s : m_spots) {
		if (s.freqHz < loHz)
			continue;
		if (s.freqHz > hiHz)
			break;
		if (s.isExpired(nowSec))
			continue;
		if (utcMinOfDay >= 0 && !s.isActiveAt(utcMinOfDay, dayOfWeek))
			continue;
		out.append(s);
	}
	return out;
}

const BandRange *BandPlanManager::rangeAt(qint64 freqHz) const
{
	auto it = std::upper_bound(m_ranges.cbegin(), m_ranges.cend(), freqHz,
	                           [](qint64 freq, const BandRange &r) { return freq < r.loHz; });
	if (it == m_ranges.cbegin())
		return nullptr;
	--it;
	if (freqHz >= it->loHz && freqHz < it->hiHz)
		return &(*it);
	return nullptr;
}

QString BandPlanManager::labelAt(qint64 freqHz) const
{
	if (const BandRange *r = rangeAt(freqHz))
		return r->name;
	return QString();
}
