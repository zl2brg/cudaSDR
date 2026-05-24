#include "PanadapterRenderer.h"
#include "cusdr_glShaders.h"

#include <QFile>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QDebug>
#include <QtGui/rhi/qrhi_platform.h>

namespace {

QShader loadSerializedShader(const char *resourcePath)
{
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "PanadapterRenderer: failed to open shader resource" << resourcePath;
        return QShader();
    }
    const QShader shader = QShader::fromSerialized(file.readAll());
    if (!shader.isValid())
        qWarning() << "PanadapterRenderer: invalid qsb" << resourcePath;
    return shader;
}

} // namespace

PanadapterRenderer::PanadapterRenderer() = default;

PanadapterRenderer::~PanadapterRenderer()
{
    release();
}

bool PanadapterRenderer::initialize(QOpenGLContext *shareContext, QOpenGLShaderProgram *sharedShader)
{
    if (!shareContext || !shareContext->isValid())
        return false;

    auto initGlFallback = [&]() -> bool {
        m_rhiActive = false;
        if (sharedShader) {
            m_glShader = sharedShader;
            m_ownsGlShader = false;
        } else {
            m_glShader = new QOpenGLShaderProgram();
            m_ownsGlShader = true;
            m_glShader->addShaderFromSourceCode(QOpenGLShader::Vertex, GlShaders::coloredVertexSource());
            m_glShader->addShaderFromSourceCode(QOpenGLShader::Fragment, GlShaders::coloredFragmentSource());
            m_glShader->bindAttributeLocation("position", 0);
            m_glShader->bindAttributeLocation("color", 1);
            m_glShader->link();
        }
        if (!m_glShader || !m_glShader->isLinked())
            return false;

        m_glVao.create();
        m_glVao.bind();
        m_glVbo.create();
        m_glVbo.bind();
        m_glVbo.setUsagePattern(QOpenGLBuffer::StreamDraw);
        m_glShader->bind();
        m_glShader->enableAttributeArray(0);
        m_glShader->enableAttributeArray(1);
        m_glShader->release();
        m_glVao.release();
        m_glVbo.release();
        return true;
    };

    if (m_rhi)
        return m_glShader && m_glShader->isLinked();

    // Always draw the RX panadapter with direct GL in the QOpenGLWidget context.
    // QRhi offscreen + texture composite is unreliable on OpenGL ES (xcb_egl) and on
    // desktop GLX; the GLES path already used this fallback — use it everywhere.
    Q_UNUSED(shareContext);
    return initGlFallback();
}

void PanadapterRenderer::release()
{
    if (m_compositeProgram) {
        delete m_compositeProgram;
        m_compositeProgram = nullptr;
    }
    if (m_compositeVbo.isCreated())
        m_compositeVbo.destroy();

    m_triStripPipeline.reset();
    m_lineStripPipeline.reset();
    m_linesPipeline.reset();
    m_vertShader = QShader();
    m_fragShader = QShader();
    m_srb.reset();
    m_ubuf.reset();
    m_vbuf.reset();
    m_rt.reset();
    m_rp.reset();
    m_colorTex.reset();
    m_rhi.reset();

    if (m_fallbackSurface) {
        delete m_fallbackSurface;
        m_fallbackSurface = nullptr;
    }

    m_rtPixelSize = QSize();
    m_vbufCapacityBytes = 0;
    m_compositeInitialized = false;
    m_rhiActive = false;

    if (m_glVao.isCreated())
        m_glVao.destroy();
    if (m_glVbo.isCreated())
        m_glVbo.destroy();
    if (m_ownsGlShader && m_glShader) {
        delete m_glShader;
        m_glShader = nullptr;
        m_ownsGlShader = false;
    }
}

bool PanadapterRenderer::ensureRenderTarget(const QSize &pixelSize)
{
    if (pixelSize.isEmpty())
        return false;

    if (m_rtPixelSize == pixelSize && m_rt && m_triStripPipeline)
        return true;

    m_triStripPipeline.reset();
    m_lineStripPipeline.reset();
    m_linesPipeline.reset();
    m_rp.reset();
    m_rt.reset();
    m_colorTex.reset();

    m_rtPixelSize = pixelSize;

    m_colorTex.reset(m_rhi->newTexture(QRhiTexture::RGBA8, pixelSize, 1,
                                       QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
    if (!m_colorTex->create()) {
        qWarning() << "PanadapterRenderer: color texture creation failed";
        return false;
    }

    QRhiTextureRenderTargetDescription desc(QRhiColorAttachment(m_colorTex.get()));
    m_rt.reset(m_rhi->newTextureRenderTarget(desc));
    m_rp.reset(m_rt->newCompatibleRenderPassDescriptor());
    if (!m_rt->create()) {
        qWarning() << "PanadapterRenderer: render target creation failed";
        return false;
    }

  if (!pipelineForTopology(QRhiGraphicsPipeline::TriangleStrip)) {
        qWarning() << "PanadapterRenderer: triangle-strip pipeline creation failed";
        return false;
    }

    return true;
}

QRhiGraphicsPipeline *PanadapterRenderer::pipelineForTopology(QRhiGraphicsPipeline::Topology topology)
{
    std::unique_ptr<QRhiGraphicsPipeline> *slot = nullptr;
    switch (topology) {
    case QRhiGraphicsPipeline::TriangleStrip: slot = &m_triStripPipeline; break;
    case QRhiGraphicsPipeline::LineStrip: slot = &m_lineStripPipeline; break;
    case QRhiGraphicsPipeline::Lines: slot = &m_linesPipeline; break;
    default: return nullptr;
    }

    if (slot->get())
        return slot->get();

    if (!m_rp || !m_vertShader.isValid() || !m_fragShader.isValid())
        return nullptr;

    std::unique_ptr<QRhiGraphicsPipeline> pipeline(m_rhi->newGraphicsPipeline());
    pipeline->setTopology(topology);
    pipeline->setShaderStages({
        { QRhiShaderStage::Vertex, m_vertShader },
        { QRhiShaderStage::Fragment, m_fragShader },
    });

    QRhiVertexInputLayout inputLayout;
    inputLayout.setBindings({ { int(sizeof(VertexData)) } });
    inputLayout.setAttributes({
        { 0, 0, QRhiVertexInputAttribute::Float3, 0 },
        { 0, 1, QRhiVertexInputAttribute::Float4, quint32(3 * sizeof(float)) },
    });
    pipeline->setVertexInputLayout(inputLayout);
    pipeline->setShaderResourceBindings(m_srb.get());
    pipeline->setRenderPassDescriptor(m_rp.get());
    pipeline->setDepthTest(false);
    pipeline->setDepthWrite(false);
    pipeline->setTargetBlends({
        {
            QRhiGraphicsPipeline::ColorMask(QRhiGraphicsPipeline::R
                                            | QRhiGraphicsPipeline::G
                                            | QRhiGraphicsPipeline::B
                                            | QRhiGraphicsPipeline::A),
            true,
            QRhiGraphicsPipeline::SrcAlpha,
            QRhiGraphicsPipeline::OneMinusSrcAlpha,
            QRhiGraphicsPipeline::Add,
            QRhiGraphicsPipeline::One,
            QRhiGraphicsPipeline::OneMinusSrcAlpha,
            QRhiGraphicsPipeline::Add,
        },
    });

    if (!pipeline->create()) {
        qWarning() << "PanadapterRenderer: pipeline create failed for topology" << topology;
        return nullptr;
    }

    *slot = std::move(pipeline);
    return slot->get();
}


void PanadapterRenderer::drawGeometry(QRhiCommandBuffer *cb,
                                      const QMatrix4x4 &matrix,
                                      const VertexData *data,
                                      int count,
                                      QRhiGraphicsPipeline *pipeline,
                                      int vertexCount)
{
    if (!cb || !pipeline || count <= 0 || vertexCount <= 0)
        return;

    const int bytes = int(count * sizeof(VertexData));
    if (!m_vbuf || bytes > m_vbufCapacityBytes) {
        m_vbuf.reset(m_rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::VertexBuffer, bytes));
        if (!m_vbuf->create())
            return;
        m_vbufCapacityBytes = bytes;
    }

    QRhiResourceUpdateBatch *batch = m_rhi->nextResourceUpdateBatch();
    batch->updateDynamicBuffer(m_ubuf.get(), 0, 64, matrix.constData());
    batch->updateDynamicBuffer(m_vbuf.get(), 0, bytes, data);
    cb->resourceUpdate(batch);

    cb->setGraphicsPipeline(pipeline);
    cb->setViewport(QRhiViewport(0, 0, m_rtPixelSize.width(), m_rtPixelSize.height()));
    cb->setShaderResources();
    const QRhiCommandBuffer::VertexInput binding = { m_vbuf.get(), 0 };
    cb->setVertexInput(0, 1, &binding);
    cb->draw(vertexCount);
}

void PanadapterRenderer::render(QOpenGLFunctions *gl,
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
                                bool isCurrentReceiver)
{
    if (bins.isEmpty() || panRect.width() <= 0 || panRect.height() <= 0)
        return;

    if (!m_rhiActive) {
        renderWithOpenGL(projection, panRect, bins, dBmMax, dBmMin, mode, scaleMult, dpr,
                         parentHeight, colors, dataEngineState, isCurrentReceiver, gl);
        return;
    }

    if (!m_rhi)
        return;

    const QSize pixelSize(qMax(1, int(panRect.width() * dpr)), qMax(1, int(panRect.height() * dpr)));
    if (!ensureRenderTarget(pixelSize)) {
        m_rhiActive = false;
        renderWithOpenGL(projection, panRect, bins, dBmMax, dBmMin, mode, scaleMult, dpr,
                         parentHeight, colors, dataEngineState, isCurrentReceiver, gl);
        return;
    }

    QRhiCommandBuffer *cb = nullptr;
    if (m_rhi->beginOffscreenFrame(&cb) != QRhi::FrameOpSuccess || !cb) {
        qWarning() << "PanadapterRenderer: beginOffscreenFrame failed, using OpenGL fallback";
        m_rhiActive = false;
        renderWithOpenGL(projection, panRect, bins, dBmMax, dBmMin, mode, scaleMult, dpr,
                         parentHeight, colors, dataEngineState, isCurrentReceiver, gl);
        return;
    }

    Q_UNUSED(parentHeight);

    QMatrix4x4 matrix = m_rhi->clipSpaceCorrMatrix();
    matrix.ortho(0.0f, float(panRect.width()), float(panRect.height()), 0.0f, -10.0f, 10.0f);

    cb->beginPass(m_rt.get(), QColor(0, 0, 0, 0), { 1.0f, 0 }, nullptr);

    const int vertexArrayLength = bins.size();
    const int x1 = 0;
    const int x2 = panRect.width();
    const int y2 = panRect.height();

    const qreal dBmRange = qAbs(dBmMax - dBmMin);
    if (dBmRange <= 0.0) {
        cb->endPass();
        m_rhi->endOffscreenFrame();
        m_rhi->finish();
        return;
    }

    const float yScale = panRect.height() / float(dBmRange);
    const float yTop = float(y2);

    float r1, g1, b1, r2, g2, b2, r3, g3, b3, r4, g4, b4;
    const float a = 1.0f;
    if (dataEngineState == QSDR::_DataEngineState::DataEngineUp) {
        r1 = 0.8f * colors.bkgR; g1 = 0.8f * colors.bkgG; b1 = 0.8f * colors.bkgB;
        r2 = 0.6f * colors.bkgR; g2 = 0.6f * colors.bkgG; b2 = 0.6f * colors.bkgB;
        r3 = 0.4f * colors.bkgR; g3 = 0.4f * colors.bkgG; b3 = 0.4f * colors.bkgB;
        r4 = 0.2f * colors.bkgR; g4 = 0.2f * colors.bkgG; b4 = 0.2f * colors.bkgB;
        if (!isCurrentReceiver) {
            r1 = r2 = r3 = r4 = 0.4f * colors.bkgR;
            g1 = g2 = g3 = g4 = 0.4f * colors.bkgG;
            b1 = b2 = b3 = b4 = 0.4f * colors.bkgB;
        }
    } else {
        r1 = r2 = r3 = r4 = 0.15f * colors.bkgR;
        g1 = g2 = g3 = g4 = 0.15f * colors.bkgG;
        b1 = b2 = b3 = b4 = 0.15f * colors.bkgB;
    }

    const VertexData bkgData[4] = {
        { float(x1), 0.0f, -4.0f, r1, g1, b1, a },
        { float(x2), 0.0f, -4.0f, r2, g2, b2, a },
        { float(x1), float(y2), -4.0f, r3, g3, b3, a },
        { float(x2), float(y2), -4.0f, r4, g4, b4, a },
    };

    QRhiGraphicsPipeline *triPipeline = pipelineForTopology(QRhiGraphicsPipeline::TriangleStrip);
    QRhiGraphicsPipeline *linePipeline = pipelineForTopology(QRhiGraphicsPipeline::LineStrip);
    QRhiGraphicsPipeline *linesPipeline = pipelineForTopology(QRhiGraphicsPipeline::Lines);

    drawGeometry(cb, matrix, bkgData, 4, triPipeline, 4);

    switch (mode) {
    case FilledLine: {
        m_vertexCache.resize(vertexArrayLength * 2);
        for (int i = 0; i < vertexArrayLength; ++i) {
            const float vx = float(x1 + (i / scaleMult));
            const float vy = float(yTop - yScale * float(bins.at(i)));
            m_vertexCache[2 * i] = { vx, vy, -1.5f, 0.7f * colors.rf, 0.7f * colors.gf, 0.7f * colors.bf, 0.4f };
            m_vertexCache[2 * i + 1] = { vx, yTop, -1.5f, 0.3f * colors.rf, 0.3f * colors.gf, 0.3f * colors.bf, 0.2f };
        }
        drawGeometry(cb, matrix, m_vertexCache.data(), m_vertexCache.size(), triPipeline, vertexArrayLength * 2);

        m_vertexCache.resize(vertexArrayLength);
        for (int i = 0; i < vertexArrayLength; ++i) {
            const float vx = float(x1 + (i / scaleMult));
            m_vertexCache[i] = { vx, float(yTop - yScale * float(bins.at(i))), -1.0f, colors.r, colors.g, colors.b, 1.0f };
        }
        drawGeometry(cb, matrix, m_vertexCache.data(), m_vertexCache.size(), linePipeline, vertexArrayLength);
        break;
    }
    case Line: {
        m_vertexCache.resize(vertexArrayLength);
        for (int i = 0; i < vertexArrayLength; ++i) {
            const float vx = float(x1 + (i / scaleMult));
            m_vertexCache[i] = { vx, float(yTop - yScale * float(bins.at(i))), -1.0f, colors.r, colors.g, colors.b, 1.0f };
        }
        drawGeometry(cb, matrix, m_vertexCache.data(), m_vertexCache.size(), linePipeline, vertexArrayLength);
        break;
    }
    case Solid: {
        m_vertexCache.resize(vertexArrayLength * 2);
        for (int i = 0; i < vertexArrayLength; ++i) {
            const float vx = float(x1 + (i / scaleMult));
            const float vy = float(yTop - yScale * float(bins.at(i)));
            m_vertexCache[2 * i] = { vx, vy, -1.0f, colors.rst, colors.gst, colors.bst, 1.0f };
            m_vertexCache[2 * i + 1] = { vx, yTop, -1.0f, colors.rsb, colors.gsb, colors.bsb, 1.0f };
        }
        drawGeometry(cb, matrix, m_vertexCache.data(), m_vertexCache.size(), linesPipeline, vertexArrayLength * 2);
        break;
    }
    default:
        break;
    }

    cb->endPass();
    m_rhi->endOffscreenFrame();
    m_rhi->finish();
}

void PanadapterRenderer::ensureCompositeResources(QOpenGLFunctions *gl)
{
    if (m_compositeInitialized || !gl)
        return;

    m_compositeProgram = new QOpenGLShaderProgram();
    const QByteArray compositeFrag = GlShaders::texturedFragmentSource("tex").toUtf8();
    m_compositeProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, GlShaders::texturedQuadVertexSource());
    m_compositeProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, compositeFrag.constData());
    m_compositeProgram->bindAttributeLocation("position", 0);
    m_compositeProgram->bindAttributeLocation("texCoord", 1);
    if (!m_compositeProgram->link())
        qWarning() << "PanadapterRenderer: composite shader link failed:" << m_compositeProgram->log();

    m_compositeVbo.create();
    m_compositeVbo.bind();
    m_compositeVbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_compositeVbo.release();

    m_compositeInitialized = true;
}

bool PanadapterRenderer::compositeToDefaultFramebuffer(QOpenGLFunctions *gl,
                                                       const QMatrix4x4& projection,
                                                       const QRect& panRect,
                                                       float dpr,
                                                       int parentHeight)
{
    if (!gl || !m_colorTex || panRect.width() <= 0 || panRect.height() <= 0)
        return false;

    ensureCompositeResources(gl);
    if (!m_compositeProgram || !m_compositeProgram->isLinked())
        return false;

    const QRhiTexture::NativeTexture nativeTex = m_colorTex->nativeTexture();
    if (!nativeTex.object) {
        qWarning() << "PanadapterRenderer: no native GL texture for composite";
        m_rhiActive = false;
        return false;
    }

    const int x1 = panRect.left();
    const int y1 = panRect.top();
    const int x2 = x1 + panRect.width();
    const int y2 = y1 + panRect.height();
    const int height = panRect.height();

    gl->glScissor(int(x1 * dpr), int((parentHeight - y2) * dpr), int((x2 - x1) * dpr), int(height * dpr));
    gl->glEnable(GL_SCISSOR_TEST);
    gl->glDisable(GL_DEPTH_TEST);
    gl->glEnable(GL_BLEND);
    gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // RHI offscreen image is vertically flipped vs. widget ortho coordinates.
    const CompositeVertex quad[4] = {
        { float(x1), float(y1), 0.0f, 1.0f },
        { float(x2), float(y1), 1.0f, 1.0f },
        { float(x1), float(y2), 0.0f, 0.0f },
        { float(x2), float(y2), 1.0f, 0.0f },
    };

    m_compositeProgram->bind();
    m_compositeProgram->setUniformValue("matrix", projection);

    m_compositeVbo.bind();
    m_compositeVbo.allocate(quad, int(sizeof(quad)));
    m_compositeProgram->enableAttributeArray(0);
    m_compositeProgram->setAttributeBuffer(0, GL_FLOAT, 0, 2, int(sizeof(CompositeVertex)));
    m_compositeProgram->enableAttributeArray(1);
    m_compositeProgram->setAttributeBuffer(1, GL_FLOAT, sizeof(float) * 2, 2, int(sizeof(CompositeVertex)));

    gl->glActiveTexture(GL_TEXTURE0);
    gl->glBindTexture(GL_TEXTURE_2D, GLuint(nativeTex.object));
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_compositeProgram->setUniformValue("tex", 0);
    const int colorLoc = m_compositeProgram->uniformLocation("textColor");
    if (colorLoc >= 0)
        m_compositeProgram->setUniformValue(colorLoc, QColor(255, 255, 255));

    gl->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    m_compositeProgram->disableAttributeArray(0);
    m_compositeProgram->disableAttributeArray(1);
    m_compositeVbo.release();
    m_compositeProgram->release();
    gl->glBindTexture(GL_TEXTURE_2D, 0);
    gl->glDisable(GL_SCISSOR_TEST);
    return true;
}

void PanadapterRenderer::renderIdleBackground(QOpenGLFunctions *gl,
                                              const QMatrix4x4& projection,
                                              const QRect& panRect,
                                              float dpr,
                                              int parentHeight,
                                              const Colors& colors,
                                              QSDR::_DataEngineState dataEngineState,
                                              bool isCurrentReceiver)
{
    if (!m_glShader || !m_glShader->isLinked() || !gl || panRect.width() <= 0 || panRect.height() <= 0)
        return;

    const int x1 = panRect.left();
    const int y1 = panRect.top();
    const int x2 = x1 + panRect.width();
    const int y2 = y1 + panRect.height();
    const int height = panRect.height();

    gl->glScissor(int(x1 * dpr), int((parentHeight - y2) * dpr), int((x2 - x1) * dpr), int(height * dpr));
    gl->glEnable(GL_SCISSOR_TEST);
    gl->glEnable(GL_MULTISAMPLE);
    gl->glEnable(GL_LINE_SMOOTH);
    gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gl->glEnable(GL_BLEND);
    gl->glDisable(GL_DEPTH_TEST);

    m_glShader->bind();
    const int matrixLoc = m_glShader->uniformLocation("matrix");
    if (matrixLoc >= 0)
        m_glShader->setUniformValue(matrixLoc, projection);

    if (m_glVao.isCreated())
        m_glVao.bind();
    m_glVbo.bind();

    const int posLoc = m_glShader->attributeLocation("position");
    const int colLoc = m_glShader->attributeLocation("color");
    const GLsizei stride = GLsizei(sizeof(VertexData));

    float r1, g1, b1, r2, g2, b2, r3, g3, b3, r4, g4, b4;
    const float a = 1.0f;
    if (dataEngineState == QSDR::_DataEngineState::DataEngineUp) {
        r1 = 0.8f * colors.bkgR; g1 = 0.8f * colors.bkgG; b1 = 0.8f * colors.bkgB;
        r2 = 0.6f * colors.bkgR; g2 = 0.6f * colors.bkgG; b2 = 0.6f * colors.bkgB;
        r3 = 0.4f * colors.bkgR; g3 = 0.4f * colors.bkgG; b3 = 0.4f * colors.bkgB;
        r4 = 0.2f * colors.bkgR; g4 = 0.2f * colors.bkgG; b4 = 0.2f * colors.bkgB;
        if (!isCurrentReceiver) {
            r1 = r2 = r3 = r4 = 0.4f * colors.bkgR;
            g1 = g2 = g3 = g4 = 0.4f * colors.bkgG;
            b1 = b2 = b3 = b4 = 0.4f * colors.bkgB;
        }
    } else {
        r1 = r2 = r3 = r4 = 0.15f * colors.bkgR;
        g1 = g2 = g3 = g4 = 0.15f * colors.bkgG;
        b1 = b2 = b3 = b4 = 0.15f * colors.bkgB;
    }

    const VertexData bkgData[4] = {
        { (float)x1, (float)y1, -4.0f, r1, g1, b1, a },
        { (float)x2, (float)y1, -4.0f, r2, g2, b2, a },
        { (float)x1, (float)y2, -4.0f, r3, g3, b3, a },
        { (float)x2, (float)y2, -4.0f, r4, g4, b4, a },
    };

    if (int(m_glVboSize) < int(sizeof(bkgData))) {
        m_glVbo.allocate(bkgData, int(sizeof(bkgData)));
        m_glVboSize = int(sizeof(bkgData));
    } else {
        m_glVbo.write(0, bkgData, int(sizeof(bkgData)));
    }
    if (posLoc >= 0) {
        gl->glEnableVertexAttribArray(GLuint(posLoc));
        gl->glVertexAttribPointer(GLuint(posLoc), 3, GL_FLOAT, GL_FALSE, stride,
                                  reinterpret_cast<const void *>(0));
    }
    if (colLoc >= 0) {
        gl->glEnableVertexAttribArray(GLuint(colLoc));
        gl->glVertexAttribPointer(GLuint(colLoc), 4, GL_FLOAT, GL_FALSE, stride,
                                  reinterpret_cast<const void *>(3 * sizeof(float)));
    }
    gl->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    if (posLoc >= 0)
        gl->glDisableVertexAttribArray(GLuint(posLoc));
    if (colLoc >= 0)
        gl->glDisableVertexAttribArray(GLuint(colLoc));
    m_glVbo.release();
    if (m_glVao.isCreated())
        m_glVao.release();
    m_glShader->release();
    gl->glDisable(GL_SCISSOR_TEST);
}

void PanadapterRenderer::renderWithOpenGL(const QMatrix4x4& projection,
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
                                          QOpenGLFunctions *gl)
{
    if (!m_glShader || !m_glShader->isLinked() || !gl)
        return;

    const int vertexArrayLength = bins.size();
    const int x1 = panRect.left();
    const int y1 = panRect.top();
    const int x2 = x1 + panRect.width();
    const int y2 = y1 + panRect.height();
    const int height = panRect.height();

    gl->glScissor(int(x1 * dpr), int((parentHeight - y2) * dpr), int((x2 - x1) * dpr), int(height * dpr));
    gl->glEnable(GL_SCISSOR_TEST);
    gl->glEnable(GL_MULTISAMPLE);
    gl->glEnable(GL_LINE_SMOOTH);
    gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gl->glEnable(GL_BLEND);
    gl->glDisable(GL_DEPTH_TEST);

    const qreal dBmRange = qAbs(dBmMax - dBmMin);
    if (dBmRange <= 0.0) {
        gl->glDisable(GL_SCISSOR_TEST);
        return;
    }

    const float yScale = panRect.height() / float(dBmRange);
    const float yTop = float(y2);

    m_glShader->bind();
    const int matrixLoc = m_glShader->uniformLocation("matrix");
    if (matrixLoc >= 0)
        m_glShader->setUniformValue(matrixLoc, projection);

    if (m_glVao.isCreated())
        m_glVao.bind();
    m_glVbo.bind();

    const int posLoc = m_glShader->attributeLocation("position");
    const int colLoc = m_glShader->attributeLocation("color");
    const GLsizei stride = GLsizei(sizeof(VertexData));

    auto updateVBO = [&](int size, const void *data) {
        if (size > m_glVboSize) {
            m_glVbo.allocate(data, size);
            m_glVboSize = size;
        } else {
            m_glVbo.write(0, data, size);
        }
        if (posLoc >= 0) {
            gl->glEnableVertexAttribArray(GLuint(posLoc));
            gl->glVertexAttribPointer(GLuint(posLoc), 3, GL_FLOAT, GL_FALSE, stride,
                                      reinterpret_cast<const void *>(0));
        }
        if (colLoc >= 0) {
            gl->glEnableVertexAttribArray(GLuint(colLoc));
            gl->glVertexAttribPointer(GLuint(colLoc), 4, GL_FLOAT, GL_FALSE, stride,
                                      reinterpret_cast<const void *>(3 * sizeof(float)));
        }
    };

    float r1, g1, b1, r2, g2, b2, r3, g3, b3, r4, g4, b4;
    const float a = 1.0f;
    if (dataEngineState == QSDR::_DataEngineState::DataEngineUp) {
        r1 = 0.8f * colors.bkgR; g1 = 0.8f * colors.bkgG; b1 = 0.8f * colors.bkgB;
        r2 = 0.6f * colors.bkgR; g2 = 0.6f * colors.bkgG; b2 = 0.6f * colors.bkgB;
        r3 = 0.4f * colors.bkgR; g3 = 0.4f * colors.bkgG; b3 = 0.4f * colors.bkgB;
        r4 = 0.2f * colors.bkgR; g4 = 0.2f * colors.bkgG; b4 = 0.2f * colors.bkgB;
        if (!isCurrentReceiver) {
            r1 = r2 = r3 = r4 = 0.4f * colors.bkgR;
            g1 = g2 = g3 = g4 = 0.4f * colors.bkgG;
            b1 = b2 = b3 = b4 = 0.4f * colors.bkgB;
        }
    } else {
        r1 = r2 = r3 = r4 = 0.15f * colors.bkgR;
        g1 = g2 = g3 = g4 = 0.15f * colors.bkgG;
        b1 = b2 = b3 = b4 = 0.15f * colors.bkgB;
    }

    const VertexData bkgData[4] = {
        { (float)x1, (float)y1, -4.0f, r1, g1, b1, a },
        { (float)x2, (float)y1, -4.0f, r2, g2, b2, a },
        { (float)x1, (float)y2, -4.0f, r3, g3, b3, a },
        { (float)x2, (float)y2, -4.0f, r4, g4, b4, a },
    };
    updateVBO(4 * int(sizeof(VertexData)), bkgData);
    gl->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    switch (mode) {
    case FilledLine: {
        m_vertexCache.resize(vertexArrayLength * 2);
        for (int i = 0; i < vertexArrayLength; ++i) {
            const float vx = float(x1 + (i / scaleMult));
            const float vy = float(yTop - yScale * float(bins.at(i)));
            m_vertexCache[2 * i] = { vx, vy, -1.5f, 0.7f * colors.rf, 0.7f * colors.gf, 0.7f * colors.bf, 0.4f };
            m_vertexCache[2 * i + 1] = { vx, yTop, -1.5f, 0.3f * colors.rf, 0.3f * colors.gf, 0.3f * colors.bf, 0.2f };
        }
        updateVBO(int(m_vertexCache.size() * sizeof(VertexData)), m_vertexCache.data());
        gl->glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexArrayLength * 2);

        m_vertexCache.resize(vertexArrayLength);
        for (int i = 0; i < vertexArrayLength; ++i) {
            const float vx = float(x1 + (i / scaleMult));
            m_vertexCache[i] = { vx, float(yTop - yScale * float(bins.at(i))), -1.0f, colors.r, colors.g, colors.b, 1.0f };
        }
        updateVBO(int(m_vertexCache.size() * sizeof(VertexData)), m_vertexCache.data());
        gl->glDrawArrays(GL_LINE_STRIP, 0, vertexArrayLength);
        break;
    }
    case Line: {
        m_vertexCache.resize(vertexArrayLength);
        for (int i = 0; i < vertexArrayLength; ++i) {
            const float vx = float(x1 + (i / scaleMult));
            m_vertexCache[i] = { vx, float(yTop - yScale * float(bins.at(i))), -1.0f, colors.r, colors.g, colors.b, 1.0f };
        }
        updateVBO(int(m_vertexCache.size() * sizeof(VertexData)), m_vertexCache.data());
        gl->glDrawArrays(GL_LINE_STRIP, 0, vertexArrayLength);
        break;
    }
    case Solid: {
        m_vertexCache.resize(vertexArrayLength * 2);
        for (int i = 0; i < vertexArrayLength; ++i) {
            const float vx = float(x1 + (i / scaleMult));
            const float vy = float(yTop - yScale * float(bins.at(i)));
            m_vertexCache[2 * i] = { vx, vy, -1.0f, colors.rst, colors.gst, colors.bst, 1.0f };
            m_vertexCache[2 * i + 1] = { vx, yTop, -1.0f, colors.rsb, colors.gsb, colors.bsb, 1.0f };
        }
        updateVBO(int(m_vertexCache.size() * sizeof(VertexData)), m_vertexCache.data());
        gl->glDrawArrays(GL_LINES, 0, vertexArrayLength * 2);
        break;
    }
    default:
        break;
    }

    if (posLoc >= 0)
        gl->glDisableVertexAttribArray(GLuint(posLoc));
    if (colLoc >= 0)
        gl->glDisableVertexAttribArray(GLuint(colLoc));
    m_glVbo.release();
    if (m_glVao.isCreated())
        m_glVao.release();
    m_glShader->release();
    gl->glDisable(GL_SCISSOR_TEST);
}
