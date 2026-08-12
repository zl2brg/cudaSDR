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

void BandPlanManager::mergeSpots(const QVector<BandSpot> &extra, qint64 nearHz)
{
	if (extra.isEmpty())
		return;

	for (const BandSpot &s : extra) {
		bool near = false;
		for (const BandSpot &existing : m_spots) {
			if (qAbs(existing.freqHz - s.freqHz) <= nearHz) {
				near = true;
				break;
			}
		}
		if (!near)
			m_spots.append(s);
	}

	std::sort(m_spots.begin(), m_spots.end(),
	          [](const BandSpot &a, const BandSpot &b) { return a.freqHz < b.freqHz; });
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

QVector<BandSpot> BandPlanManager::spotsInSpan(qint64 loHz, qint64 hiHz) const
{
	QVector<BandSpot> out;
	if (hiHz <= loHz || m_spots.isEmpty())
		return out;

	for (const BandSpot &s : m_spots) {
		if (s.freqHz < loHz)
			continue;
		if (s.freqHz > hiHz)
			break;
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
