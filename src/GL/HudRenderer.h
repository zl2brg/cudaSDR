#ifndef HUDRENDERER_H
#define HUDRENDERER_H

#include <QOpenGLFunctions>
#include <QRect>

class QGLReceiverPanel;

class HudRenderer : protected QOpenGLFunctions {
public:
    explicit HudRenderer(QGLReceiverPanel *panel);
    ~HudRenderer();

    void drawVFOControl();
    void drawPanadapterSMeter();
    void drawPanadapterFreq();
    void drawCwDecoderHUD();
    void drawRttyDecoderHUD();
    void drawCrossHair();
    void drawFilterLabels();
    void drawAGCLabels();

private:
    void ensureGL();
    void drawRttyTuningScope(const QRect &scopeRect, float markHz);

    QGLReceiverPanel *m_panel;
    bool m_glReady;
    float m_rttyScopePeak = 0.0001f;
};

#endif // HUDRENDERER_H
