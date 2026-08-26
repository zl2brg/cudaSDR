#ifndef TRACERENDERER_H
#define TRACERENDERER_H

#include <QOpenGLFunctions>

class QGLReceiverPanel;

class TraceRenderer : protected QOpenGLFunctions {
public:
    explicit TraceRenderer(QGLReceiverPanel *panel);
    ~TraceRenderer();

    void drawPanadapter();
    void drawWaterfall();
    void drawBandPlanStrip();

private:
    void ensureGL();

    QGLReceiverPanel *m_panel;
    bool m_glReady;
};

#endif // TRACERENDERER_H
