#ifndef PANADAPTERRENDERER_H
#define PANADAPTERRENDERER_H

#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QRect>
#include <QVector>
#include <QVarLengthArray>
#include <memory>

#include <rhi/qrhi.h>

#include "../cusdr_settings.h"

class QOpenGLContext;
class QOffscreenSurface;

class PanadapterRenderer {
public:
    struct Colors {
        float r, g, b;       // Line color
        float rf, gf, bf;    // Filled color
        float rst, gst, bst; // Solid Top color
        float rsb, gsb, bsb; // Solid Bottom color
        float bkgR, bkgG, bkgB; // Background color
    };

    explicit PanadapterRenderer();
    ~PanadapterRenderer();

    bool initialize(QOpenGLContext *shareContext, QOpenGLShaderProgram *sharedShader = nullptr);
    void release();

    bool usesCompositePass() const { return m_rhiActive; }

    void render(QOpenGLFunctions *gl,
                const QMatrix4x4& projection,
                const QRect& panRect,
                const QVector<qreal>& bins,
                qreal dBmMax, qreal dBmMin,
                PanGraphicsMode mode,
                float scaleMult,
                float dpr,
                int parentHeight,
                const Colors& colors,
                QSDR::_DataEngineState dataEngineState,
                bool isCurrentReceiver,
                const QVector<qreal>& peakHoldBins = {},
                bool scaleColorByLevel = false);

    void renderIdleBackground(QOpenGLFunctions *gl,
                              const QMatrix4x4& projection,
                              const QRect& panRect,
                              float dpr,
                              int parentHeight,
                              const Colors& colors,
                              QSDR::_DataEngineState dataEngineState,
                              bool isCurrentReceiver);

    bool compositeToDefaultFramebuffer(QOpenGLFunctions *gl,
                                       const QMatrix4x4& projection,
                                       const QRect& panRect,
                                       float dpr,
                                       int parentHeight);

private:
    struct VertexData {
        float x, y, z;
        float r, g, b, a;
    };

    struct CompositeVertex {
        float x, y, z;
        float u, v;
    };

    bool ensureRenderTarget(const QSize &pixelSize);
    QRhiGraphicsPipeline *pipelineForTopology(QRhiGraphicsPipeline::Topology topology);
    void drawGeometry(QRhiCommandBuffer *cb,
                      const QMatrix4x4 &matrix,
                      const VertexData *data,
                      int count,
                      QRhiGraphicsPipeline *pipeline,
                      int vertexCount);
    void ensureCompositeResources(QOpenGLFunctions *gl);
    void renderWithOpenGL(const QMatrix4x4& projection,
                          const QRect& panRect,
                          const QVector<qreal>& bins,
                          qreal dBmMax, qreal dBmMin,
                          PanGraphicsMode mode,
                          float scaleMult,
                          float dpr,
                          int parentHeight,
                          const Colors& colors,
                          QSDR::_DataEngineState dataEngineState,
                          bool isCurrentReceiver,
                          QOpenGLFunctions *gl,
                          const QVector<qreal>& peakHoldBins = {},
                          bool scaleColorByLevel = false);

    bool m_rhiActive = false;
    std::unique_ptr<QRhi> m_rhi;
    QOffscreenSurface *m_fallbackSurface = nullptr;

    std::unique_ptr<QRhiBuffer> m_vbuf;
    std::unique_ptr<QRhiBuffer> m_ubuf;
    std::unique_ptr<QRhiTexture> m_colorTex;
    std::unique_ptr<QRhiTextureRenderTarget> m_rt;
    std::unique_ptr<QRhiRenderPassDescriptor> m_rp;
    std::unique_ptr<QRhiShaderResourceBindings> m_srb;
    std::unique_ptr<QRhiGraphicsPipeline> m_triStripPipeline;
    std::unique_ptr<QRhiGraphicsPipeline> m_lineStripPipeline;
    std::unique_ptr<QRhiGraphicsPipeline> m_linesPipeline;
    QShader m_vertShader;
    QShader m_fragShader;

    QSize m_rtPixelSize;
    int m_vbufCapacityBytes = 0;
    QVarLengthArray<VertexData> m_vertexCache;

    QOpenGLShaderProgram *m_compositeProgram = nullptr;
    QOpenGLBuffer m_compositeVbo;
    bool m_compositeInitialized = false;

    QOpenGLShaderProgram *m_glShader = nullptr;
    bool m_ownsGlShader = false;
    QOpenGLVertexArrayObject m_glVao;
    QOpenGLBuffer m_glVbo;
    int m_glVboSize = 0;
};

#endif // PANADAPTERRENDERER_H
