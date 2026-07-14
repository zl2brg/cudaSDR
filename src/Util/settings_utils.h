#ifndef SETTINGS_UTILS_H
#define SETTINGS_UTILS_H

#include "Settings/SettingsTypes.h"
#include "cusdr_hamDatabase.h"

#include <QString>

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

} // namespace SettingsUtils

#endif // SETTINGS_UTILS_H
