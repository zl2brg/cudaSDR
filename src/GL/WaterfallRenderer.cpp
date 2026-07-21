#include "WaterfallRenderer.h"
#include "cusdr_glShaders.h"
#include <QDebug>
#include <QOpenGLContext>

WaterfallRenderer::WaterfallRenderer()
    : m_textureId(0)
    , m_currentLine(0)
    , m_lineCnt(0)
    , m_oldWidth(0)
    , m_oldHeight(0)
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
    // Flush any pending PBO upload before rebuilding the texture/PBOs.
    if (m_pboActive && m_textureId != 0 && m_oldWidth > 0) {
        glBindTexture(GL_TEXTURE_2D, m_textureId);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pboIds[1 - m_pboIndex]);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, m_prevLine, m_oldWidth, 1, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        m_pboActive = false;
    }

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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, blackBuffer.data());
    
    // Allocate PBOs memory for the new width
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pboIds[0]);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, width * sizeof(TGL_ubyteRGBA), NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pboIds[1]);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, width * sizeof(TGL_ubyteRGBA), NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    m_currentLine = 0;
    m_lineCnt = 0;
    m_oldWidth = width;
    m_oldHeight = height;
    m_updatePending = false;
    m_pboIndex = 0;
    m_pboActive = false;
}

void WaterfallRenderer::render(const QMatrix4x4& projection, const QRect& rect, const QVarLengthArray<TGL_ubyteRGBA>& pixelData, QSDR::_DataEngineState dataEngineState) {
    if (rect.isEmpty() || !m_shader || !m_shader->isLinked())
        return;

    int width = rect.width();
    int height = rect.height();
    float top = (float)rect.top();
    float left = (float)rect.left();
    float right = left + (float)width;
    float bottom = top + (float)height;

    if (m_textureId == 0 || m_oldWidth != width || m_oldHeight != height || m_updatePending) {
        setupTexture(width, height);
    }

    if (dataEngineState == QSDR::DataEngineUp && !pixelData.isEmpty() && pixelData.size() >= width) {
        glBindTexture(GL_TEXTURE_2D, m_textureId);

        if (m_pboActive) {
            // Unpack from the other PBO (contains previous frame's data)
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pboIds[1 - m_pboIndex]);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, m_prevLine, width, 1, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        }

        // Fill current PBO with the new line's data
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pboIds[m_pboIndex]);
        glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, width * sizeof(TGL_ubyteRGBA), pixelData.constData());

        m_prevLine = m_currentLine;
        m_currentLine = (m_currentLine + 1) % height;
        if (m_lineCnt < height) m_lineCnt++;

        m_pboIndex = 1 - m_pboIndex;
        m_pboActive = true;

        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
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

    float v0 = (float)m_currentLine / height;
    
    // We draw the waterfall using two quads to handle the wrap-around texture.
    // Quad 1: Top part of display (showing the latest lines)
    // Quad 2: Bottom part of display (showing older lines)
    
    float splitY = top + (float)(m_currentLine + 1);
    if (dataEngineState != QSDR::DataEngineUp) {
        splitY = top + (float)((m_currentLine + height) % height);
    }

    VertexData vertices[12]; // 2 quads * 6 vertices (triangles)
    
    float h1 = (float)(m_currentLine + 1) / height;

    // Quad 1 (Top to current line)
    // Maps texture from currentLine (v0) up to the beginning of texture (0)
    vertices[0] = { left,  top,    -3.0f, 0.0f, v0 };
    vertices[1] = { right, top,    -3.0f, 1.0f, v0 };
    vertices[2] = { left,  splitY, -3.0f, 0.0f, 0.0f };
    vertices[3] = { right, top,    -3.0f, 1.0f, v0 };
    vertices[4] = { right, splitY, -3.0f, 1.0f, 0.0f };
    vertices[5] = { left,  splitY, -3.0f, 0.0f, 0.0f };

    // Quad 2 (current line to bottom)
    // Maps texture from end of texture (1) down to currentLine + 1 (h1)
    vertices[6]  = { left,  splitY, -3.0f, 0.0f, 1.0f };
    vertices[7]  = { right, splitY, -3.0f, 1.0f, 1.0f };
    vertices[8]  = { left,  bottom, -3.0f, 0.0f, h1 };
    vertices[9]  = { right, splitY, -3.0f, 1.0f, 1.0f };
    vertices[10] = { right, bottom, -3.0f, 1.0f, h1 };
    vertices[11] = { left,  bottom, -3.0f, 0.0f, h1 };

    m_vbo.allocate(vertices, 12 * sizeof(VertexData));
    m_shader->enableAttributeArray(0);
    m_shader->enableAttributeArray(1);
    m_shader->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(float) * 5);
    m_shader->setAttributeBuffer(1, GL_FLOAT, sizeof(float) * 3, 2, sizeof(float) * 5);

    glDrawArrays(GL_TRIANGLES, 0, 12);

    m_shader->disableAttributeArray(0);
    m_shader->disableAttributeArray(1);
    m_vao.release();
    m_vbo.release();
    glBindTexture(GL_TEXTURE_2D, 0);
    m_shader->release();
}
