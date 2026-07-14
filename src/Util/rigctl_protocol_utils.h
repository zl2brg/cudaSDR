#ifndef RIGCTL_PROTOCOL_UTILS_H
#define RIGCTL_PROTOCOL_UTILS_H

#include "cusdr_hamDatabase.h"

#include <QString>

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

} // namespace RigctlProtocol

#endif // RIGCTL_PROTOCOL_UTILS_H
