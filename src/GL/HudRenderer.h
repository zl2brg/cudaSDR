#ifndef HUDRENDERER_H
#define HUDRENDERER_H

#include <QOpenGLFunctions>

class QGLReceiverPanel;

class HudRenderer : protected QOpenGLFunctions {
public:
    explicit HudRenderer(QGLReceiverPanel *panel);
    ~HudRenderer();

    void drawVFOControl();
    void drawReceiverInfo();
    void drawCwDecoderHUD();
    void drawCrossHair();
    void drawFilterLabels();
    void drawAGCLabels();

private:
    void ensureGL();

    QGLReceiverPanel *m_panel;
    bool m_glReady;
};

#endif // HUDRENDERER_H
