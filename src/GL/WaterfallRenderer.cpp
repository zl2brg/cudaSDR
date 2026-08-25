#include "WaterfallRenderer.h"
#include "cusdr_glShaders.h"
#include <QDebug>
#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <QtMath>

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

void expandPixelRow(const float* logical, int logicalWidth,
                    float* devRow, int devWidth, qreal dpr)
{
    const qreal ratio = qMax<qreal>(1.0, dpr);
    for (int x = 0; x < devWidth; ++x) {
        const int lx = qBound(0, int(qFloor(x / ratio)), logicalWidth - 1);
        devRow[x] = logical[lx];
    }
}

int lerpByte(int a, int b, float t)
{
    return int((1.0f - t) * float(a) + t * float(b));
}

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
    t = qBound(0.0f, t, 1.0f);

    if (mode == Simple) {
        if (t <= 0.0f)
            return packRgb(lo.red(), lo.green(), lo.blue());
        if (t >= 1.0f)
            return packRgb(255, 255, 255);
        if (t <= 0.5f) {
            const float p = t * 2.0f;
            return packRgb(lerpByte(lo.red(), mid.red(), p),
                           lerpByte(lo.green(), mid.green(), p),
                           lerpByte(lo.blue(), mid.blue(), p));
        }
        const float p = (t - 0.5f) * 2.0f;
        return packRgb(lerpByte(mid.red(), 255, p),
                       lerpByte(mid.green(), 255, p),
                       lerpByte(mid.blue(), 255, p));
    }

    // Enhanced — PowerSDR / KISS Konsole segments.
    if (t <= 0.0f)
        return packRgb(lo.red(), lo.green(), lo.blue());
    if (t >= 1.0f)
        return packRgb(hi.red(), hi.green(), hi.blue());

    int r, g, b;
    if (t < 2.0f / 9.0f) {
        const float p = t / (2.0f / 9.0f);
        r = int((1.0f - p) * lo.red());
        g = int((1.0f - p) * lo.green());
        b = int(lo.blue() + p * (255 - lo.blue()));
    } else if (t < 3.0f / 9.0f) {
        const float p = (t - 2.0f / 9.0f) / (1.0f / 9.0f);
        r = 0;
        g = int(p * 255);
        b = 255;
    } else if (t < 4.0f / 9.0f) {
        const float p = (t - 3.0f / 9.0f) / (1.0f / 9.0f);
        r = 0;
        g = 255;
        b = int((1.0f - p) * 255);
    } else if (t < 5.0f / 9.0f) {
        const float p = (t - 4.0f / 9.0f) / (1.0f / 9.0f);
        r = int(p * 255);
        g = 255;
        b = 0;
    } else if (t < 7.0f / 9.0f) {
        const float p = (t - 5.0f / 9.0f) / (2.0f / 9.0f);
        r = 255;
        g = int((1.0f - p) * 255);
        b = 0;
    } else if (t < 8.0f / 9.0f) {
        const float p = (t - 7.0f / 9.0f) / (1.0f / 9.0f);
        r = 255;
        g = 0;
        b = int(p * 255);
    } else {
        const float p = (t - 8.0f / 9.0f) / (1.0f / 9.0f);
        r = int((0.75f + 0.25f * (1.0f - p)) * 255);
        g = int(p * 255 * 0.5f);
        b = 255;
    }
    return packRgb(r, g, b);
}

} // namespace

WaterfallRenderer::WaterfallRenderer()
    : m_textureId(0)
    , m_lutId(0)
    , m_headLine(0)
    , m_oldWidth(0)
    , m_oldHeight(0)
    , m_oldDpr(0.0f)
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

WaterfallRenderer::~WaterfallRenderer() {
    if (m_vao.isCreated()) m_vao.destroy();
    if (m_vbo.isCreated()) m_vbo.destroy();
    if (m_shader) delete m_shader;
    if (QOpenGLContext::currentContext()) {
        if (m_textureId != 0) {
            glDeleteTextures(1, &m_textureId);
            m_textureId = 0;
        }
        if (m_lutId != 0) {
            glDeleteTextures(1, &m_lutId);
            m_lutId = 0;
        }
        for (int i = 0; i < 2; ++i) {
            if (m_pbo[i].isCreated())
                m_pbo[i].destroy();
        }
    }
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
    m_devRow.resize(width);

    QVector<float> quiet(width * height, kEmptyDbm);
    allocateTexture2D(GL_R32F, width, height, GL_RED, GL_FLOAT, quiet.constData());

    m_headLine = 0;
    m_oldWidth = width;
    m_oldHeight = height;
    m_updatePending = false;
}

void WaterfallRenderer::uploadNewRow(const float* logical, int logicalWidth,
                                     int texWidth, int texHeight, qreal dpr)
{
    m_headLine = (m_headLine - 1 + texHeight) % texHeight;
    glBindTexture(GL_TEXTURE_2D, m_textureId);

    const int rowBytes = texWidth * int(sizeof(float));
    QOpenGLBuffer &pbo = m_pbo[m_pboIndex];

    if (pbo.isCreated()) {
        pbo.bind();
        pbo.allocate(rowBytes);

        auto *mapped = static_cast<float *>(
            pbo.mapRange(0, rowBytes,
                         QOpenGLBuffer::RangeWrite | QOpenGLBuffer::RangeInvalidateBuffer));
        if (mapped) {
            expandPixelRow(logical, logicalWidth, mapped, texWidth, dpr);
            pbo.unmap();
        } else {
            m_devRow.resize(texWidth);
            expandPixelRow(logical, logicalWidth, m_devRow.data(), texWidth, dpr);
            pbo.write(0, m_devRow.constData(), rowBytes);
        }

        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, m_headLine, texWidth, 1,
                        GL_RED, GL_FLOAT, nullptr);
        pbo.release();
        m_pboIndex ^= 1;
        return;
    }

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    m_devRow.resize(texWidth);
    expandPixelRow(logical, logicalWidth, m_devRow.data(), texWidth, dpr);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, m_headLine, texWidth, 1,
                    GL_RED, GL_FLOAT, m_devRow.constData());
}

void WaterfallRenderer::render(const QMatrix4x4& projection, const QRect& rect, const QVarLengthArray<float>& intensity,
                               QSDR::_DataEngineState dataEngineState, float dpr, bool newLine, const WaterfallMapping& mapping) {
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

    if (newLine && dataEngineState == QSDR::DataEngineUp
        && !intensity.isEmpty() && intensity.size() >= logicalWidth) {
        uploadNewRow(intensity.constData(), logicalWidth, texWidth, texHeight, ratio);
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
