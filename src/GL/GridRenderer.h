#ifndef GRIDRENDERER_H
#define GRIDRENDERER_H

#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QPainter>

class QGLReceiverPanel;

class GridRenderer : protected QOpenGLFunctions {
public:
    explicit GridRenderer(QGLReceiverPanel *panel);
    ~GridRenderer();

    void updateFrequencyRuler();
    void updateDBmRuler();
    void drawPanVerticalScale();
    void drawPanHorizontalScale();
    void drawWaterfallVerticalScale();
    void drawPanadapterGrid();
    void invalidateScaleFBOs();

private:
    void ensureGL();
    void renderPanVerticalScale();
    void renderPanHorizontalScale();
    void renderWaterfallVerticalScale();

    QGLReceiverPanel *m_panel;
    QOpenGLFramebufferObject *m_frequencyScaleFBO;
    QOpenGLFramebufferObject *m_dBmScaleFBO;
    QOpenGLFramebufferObject *m_secScaleWaterfallFBO;
    QPainter painter;
    bool m_glReady;
};

#endif // GRIDRENDERER_H
