#ifndef SETTINGS_UTILS_H
#define SETTINGS_UTILS_H

#include "Settings/SettingsTypes.h"
#include "cusdr_hamDatabase.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QSettings>
#include <QString>
#include <QVector>
#include <algorithm>
#include <type_traits>

namespace SettingsUtils {

inline bool agcHangEnabledForMode(AGCMode mode)
{
    return mode != static_cast<AGCMode>(agcOFF)
        && mode != static_cast<AGCMode>(agcMED)
        && mode != static_cast<AGCMode>(agcFAST);
}

inline bool sampleRateToParams(int rate, int &speed, int &outputIncrement)
{
    switch (rate) {
        case 48000:
            speed = 0;
            outputIncrement = 1;
            return true;
        case 96000:
            speed = 1;
            outputIncrement = 2;
            return true;
        case 192000:
            speed = 2;
            outputIncrement = 4;
            return true;
        case 384000:
            speed = 3;
            outputIncrement = 8;
            return true;
        case 768000:
            speed = 4;
            outputIncrement = 16;
            return true;
        case 1536000:
            speed = 5;
            outputIncrement = 32;
            return true;
        default:
            return false;
    }
}

inline int clampIntRange(int value, int minValue, int maxValue, int fallback)
{
    if (value < minValue || value > maxValue)
        return fallback;
    return value;
}

inline int clampNetworkPort(int value, int fallback)
{
    return clampIntRange(value, 0, 65535, fallback);
}

inline int clampMinimumWidgetWidth(int value)
{
    return clampIntRange(value, 235, 350, 300);
}

inline int clampMinimumGroupBoxWidth(int value, int minimumWidgetWidth)
{
    if (value < 230 || value > 295 || value > minimumWidgetWidth - 5)
        return 250;
    return value;
}

inline int clampMultiRxView(int value)
{
    return clampIntRange(value, 0, 2, 0);
}

inline int clampDriveLevel(int level)
{
    return clampIntRange(level, 0, 100, 0);
}

inline int clampSocketBufferSizeKb(int value)
{
    if (value == 16 || value == 32 || value == 64 || value == 128 || value == 256)
        return value;
    return 32;
}

inline QString stripSurroundingQuotes(QString value)
{
    while (value.startsWith(QLatin1Char('"')))
        value = value.right(value.length() - 1).trimmed();
    while (value.endsWith(QLatin1Char('"')))
        value = value.left(value.length() - 1).trimmed();
    return value;
}

inline int clampBoundedInt(int value, int lo, int hi)
{
    return qBound(lo, value, hi);
}

inline double clampBoundedDouble(double value, double lo, double hi)
{
    return qBound(lo, value, hi);
}

inline int clampEqDeg(int deg)
{
    return clampBoundedInt(deg, 0, 8);
}

inline int clampEqBandDb(int db)
{
    return clampBoundedInt(db, -12, 12);
}

inline double clampDb(double value, double lo, double hi)
{
    return clampBoundedDouble(value, lo, hi);
}

inline long clampFreq(double value, double lo, double hi, double fallback)
{
    if (value < lo || value > hi)
        return static_cast<long>(fallback);
    return static_cast<long>(value);
}

inline float clampTciGain(float gain)
{
    return qBound(0.0f, gain, 2.0f);
}

inline bool iniOn(const QSettings *settings, const QString &key,
                  const QString &defaultValue = QStringLiteral("off"))
{
    return settings->value(key, defaultValue).toString().compare(
               QLatin1String("on"), Qt::CaseInsensitive) == 0;
}

inline void setIniOn(QSettings *settings, const QString &key, bool on)
{
    settings->setValue(key, on ? QStringLiteral("on") : QStringLiteral("off"));
}

template<typename T>
inline T jsonValueAs(const QJsonValue &value);

template<>
inline int jsonValueAs<int>(const QJsonValue &value)
{
    return value.toInt();
}

template<>
inline double jsonValueAs<double>(const QJsonValue &value)
{
    return value.toDouble();
}

template<>
inline long jsonValueAs<long>(const QJsonValue &value)
{
    return static_cast<long>(value.toDouble());
}

template<>
inline qint64 jsonValueAs<qint64>(const QJsonValue &value)
{
    return static_cast<qint64>(value.toDouble());
}

template<>
inline bool jsonValueAs<bool>(const QJsonValue &value)
{
    return value.toBool();
}

template<typename T, typename Setter>
void applyJsonArray(const QJsonObject &json, const QString &key, int maxCount, Setter setter)
{
    if (!json.contains(key) || !json.value(key).isArray() || maxCount <= 0)
        return;
    const QJsonArray arr = json.value(key).toArray();
    const int n = std::min(static_cast<int>(arr.size()), maxCount);
    for (int i = 0; i < n; ++i)
        setter(i, jsonValueAs<T>(arr.at(i)));
}

template<typename T>
QVector<T> jsonArrayToVector(const QJsonObject &json, const QString &key, int count, T fill = T{})
{
    QVector<T> out(count, fill);
    if (!json.contains(key) || !json.value(key).isArray() || count <= 0)
        return out;
    const QJsonArray arr = json.value(key).toArray();
    const int n = std::min(static_cast<int>(arr.size()), count);
    for (int i = 0; i < n; ++i)
        out[i] = jsonValueAs<T>(arr.at(i));
    return out;
}

template<typename Container>
QJsonArray toJsonArray(const Container &values)
{
    QJsonArray arr;
    for (const auto &v : values) {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_floating_point_v<T> || std::is_same_v<T, long> || std::is_same_v<T, qint64>)
            arr.append(static_cast<double>(v));
        else
            arr.append(static_cast<int>(v));
    }
    return arr;
}

} // namespace SettingsUtils

#endif // SETTINGS_UTILS_H
