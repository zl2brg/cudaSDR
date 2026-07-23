#ifndef RIGCTL_PROTOCOL_UTILS_H
#define RIGCTL_PROTOCOL_UTILS_H

#include "cusdr_hamDatabase.h"

#include <QString>
#include <QStringList>
#include <optional>

namespace RigctlProtocol {

inline QString dspModeToRigctlMode(DSPMode mode)
{
    switch (mode) {
        case LSB:  return QStringLiteral("LSB");
        case USB:  return QStringLiteral("USB");
        case DSB:  return QStringLiteral("DSB");
        case CWL:  return QStringLiteral("CWR");
        case CWU:  return QStringLiteral("CW");
        case FMN:  return QStringLiteral("FM");
        case AM:   return QStringLiteral("AM");
        case DIGU: return QStringLiteral("PKTUSB");
        case DIGL: return QStringLiteral("PKTLSB");
        case SAM:  return QStringLiteral("AMS");
        default:   return QStringLiteral("USB");
    }
}

inline int rigctlModeToDsp(const QString &mode)
{
    const QString m = mode.toUpper();
    if (m == QLatin1String("LSB"))    return LSB;
    if (m == QLatin1String("USB"))    return USB;
    if (m == QLatin1String("DSB"))    return DSB;
    if (m == QLatin1String("CWR"))    return CWL;
    if (m == QLatin1String("CW"))     return CWU;
    if (m == QLatin1String("FM"))     return FMN;
    if (m == QLatin1String("AM"))     return AM;
    if (m == QLatin1String("AMS"))    return SAM;
    if (m == QLatin1String("PKTUSB")) return DIGU;
    if (m == QLatin1String("PKTLSB")) return DIGL;
    return -1;
}

// Newer WSJT-X / hamlib often insert a VFO token: "F VFOA 14074000".
inline bool looksLikeVfoToken(const QString &token)
{
    const QString t = token.toUpper();
    return t.startsWith(QLatin1String("VFO"))
           || t == QLatin1String("MAIN")
           || t == QLatin1String("SUB")
           || t == QLatin1String("CURRVFO")
           || t == QLatin1String("TX")
           || t == QLatin1String("RX");
}

// Parse Hz from "F <hz>" or "F <VFO> <hz>" (also works for long-form args).
inline std::optional<qint64> parseFrequencyHz(const QStringList &parts, int firstArgIndex = 1)
{
    if (parts.size() <= firstArgIndex)
        return std::nullopt;

    if (parts.size() >= firstArgIndex + 2 && looksLikeVfoToken(parts.at(firstArgIndex))) {
        bool ok = false;
        const qint64 freq = static_cast<qint64>(parts.at(firstArgIndex + 1).toDouble(&ok));
        if (ok && freq > 0)
            return freq;
        return std::nullopt;
    }

    bool ok = false;
    const qint64 freq = static_cast<qint64>(parts.at(firstArgIndex).toDouble(&ok));
    if (ok && freq > 0)
        return freq;
    return std::nullopt;
}

// Parse mode from "M <mode> [bw]" or "M <VFO> <mode> [bw]".
inline QString parseModeToken(const QStringList &parts, int firstArgIndex = 1)
{
    if (parts.size() <= firstArgIndex)
        return {};
    if (looksLikeVfoToken(parts.at(firstArgIndex))) {
        if (parts.size() <= firstArgIndex + 1)
            return {};
        return parts.at(firstArgIndex + 1);
    }
    return parts.at(firstArgIndex);
}

// Parse PTT from "T <0|1>" or "T <VFO> <0|1>".
inline std::optional<int> parsePttValue(const QStringList &parts, int firstArgIndex = 1)
{
    if (parts.size() <= firstArgIndex)
        return std::nullopt;

    int idx = firstArgIndex;
    if (looksLikeVfoToken(parts.at(idx))) {
        if (parts.size() <= idx + 1)
            return std::nullopt;
        ++idx;
    }

    bool ok = false;
    const int val = parts.at(idx).toInt(&ok);
    if (!ok)
        return std::nullopt;
    return val;
}

} // namespace RigctlProtocol

#endif // RIGCTL_PROTOCOL_UTILS_H
