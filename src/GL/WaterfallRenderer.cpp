#include "WaterfallRenderer.h"
#include "WaterfallPalette.h"
#include "cusdr_glShaders.h"
#include <QDebug>
#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <QtMath>
#include <cstring>

#ifndef GL_RED
#define GL_RED 0x1903
#endif
#ifndef GL_R32F
#define GL_R32F 0x822E
#endif
#ifndef GL_RGBA8
#define GL_RGBA8 0x8058
#endif

namespace {

constexpr int kPaletteLutSize = 256;
constexpr float kEmptyDbm = -1000.0f;

TGL_ubyteRGBA packRgb(int r, int g, int b)
{
    TGL_ubyteRGBA c;
    c.red = GLubyte(qBound(0, r, 255));
    c.green = GLubyte(qBound(0, g, 255));
    c.blue = GLubyte(qBound(0, b, 255));
    c.alpha = 255;
    return c;
}

TGL_ubyteRGBA paletteColor(WaterfallColorMode mode, float t,
                           const QColor& lo, const QColor& mid, const QColor& hi)
{
    SignalPalette palette;
    palette.mode = mode;
    palette.lo = lo;
    palette.mid = mid;
    palette.hi = hi;
    const PaletteRgb c = waterfallPaletteColor(palette, t);
    return packRgb(c.r, c.g, c.b);
}

} // namespace

WaterfallRenderer::WaterfallRenderer()
    : m_textureId(0)
    , m_lutId(0)
    , m_headLine(0)
    , m_oldWidth(0)
    , m_oldHeight(0)
    , m_updatePending(false)
    , m_hasTexStorage(false)
    , m_glTexStorage2D(nullptr)
    , m_shader(nullptr)
    , m_vbo(QOpenGLBuffer::VertexBuffer)
    , m_pboIndex(0)
    , m_lutMode(Simple)
{
    m_pbo[0] = QOpenGLBuffer(QOpenGLBuffer::PixelUnpackBuffer);
    m_pbo[1] = QOpenGLBuffer(QOpenGLBuffer::PixelUnpackBuffer);
}

void WaterfallRenderer::release() {
    if (QOpenGLContext *ctx = QOpenGLContext::currentContext()) {
        QOpenGLFunctions *gl = ctx->functions();
        if (m_vao.isCreated())
            m_vao.destroy();
        if (m_vbo.isCreated())
            m_vbo.destroy();
        if (m_textureId != 0 && gl) {
            gl->glDeleteTextures(1, &m_textureId);
            m_textureId = 0;
        }
        if (m_lutId != 0 && gl) {
            gl->glDeleteTextures(1, &m_lutId);
            m_lutId = 0;
        }
        for (int i = 0; i < 2; ++i) {
            if (m_pbo[i].isCreated())
                m_pbo[i].destroy();
        }
    }
    delete m_shader;
    m_shader = nullptr;
    m_oldWidth = 0;
    m_oldHeight = 0;
    m_headLine = 0;
    m_updatePending = true;
    m_lutMode = Simple;
    m_lutLo = QColor();
    m_lutMid = QColor();
    m_lutHi = QColor();
}

WaterfallRenderer::~WaterfallRenderer() {
    release();
}

void WaterfallRenderer::resolveTexStorage()
{
    m_hasTexStorage = false;
    m_glTexStorage2D = nullptr;

    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    if (!ctx)
        return;

    const QSurfaceFormat fmt = ctx->format();
    const int major = fmt.majorVersion();
    const int minor = fmt.minorVersion();
    const bool gl42 = major > 4 || (major == 4 && minor >= 2);
    const bool es30 = ctx->isOpenGLES() && major >= 3;
    if (!gl42 && !es30
        && !ctx->hasExtension(QByteArrayLiteral("GL_ARB_texture_storage"))
        && !ctx->hasExtension(QByteArrayLiteral("GL_EXT_texture_storage")))
        return;

    m_glTexStorage2D = reinterpret_cast<TexStorage2DFn>(ctx->getProcAddress("glTexStorage2D"));
    if (!m_glTexStorage2D)
        m_glTexStorage2D = reinterpret_cast<TexStorage2DFn>(ctx->getProcAddress("glTexStorage2DEXT"));
    m_hasTexStorage = m_glTexStorage2D != nullptr;
    if (m_hasTexStorage)
        qDebug() << "WaterfallRenderer: using immutable texture storage";
}

void WaterfallRenderer::allocateTexture2D(GLenum internalFormat, int width, int height,
                                          GLenum uploadFormat, GLenum uploadType, const void *pixels)
{
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    if (m_hasTexStorage && m_glTexStorage2D) {
        m_glTexStorage2D(GL_TEXTURE_2D, 1, internalFormat, width, height);
        if (pixels)
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, uploadFormat, uploadType, pixels);
        return;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, uploadFormat, uploadType, pixels);
}

void WaterfallRenderer::initialize() {
    release();
    initializeOpenGLFunctions();
    resolveTexStorage();

    m_shader = new QOpenGLShaderProgram();

    const QByteArray fragSrc = GlShaders::waterfallFragmentSource("waterfallTexture", "paletteLUT").toUtf8();
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

    for (int i = 0; i < 2; ++i) {
        if (!m_pbo[i].create())
            qWarning() << "WaterfallRenderer: PBO create failed" << i;
        else
            m_pbo[i].setUsagePattern(QOpenGLBuffer::StreamDraw);
    }

    glGenTextures(1, &m_lutId);
    glBindTexture(GL_TEXTURE_2D, m_lutId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    QVector<TGL_ubyteRGBA> lut(kPaletteLutSize);
    lut.fill(packRgb(0, 0, 0));
    allocateTexture2D(GL_RGBA8, kPaletteLutSize, 1, GL_RGBA, GL_UNSIGNED_BYTE, lut.constData());
    glBindTexture(GL_TEXTURE_2D, 0);
}

void WaterfallRenderer::reset() {
    m_updatePending = true;
}

void WaterfallRenderer::ensurePaletteLUT(const WaterfallMapping& mapping)
{
    if (m_lutId == 0)
        return;
    if (m_lutMode == mapping.mode && m_lutLo == mapping.lo
        && m_lutMid == mapping.mid && m_lutHi == mapping.hi)
        return;

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    QVector<TGL_ubyteRGBA> lut(kPaletteLutSize);
    const float denom = float(kPaletteLutSize - 1);
    for (int i = 0; i < kPaletteLutSize; ++i)
        lut[i] = paletteColor(mapping.mode, float(i) / denom, mapping.lo, mapping.mid, mapping.hi);

    glBindTexture(GL_TEXTURE_2D, m_lutId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kPaletteLutSize, 1,
                    GL_RGBA, GL_UNSIGNED_BYTE, lut.constData());
    glBindTexture(GL_TEXTURE_2D, 0);

    m_lutMode = mapping.mode;
    m_lutLo = mapping.lo;
    m_lutMid = mapping.mid;
    m_lutHi = mapping.hi;
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

    const int rowBytes = width * int(sizeof(float));
    for (int i = 0; i < 2; ++i) {
        if (!m_pbo[i].isCreated())
            continue;
        m_pbo[i].bind();
        m_pbo[i].allocate(rowBytes);
        m_pbo[i].release();
    }
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    m_pboIndex = 0;

    QVector<float> quiet(width * height, kEmptyDbm);
    allocateTexture2D(GL_R32F, width, height, GL_RED, GL_FLOAT, quiet.constData());

    m_headLine = 0;
    m_oldWidth = width;
    m_oldHeight = height;
    m_updatePending = false;
}

void WaterfallRenderer::uploadNewRow(const float* row, int width, int height)
{
    m_headLine = (m_headLine - 1 + height) % height;
    glBindTexture(GL_TEXTURE_2D, m_textureId);

    const int rowBytes = width * int(sizeof(float));
    QOpenGLBuffer &pbo = m_pbo[m_pboIndex];

    if (pbo.isCreated()) {
        pbo.bind();

        void *mapped = pbo.mapRange(0, rowBytes,
                                    QOpenGLBuffer::RangeWrite | QOpenGLBuffer::RangeInvalidateBuffer);
        if (mapped) {
            std::memcpy(mapped, row, size_t(rowBytes));
            pbo.unmap();
        } else {
            pbo.write(0, row, rowBytes);
        }

        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, m_headLine, width, 1,
                        GL_RED, GL_FLOAT, nullptr);
        pbo.release();
        m_pboIndex ^= 1;
        return;
    }

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, m_headLine, width, 1,
                    GL_RED, GL_FLOAT, row);
}

void WaterfallRenderer::render(const QMatrix4x4& projection, const QRect& rect, const QVarLengthArray<float>& intensity,
                               QSDR::_DataEngineState dataEngineState, float dpr, bool newLine, const WaterfallMapping& mapping) {
    Q_UNUSED(dpr)
    if (rect.isEmpty() || !m_shader || !m_shader->isLinked())
        return;

    const int texWidth = rect.width();
    const int texHeight = rect.height();

    float top = (float)rect.top();
    float left = (float)rect.left();
    float right = left + (float)texWidth;
    float bottom = top + (float)texHeight;

    if (m_textureId == 0 || m_oldWidth != texWidth || m_oldHeight != texHeight || m_updatePending)
        setupTexture(texWidth, texHeight);

    if (newLine && dataEngineState == QSDR::DataEngineUp
        && !intensity.isEmpty() && intensity.size() >= texWidth) {
        uploadNewRow(intensity.constData(), texWidth, texHeight);
    }

    ensurePaletteLUT(mapping);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    m_shader->bind();
    m_shader->setUniformValue("matrix", projection);
    m_shader->setUniformValue("lowerThreshold", mapping.lowerThreshold);
    m_shader->setUniformValue("upperThreshold", mapping.upperThreshold);
    m_shader->setUniformValue("colorRange", mapping.colorRange);
    m_shader->setUniformValue("paletteMode", int(mapping.mode));
    m_shader->setUniformValue("alpha", mapping.alpha);

    m_vao.bind();
    m_vbo.bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_lutId);

    const int texUniform = m_shader->uniformLocation("waterfallTexture");
    if (texUniform >= 0)
        m_shader->setUniformValue(texUniform, 0);
    const int lutUniform = m_shader->uniformLocation("paletteLUT");
    if (lutUniform >= 0)
        m_shader->setUniformValue(lutUniform, 1);

    const float vTop = (texHeight > 0) ? (float(m_headLine) / float(texHeight)) : 0.0f;
    const float vBottom = vTop + 1.0f;

    VertexData vertices[6];
    vertices[0] = { left,  top,    -3.0f, 0.0f, vTop };
    vertices[1] = { right, top,    -3.0f, 1.0f, vTop };
    vertices[2] = { left,  bottom, -3.0f, 0.0f, vBottom };
    vertices[3] = { right, top,    -3.0f, 1.0f, vTop };
    vertices[4] = { right, bottom, -3.0f, 1.0f, vBottom };
    vertices[5] = { left,  bottom, -3.0f, 0.0f, vBottom };

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
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    m_shader->release();
}
