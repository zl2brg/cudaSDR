#ifndef OVERLAYRENDERER_H
#define OVERLAYRENDERER_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QVector>
#include <QRect>
#include <QMatrix4x4>
#include "../cusdr_settings.h"
#include "cusdr_oglUtils.h"

class OverlayRenderer : protected QOpenGLFunctions {
public:
    explicit OverlayRenderer();
    ~OverlayRenderer();

    void initialize(QOpenGLShaderProgram* sharedShader = nullptr);

    void drawGrid(const QMatrix4x4& projection,
                  const QRect& panRect,
                  const QRect& freqScalePanRect,
                  const TScale& freqScale,
                  const TScale& dBmScale,
                  int freqPlotLeft,
                  float r, float g, float b, float alpha,
                  bool panGridEnabled);

    void drawFrequencyScaleTicks(const QMatrix4x4& projection,
                                 const QRect& freqScalePanRect,
                                 const TScale& freqScale,
                                 float deltaF,
                                 float zoomFactor,
                                 float r, float g, float b, float alpha);

    void drawDBmScaleTicks(const QMatrix4x4& projection,
                           const QRect& dBmScalePanRect,
                           const TScale& dBmScale,
                           float r, float g, float b, float alpha);

    void drawCenterLine(const QMatrix4x4& projection,
                        const QRect& panRect,
                        const QRect& freqScalePanRect,
                        const QRect& waterfallRect,
                        int centerlineHeight,
                        float deltaF,
                        float zoomFactor,
                        const QColor& vfoColor,
                        bool dragMouse,
                        bool panLocked);

    void drawFilter(const QMatrix4x4& projection,
                    const QRect& panRect,
                    const QRect& waterfallRect,
                    float filterLo, float filterHi,
                    float deltaF, float zoomFactor,
                    const QColor& filterColor,
                    bool highlightFilter,
                    bool dragPanning,
                    bool showLeftBoundary, bool showRightBoundary,
                    // Outputs for caller (needed for text rendering)
                    int& filterLeft, int& filterRight,
                    int& filterTop, int& filterBottom);

    void drawCrossHair(const QMatrix4x4& projection,
                       const QRect& panRect,
                       const QRect& dBmScalePanRect,
                       const QPoint& mousePos,
                       float dpr,
                       int parentHeight);

    void drawAGCControl(const QMatrix4x4& projection,
                        const QRect& panRect,
                        const QRect& dBmScalePanRect,
                        AGCMode mode,
                        bool hangEnabled,
                        float agcThreshold,
                        float agcHangLevel,
                        float agcFixedGain,
                        qreal dBmMax, qreal dBmMin,
                        float dpr,
                        int parentHeight,
                        // Outputs
                        float& threshPixel, float& hangPixel, float& fixedPixel);

    // Translucent filled rect (optional top→bottom colour gradient).
    void drawFilledRect(const QMatrix4x4& projection,
                        const QRect& rect,
                        const QColor& topColor,
                        const QColor& bottomColor,
                        float z = 0.0f);

private:
    struct VertexData {
        float x, y, z;
        float r, g, b, a;
    };

    QOpenGLShaderProgram* m_shader;
    bool m_ownsShader;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;
};

#endif // OVERLAYRENDERER_H
