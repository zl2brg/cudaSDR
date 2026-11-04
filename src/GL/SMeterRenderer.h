/**
 * @file  SMeterRenderer.h
 * @brief Renderer for the analog S-Meter arc, needle, labels, and FBO caching in OGLDisplayPanel.
 * @author Simon Eatough <simon.eatough@gmail.com>
 * @date 2026-09-08
 */

#ifndef SMETER_RENDERER_H
#define SMETER_RENDERER_H

#include <QColor>
#include <QMatrix4x4>
#include <QtOpenGL/QOpenGLFramebufferObject>
#include <QtGlobal>
#include <cmath>

class OGLDisplayPanel;

class SMeterRenderer {
public:
    explicit SMeterRenderer(OGLDisplayPanel *panel);
    ~SMeterRenderer();

    void paintSMeter();
    void renderSMeterScale();
    void drawSMeterNeedle(const QMatrix4x4 &projection, int x1);
    void drawSMeterScaleLabels(const QMatrix4x4 &projection, int xOffset);
    void paintTxMeters(const QMatrix4x4 &projection, int xOffset);
    void invalidateFBO();

    /** 0..1 fill. Square-root curve; 0–10 W then 0–100 W. */
    static inline qreal txPowerFraction(qreal watts)
    {
        const qreal pVal = qMax(0.0, watts);
        const qreal maxP = (pVal > 10.0) ? 100.0 : 10.0;
        return qBound(0.0, std::sqrt(pVal / maxP), 1.0);
    }

    /** 0.30 at SWR 1.0, 1.0 at SWR >= 3.0. */
    static inline qreal txSwrFraction(qreal swr)
    {
        const qreal swrVal = qMax(1.0, swr);
        if (swrVal <= 1.0)
            return 0.30;
        return qBound(0.30, 0.30 + 0.70 * ((swrVal - 1.0) / 2.0), 1.0);
    }

    static inline QColor txSwrColor(qreal swr)
    {
        if (swr >= 2.5)
            return QColor(242, 56, 109);
        if (swr >= 1.5)
            return QColor(255, 255, 50);
        return QColor(56, 242, 115);
    }

private:
    OGLDisplayPanel *m_panel;
    QOpenGLFramebufferObject *m_smeterFBO = nullptr;
};

#endif // SMETER_RENDERER_H
