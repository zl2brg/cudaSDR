/**
 * @file  SMeterRenderer.h
 * @brief Renderer for the analog S-Meter arc, needle, labels, and FBO caching in OGLDisplayPanel.
 * @author Simon Eatough <simon.eatough@gmail.com>
 * @date 2026-09-08
 */

#ifndef SMETER_RENDERER_H
#define SMETER_RENDERER_H

#include <QMatrix4x4>
#include <QtMath>
#include <QtOpenGL/QOpenGLFramebufferObject>

class OGLDisplayPanel;

class SMeterRenderer {
public:
    explicit SMeterRenderer(OGLDisplayPanel *panel);
    ~SMeterRenderer();

    void paintSMeter();
    void renderSMeterScale();
    void drawSMeterNeedle(const QMatrix4x4 &projection, int x1);
    void drawSMeterScaleLabels(const QMatrix4x4 &projection, int xOffset);
    void invalidateFBO();

    /** Pixels per dB for a scale spanning [minDb, maxDb] over width pixels. */
    static qreal unitForRange(qreal width, qreal minDb, qreal maxDb)
    {
        const qreal range = qAbs(maxDb - minDb);
        return (range > 0.0) ? (width / range) : 0.0;
    }
    /** X offset of an absolute dBm value from the left edge of the scale. */
    static float xForDbm(qreal dbm, qreal minDb, qreal unit)
    {
        return float((dbm - minDb) * unit);
    }

private:
    OGLDisplayPanel *m_panel;
    QOpenGLFramebufferObject *m_smeterFBO = nullptr;
};

#endif // SMETER_RENDERER_H
