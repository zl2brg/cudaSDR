#include "WaterfallRenderer.h"
#include "cusdr_glShaders.h"
#include <QDebug>
#include <QOpenGLContext>
#include <QtMath>

namespace {

void expandPixelRow(const TGL_ubyteRGBA* logical, int logicalWidth,
                    TGL_ubyteRGBA* devRow, int devWidth, qreal dpr)
{
    const qreal ratio = qMax<qreal>(1.0, dpr);
    for (int x = 0; x < devWidth; ++x) {
        const int lx = qBound(0, int(qFloor(x / ratio)), logicalWidth - 1);
        devRow[x] = logical[lx];
    }
}

} // namespace

WaterfallRenderer::WaterfallRenderer()
    : m_textureId(0)
    , m_currentLine(0)
    , m_lineCnt(0)
    , m_oldWidth(0)
    , m_oldHeight(0)
    , m_oldDpr(0.0f)
    , m_updatePending(false)
    , m_shader(nullptr)
    , m_vbo(QOpenGLBuffer::VertexBuffer)
    , m_pboIndex(0)
    , m_prevLine(0)
    , m_pboActive(false)
{
    m_pboIds[0] = 0;
    m_pboIds[1] = 0;
}

WaterfallRenderer::~WaterfallRenderer() {
    if (m_vao.isCreated()) m_vao.destroy();
    if (m_vbo.isCreated()) m_vbo.destroy();
    if (m_shader) delete m_shader;
    // GL object deletion requires a current context; callers must makeCurrent()
    // before destroying this renderer, or accept that Qt will reclaim context resources.
    if (QOpenGLContext::currentContext()) {
        if (m_textureId != 0) {
            glDeleteTextures(1, &m_textureId);
            m_textureId = 0;
        }
        if (m_pboIds[0] != 0) {
            glDeleteBuffers(2, m_pboIds);
            m_pboIds[0] = m_pboIds[1] = 0;
        }
    }
}

void WaterfallRenderer::initialize() {
    initializeOpenGLFunctions();

    glGenBuffers(2, m_pboIds);

    m_shader = new QOpenGLShaderProgram();

    const QByteArray fragSrc = GlShaders::waterfallFragmentSource("waterfallTexture").toUtf8();
    m_shader->addShaderFromSourceCode(QOpenGLShader::Vertex, GlShaders::texturedVertexSource());
    m_shader->addShaderFromSourceCode(QOpenGLShader::Fragment, fragSrc.constData());
    m_shader->bindAttributeLocation("position", 0);
    m_shader->bindAttributeLocation("texCoord", 1);
    if (!m_shader->link())
        qWarning() << "WaterfallRenderer: shader link failed:" << m_shader->log();

    m_vao.create();
    m_vao.bind();
    m_vbo.create();
    m_vbo.bind();
    m_vbo.setUsagePattern(QOpenGLBuffer::StreamDraw);
    
    m_shader->bind();
    m_shader->enableAttributeArray(0);
    m_shader->enableAttributeArray(1);
    m_shader->release();
    
    m_vao.release();
    m_vbo.release();
}

void WaterfallRenderer::reset() {
    m_updatePending = true;
}

void WaterfallRenderer::setupTexture(int width, int height) {
    if (m_textureId != 0) {
        glDeleteTextures(1, &m_textureId);
    }
    glGenTextures(1, &m_textureId);
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    QVector<TGL_ubyteRGBA> blackBuffer(width * height);
    TGL_ubyteRGBA black; black.red = 0; black.green = 0; black.blue = 0; black.alpha = 255;
    blackBuffer.fill(black);
    m_textureBuffer = blackBuffer;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, blackBuffer.data());
    
    m_currentLine = 0;
    m_lineCnt = 0;
    m_oldWidth = width;
    m_oldHeight = height;
    m_updatePending = false;
    m_pboIndex = 0;
    m_pboActive = false;
}

void WaterfallRenderer::render(const QMatrix4x4& projection, const QRect& rect, const QVarLengthArray<TGL_ubyteRGBA>& pixelData, QSDR::_DataEngineState dataEngineState, float dpr) {
    if (rect.isEmpty() || !m_shader || !m_shader->isLinked())
        return;

    const int logicalWidth = rect.width();
    const int logicalHeight = rect.height();
    const int texWidth = qMax(1, int(qRound(logicalWidth * dpr)));
    const int texHeight = qMax(1, int(qRound(logicalHeight * dpr)));
    const qreal ratio = qMax<qreal>(1.0, dpr);

    float top = (float)rect.top();
    float left = (float)rect.left();
    float right = left + (float)logicalWidth;
    float bottom = top + (float)logicalHeight;

    if (m_textureId == 0 || m_oldWidth != texWidth || m_oldHeight != texHeight
        || !qFuzzyCompare(m_oldDpr, dpr) || m_updatePending) {
        setupTexture(texWidth, texHeight);
        m_oldDpr = dpr;
    }

    if (dataEngineState == QSDR::DataEngineUp && !pixelData.isEmpty() && pixelData.size() >= logicalWidth) {
        glBindTexture(GL_TEXTURE_2D, m_textureId);
        if (m_textureBuffer.size() != texWidth * texHeight) {
            TGL_ubyteRGBA black; black.red = 0; black.green = 0; black.blue = 0; black.alpha = 255;
            m_textureBuffer.resize(texWidth * texHeight);
            m_textureBuffer.fill(black);
        }

        QVector<TGL_ubyteRGBA> devRow(texWidth);
        expandPixelRow(pixelData.constData(), logicalWidth, devRow.data(), texWidth, ratio);

        // Store history at device-pixel resolution so each spectrum row maps 1:1
        // to a device pixel row when the DPR-scaled viewport is used.
        if (texHeight > 1) {
            memmove(m_textureBuffer.data() + texWidth,
                    m_textureBuffer.constData(),
                    static_cast<size_t>(texWidth) * static_cast<size_t>(texHeight - 1) * sizeof(TGL_ubyteRGBA));
        }
        memcpy(m_textureBuffer.data(),
               devRow.constData(),
               static_cast<size_t>(texWidth) * sizeof(TGL_ubyteRGBA));

        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texWidth, texHeight, GL_RGBA, GL_UNSIGNED_BYTE, m_textureBuffer.constData());
        m_currentLine = 0;
        if (m_lineCnt < texHeight) m_lineCnt++;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    m_shader->bind();
    m_shader->setUniformValue("matrix", projection);

    m_vao.bind();
    m_vbo.bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    const int texUniform = m_shader->uniformLocation("waterfallTexture");
    if (texUniform >= 0)
        m_shader->setUniformValue(texUniform, 0);

    VertexData vertices[6];
    vertices[0] = { left,  top,    -3.0f, 0.0f, 0.0f };
    vertices[1] = { right, top,    -3.0f, 1.0f, 0.0f };
    vertices[2] = { left,  bottom, -3.0f, 0.0f, 1.0f };
    vertices[3] = { right, top,    -3.0f, 1.0f, 0.0f };
    vertices[4] = { right, bottom, -3.0f, 1.0f, 1.0f };
    vertices[5] = { left,  bottom, -3.0f, 0.0f, 1.0f };

    m_vbo.allocate(vertices, 6 * sizeof(VertexData));
    m_shader->enableAttributeArray(0);
    m_shader->enableAttributeArray(1);
    m_shader->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(float) * 5);
    m_shader->setAttributeBuffer(1, GL_FLOAT, sizeof(float) * 3, 2, sizeof(float) * 5);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    m_shader->disableAttributeArray(0);
    m_shader->disableAttributeArray(1);
    m_vao.release();
    m_vbo.release();
    glBindTexture(GL_TEXTURE_2D, 0);
    m_shader->release();
}
