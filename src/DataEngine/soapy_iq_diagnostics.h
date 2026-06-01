#ifndef SOAPY_IQ_DIAGNOSTICS_H
#define SOAPY_IQ_DIAGNOSTICS_H

#include <QtGlobal>
#include <QString>
#include <QDebug>
#include <cmath>
#include <cstdint>

#include "QtWDSP/qtdsp_qComplex.h"

struct IqDiagStats {
    float minRe = 0.0f;
    float maxRe = 0.0f;
    float minIm = 0.0f;
    float maxIm = 0.0f;
    double rms = 0.0;
    double peak = 0.0;
    int nearZero = 0;
    float s0re = 0.0f;
    float s0im = 0.0f;
    float s1re = 0.0f;
    float s1im = 0.0f;
};

namespace SoapyIqDiag {

inline IqDiagStats statsFromInterleavedFloat(const float *iq, int numComplex, bool negateIm = false)
{
    IqDiagStats s;
    if (!iq || numComplex <= 0)
        return s;

    s.s0re = iq[0];
    s.s0im = negateIm ? -iq[1] : iq[1];
    if (numComplex > 1) {
        s.s1re = iq[2];
        s.s1im = negateIm ? -iq[3] : iq[3];
    }

    s.minRe = s.maxRe = s.s0re;
    s.minIm = s.maxIm = s.s0im;

    double sumSq = 0.0;
    for (int i = 0; i < numComplex; ++i) {
        const float re = iq[2 * i];
        const float im = negateIm ? -iq[2 * i + 1] : iq[2 * i + 1];
        s.minRe = qMin(s.minRe, re);
        s.maxRe = qMax(s.maxRe, re);
        s.minIm = qMin(s.minIm, im);
        s.maxIm = qMax(s.maxIm, im);
        const double magSq = static_cast<double>(re) * re + static_cast<double>(im) * im;
        sumSq += magSq;
        const double mag = std::sqrt(magSq);
        s.peak = qMax(s.peak, mag);
        if (mag < 1.0e-6)
            ++s.nearZero;
    }
    s.rms = std::sqrt(sumSq / numComplex);
    return s;
}

inline IqDiagStats statsFromInterleavedInt32(const int32_t *iq, int numComplex, double scale = 1.0 / 8388607.0)
{
    IqDiagStats s;
    if (!iq || numComplex <= 0)
        return s;

    s.s0re = static_cast<float>(iq[0] * scale);
    s.s0im = static_cast<float>(iq[1] * scale);
    if (numComplex > 1) {
        s.s1re = static_cast<float>(iq[2] * scale);
        s.s1im = static_cast<float>(iq[3] * scale);
    }

    s.minRe = s.maxRe = s.s0re;
    s.minIm = s.maxIm = s.s0im;

    double sumSq = 0.0;
    for (int i = 0; i < numComplex; ++i) {
        const float re = static_cast<float>(iq[2 * i] * scale);
        const float im = static_cast<float>(iq[2 * i + 1] * scale);
        s.minRe = qMin(s.minRe, re);
        s.maxRe = qMax(s.maxRe, re);
        s.minIm = qMin(s.minIm, im);
        s.maxIm = qMax(s.maxIm, im);
        const double magSq = static_cast<double>(re) * re + static_cast<double>(im) * im;
        sumSq += magSq;
        const double mag = std::sqrt(magSq);
        s.peak = qMax(s.peak, mag);
        if (mag < 1.0e-6)
            ++s.nearZero;
    }
    s.rms = std::sqrt(sumSq / numComplex);
    return s;
}

inline IqDiagStats statsFromCpx(const CPX &buf, int numComplex)
{
    IqDiagStats s;
    if (numComplex <= 0 || buf.isEmpty())
        return s;

    const cpx *iq = buf.constData();
    s.s0re = static_cast<float>(iq[0].re);
    s.s0im = static_cast<float>(iq[0].im);
    if (numComplex > 1) {
        s.s1re = static_cast<float>(iq[1].re);
        s.s1im = static_cast<float>(iq[1].im);
    }

    s.minRe = s.maxRe = s.s0re;
    s.minIm = s.maxIm = s.s0im;

    double sumSq = 0.0;
    for (int i = 0; i < numComplex; ++i) {
        const float re = static_cast<float>(iq[i].re);
        const float im = static_cast<float>(iq[i].im);
        s.minRe = qMin(s.minRe, re);
        s.maxRe = qMax(s.maxRe, re);
        s.minIm = qMin(s.minIm, im);
        s.maxIm = qMax(s.maxIm, im);
        const double magSq = static_cast<double>(re) * re + static_cast<double>(im) * im;
        sumSq += magSq;
        const double mag = std::sqrt(magSq);
        s.peak = qMax(s.peak, mag);
        if (mag < 1.0e-6)
            ++s.nearZero;
    }
    s.rms = std::sqrt(sumSq / numComplex);
    return s;
}

inline QString formatStats(const IqDiagStats &s)
{
    return QStringLiteral("rms=%1 peak=%2 re[%3,%4] im[%5,%6] nearZero=%7 s0=%8+%9i s1=%10+%11i")
        .arg(s.rms, 0, 'g', 4)
        .arg(s.peak, 0, 'g', 4)
        .arg(s.minRe, 0, 'g', 3)
        .arg(s.maxRe, 0, 'g', 3)
        .arg(s.minIm, 0, 'g', 3)
        .arg(s.maxIm, 0, 'g', 3)
        .arg(s.nearZero)
        .arg(s.s0re, 0, 'g', 3)
        .arg(s.s0im, 0, 'g', 3)
        .arg(s.s1re, 0, 'g', 3)
        .arg(s.s1im, 0, 'g', 3);
}

inline bool shouldLog(quint64 counter, quint64 interval = 50)
{
    return interval > 0 && (counter % interval) == 1;
}

} // namespace SoapyIqDiag

#define SOAPY_IQ_DIAG(tag, block, extra, stats) \
    qDebug().nospace() << "[SoapyIQ] " << (tag) << " block=" << (block) << " " << (extra) << " " << SoapyIqDiag::formatStats(stats)

#endif // SOAPY_IQ_DIAGNOSTICS_H
