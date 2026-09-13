/**
 * @file  opengl_offscreen_tests.cpp
 * @brief Automated headless OpenGL test suite using QOffscreenSurface.
 *
 * Tests shader compilation, VAO/VBO/PBO resource lifecycle, WaterfallRenderer PBO streaming,
 * PanadapterRenderer spectrum drawing, and offscreen FBO rendering without requiring a display server.
 *
 * @author Simon Eatough <simon.eatough@gmail.com>
 * @date 2026-09-09
 */

#include <QtTest/QtTest>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLFramebufferObject>
#include <QMatrix4x4>
#include <QVarLengthArray>
#include <cmath>

#include "GL/cusdr_glShaders.h"
#include "GL/PanadapterRenderer.h"
#include "GL/WaterfallRenderer.h"
#include "GL/OverlayRenderer.h"
#include "cusdr_settings.h"

class OpenglOffscreenTests : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testOffscreenContextAndCapabilities();
    void testGlShadersColoredCompilation();
    void testGlShadersTexturedCompilation();
    void testGlShadersWidebandCompilation();
    void testGlShadersWaterfallCompilation();
    void testVaoVboAllocationAndBinding();
    void testWaterfallRendererLifecycle();
    void testPanadapterRendererLifecycle();
    void testOffscreenFboRasterization();
    void testOverlayRendererLifecycle();
    void testOverlayFilterBandwidthRasterization();
    void testOverlayGridRasterization();

private:
    QOpenGLContext *m_context = nullptr;
    QOffscreenSurface *m_surface = nullptr;
    QOpenGLFunctions *m_gl = nullptr;
    bool m_glAvailable = false;
};

void OpenglOffscreenTests::initTestCase()
{
    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setVersion(3, 2);
    format.setProfile(QSurfaceFormat::CompatibilityProfile);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);

    m_context = new QOpenGLContext(this);
    m_context->setFormat(format);
    if (!m_context->create()) {
        // Fallback to default renderable format
        format.setRenderableType(QSurfaceFormat::DefaultRenderableType);
        format.setProfile(QSurfaceFormat::NoProfile);
        m_context->setFormat(format);
        m_context->create();
    }

    m_surface = new QOffscreenSurface(nullptr, this);
    m_surface->setFormat(m_context->format());
    m_surface->create();

    if (!m_context->isValid() || !m_surface->isValid() || !m_context->makeCurrent(m_surface)) {
        m_glAvailable = false;
        qWarning() << "OpenGL offscreen context not available on this platform";
        return;
    }

    m_glAvailable = true;
    m_gl = m_context->functions();
    m_gl->initializeOpenGLFunctions();
}

void OpenglOffscreenTests::cleanupTestCase()
{
    if (m_context && m_context->isValid() && m_surface) {
        m_context->doneCurrent();
    }
}

void OpenglOffscreenTests::testOffscreenContextAndCapabilities()
{
    if (!m_glAvailable)
        QSKIP("OpenGL offscreen context not supported on this platform");

    QVERIFY(m_context != nullptr);
    QVERIFY(m_context->isValid());
    QVERIFY(m_surface != nullptr);
    QVERIFY(m_surface->isValid());
    QCOMPARE(QOpenGLContext::currentContext(), m_context);

    const GLubyte *vendor = m_gl->glGetString(GL_VENDOR);
    const GLubyte *renderer = m_gl->glGetString(GL_RENDERER);
    const GLubyte *version = m_gl->glGetString(GL_VERSION);

    QVERIFY(vendor != nullptr);
    QVERIFY(renderer != nullptr);
    QVERIFY(version != nullptr);

    qDebug() << "Headless OpenGL Vendor:" << reinterpret_cast<const char*>(vendor)
             << "Renderer:" << reinterpret_cast<const char*>(renderer)
             << "Version:" << reinterpret_cast<const char*>(version);
}

void OpenglOffscreenTests::testGlShadersColoredCompilation()
{
    if (!m_glAvailable)
        QSKIP("OpenGL offscreen context not supported on this platform");

    QOpenGLShaderProgram program;
    QVERIFY(program.addShaderFromSourceCode(QOpenGLShader::Vertex, GlShaders::coloredVertexSource()));
    QVERIFY(program.addShaderFromSourceCode(QOpenGLShader::Fragment, GlShaders::coloredFragmentSource()));

    program.bindAttributeLocation("position", 0);
    program.bindAttributeLocation("color", 1);
    QVERIFY2(program.link(), qPrintable(program.log()));
    QVERIFY(program.isLinked());

    int matrixLoc = program.uniformLocation("matrix");
    QVERIFY2(matrixLoc >= 0, "Uniform 'matrix' not found in colored shader");
}

void OpenglOffscreenTests::testGlShadersTexturedCompilation()
{
    if (!m_glAvailable)
        QSKIP("OpenGL offscreen context not supported on this platform");

    QOpenGLShaderProgram program;
    QVERIFY(program.addShaderFromSourceCode(QOpenGLShader::Vertex, GlShaders::texturedVertexSource()));
    const QString fragSrc = GlShaders::texturedFragmentSource("u_texture");
    QVERIFY(program.addShaderFromSourceCode(QOpenGLShader::Fragment, fragSrc));

    program.bindAttributeLocation("position", 0);
    program.bindAttributeLocation("texCoord", 1);
    QVERIFY2(program.link(), qPrintable(program.log()));
    QVERIFY(program.isLinked());

    int matrixLoc = program.uniformLocation("matrix");
    int texLoc = program.uniformLocation("u_texture");
    int colorLoc = program.uniformLocation("textColor");

    QVERIFY2(matrixLoc >= 0, "Uniform 'matrix' not found in textured shader");
    QVERIFY2(texLoc >= 0, "Uniform 'u_texture' not found in textured shader");
    QVERIFY2(colorLoc >= 0, "Uniform 'textColor' not found in textured shader");
}

void OpenglOffscreenTests::testGlShadersWidebandCompilation()
{
    if (!m_glAvailable)
        QSKIP("OpenGL offscreen context not supported on this platform");

    QOpenGLShaderProgram program;
    QVERIFY(program.addShaderFromSourceCode(QOpenGLShader::Vertex, GlShaders::widebandVertexSource()));
    QVERIFY(program.addShaderFromSourceCode(QOpenGLShader::Fragment, GlShaders::widebandFragmentSource()));

    program.bindAttributeLocation("a_pos", 0);
    program.bindAttributeLocation("a_color", 1);
    QVERIFY2(program.link(), qPrintable(program.log()));
    QVERIFY(program.isLinked());

    int mvpLoc = program.uniformLocation("u_mvp");
    QVERIFY2(mvpLoc >= 0, "Uniform 'u_mvp' not found in wideband shader");
}

void OpenglOffscreenTests::testGlShadersWaterfallCompilation()
{
    if (!m_glAvailable)
        QSKIP("OpenGL offscreen context not supported on this platform");

    QOpenGLShaderProgram program;
    QVERIFY(program.addShaderFromSourceCode(QOpenGLShader::Vertex, GlShaders::texturedVertexSource()));
    const QString fragSrc = GlShaders::waterfallFragmentSource("waterfallTexture", "paletteLUT");
    QVERIFY(program.addShaderFromSourceCode(QOpenGLShader::Fragment, fragSrc));

    program.bindAttributeLocation("position", 0);
    program.bindAttributeLocation("texCoord", 1);
    QVERIFY2(program.link(), qPrintable(program.log()));
    QVERIFY(program.isLinked());

    QVERIFY(program.uniformLocation("waterfallTexture") >= 0);
    QVERIFY(program.uniformLocation("paletteLUT") >= 0);
    QVERIFY(program.uniformLocation("lowerThreshold") >= 0);
    QVERIFY(program.uniformLocation("upperThreshold") >= 0);
    QVERIFY(program.uniformLocation("colorRange") >= 0);
    QVERIFY(program.uniformLocation("paletteMode") >= 0);
    QVERIFY(program.uniformLocation("alpha") >= 0);
}

void OpenglOffscreenTests::testVaoVboAllocationAndBinding()
{
    if (!m_glAvailable)
        QSKIP("OpenGL offscreen context not supported on this platform");

    QOpenGLVertexArrayObject vao;
    QVERIFY(vao.create());
    vao.bind();

    QOpenGLBuffer vbo(QOpenGLBuffer::VertexBuffer);
    QVERIFY(vbo.create());
    vbo.bind();
    vbo.setUsagePattern(QOpenGLBuffer::StreamDraw);

    struct Vertex {
        float x, y, z;
        float u, v;
    };
    Vertex quad[4] = {
        {-1.0f, -1.0f, 0.0f, 0.0f, 0.0f},
        { 1.0f, -1.0f, 0.0f, 1.0f, 0.0f},
        {-1.0f,  1.0f, 0.0f, 0.0f, 1.0f},
        { 1.0f,  1.0f, 0.0f, 1.0f, 1.0f}
    };
    vbo.allocate(quad, sizeof(quad));
    QCOMPARE(vbo.size(), static_cast<int>(sizeof(quad)));

    vbo.release();
    vao.release();

    vbo.destroy();
    vao.destroy();
}

void OpenglOffscreenTests::testWaterfallRendererLifecycle()
{
    if (!m_glAvailable)
        QSKIP("OpenGL offscreen context not supported on this platform");

    WaterfallRenderer waterfall;
    waterfall.initialize();

    // Generate 512 mock bins ranging from -140.0 dBm to -30.0 dBm
    constexpr int nBins = 512;
    QVarLengthArray<float> row(nBins);
    for (int i = 0; i < nBins; ++i) {
        row[i] = -140.0f + (110.0f * i / nBins);
    }

    QMatrix4x4 proj;
    proj.ortho(0, 512, 256, 0, -1.0f, 1.0f);
    QRect rect(0, 0, 512, 256);

    WaterfallMapping mapping;
    mapping.lowerThreshold = -140.0f;
    mapping.upperThreshold = -30.0f;
    mapping.colorRange = 110.0f;
    mapping.mode = Simple;

    // Render multiple consecutive rows with newLine=true (cycles PBO ping-pong streaming)
    for (int r = 0; r < 8; ++r) {
        waterfall.render(proj, rect, row, QSDR::DataEngineUp, 1.0f, true, mapping);
        GLenum err = m_gl->glGetError();
        QCOMPARE(err, static_cast<GLenum>(GL_NO_ERROR));
    }

    // Switch color mode to Enhanced (PowerSDR palette)
    mapping.mode = Enhanced;
    for (int r = 0; r < 4; ++r) {
        waterfall.render(proj, rect, row, QSDR::DataEngineUp, 1.0f, true, mapping);
        GLenum err = m_gl->glGetError();
        QCOMPARE(err, static_cast<GLenum>(GL_NO_ERROR));
    }

    // Test reset
    waterfall.reset();
    waterfall.render(proj, rect, row, QSDR::DataEngineUp, 1.0f, true, mapping);
    QCOMPARE(m_gl->glGetError(), static_cast<GLenum>(GL_NO_ERROR));
}

void OpenglOffscreenTests::testPanadapterRendererLifecycle()
{
    if (!m_glAvailable)
        QSKIP("OpenGL offscreen context not supported on this platform");

    PanadapterRenderer pan;
    bool initOk = pan.initialize(m_context);
    QVERIFY(initOk);

    QMatrix4x4 proj;
    proj.ortho(0, 512, 256, 0, -10.0f, 10.0f);
    QRect panRect(0, 0, 512, 256);

    PanadapterRenderer::Colors colors{};
    colors.r = 0.0f; colors.g = 1.0f; colors.b = 0.0f;       // Green line
    colors.rf = 0.0f; colors.gf = 0.5f; colors.bf = 0.0f;    // Filled
    colors.bkgR = 0.05f; colors.bkgG = 0.05f; colors.bkgB = 0.1f;

    // Test renderIdleBackground
    pan.renderIdleBackground(m_gl, proj, panRect, 1.0f, 256, colors, QSDR::DataEngineDown, true);
    QCOMPARE(m_gl->glGetError(), static_cast<GLenum>(GL_NO_ERROR));

    // Generate 1024 spectrum bins (dB above dBmMin, positive values) with noise and a CW peak at bin 512
    constexpr int nBins = 1024;
    QVector<qreal> bins(nBins, 25.0);
    for (int i = 0; i < nBins; ++i) {
        double dist = std::abs(i - 512);
        if (dist < 10) {
            bins[i] = 95.0 - dist * 4.0;
        }
    }
    QVector<qreal> peakHold = bins;

    // Render Lines mode
    pan.render(m_gl, proj, panRect, bins, -30.0, -140.0, Line,
               1.0f, 1.0f, 256, colors, QSDR::DataEngineUp, true, peakHold, false);
    QCOMPARE(m_gl->glGetError(), static_cast<GLenum>(GL_NO_ERROR));

    // Render Filled mode
    pan.render(m_gl, proj, panRect, bins, -30.0, -140.0, FilledLine,
               1.0f, 1.0f, 256, colors, QSDR::DataEngineUp, true, peakHold, false);
    QCOMPARE(m_gl->glGetError(), static_cast<GLenum>(GL_NO_ERROR));

    pan.release();
}

void OpenglOffscreenTests::testOffscreenFboRasterization()
{
    if (!m_glAvailable)
        QSKIP("OpenGL offscreen context not supported on this platform");

    constexpr int fboWidth = 256;
    constexpr int fboHeight = 128;

    QOpenGLFramebufferObjectFormat fboFormat;
    fboFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    QOpenGLFramebufferObject fbo(fboWidth, fboHeight, fboFormat);
    QVERIFY(fbo.isValid());

    QVERIFY(fbo.bind());
    m_gl->glViewport(0, 0, fboWidth, fboHeight);
    m_gl->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    m_gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    PanadapterRenderer pan;
    QVERIFY(pan.initialize(m_context));

    QMatrix4x4 proj;
    proj.ortho(0, fboWidth, fboHeight, 0, -10.0f, 10.0f);
    QRect panRect(0, 0, fboWidth, fboHeight);

    PanadapterRenderer::Colors colors{};
    colors.r = 1.0f; colors.g = 1.0f; colors.b = 0.0f;       // Bright yellow line
    colors.rf = 0.8f; colors.gf = 0.6f; colors.bf = 0.0f;    // Amber fill
    colors.bkgR = 0.0f; colors.bkgG = 0.0f; colors.bkgB = 0.0f;

    // Bins are dB above dBmMin (positive values)
    QVector<qreal> bins(fboWidth, 20.0);
    // Add strong carrier peak in center (80 dB above min)
    for (int i = fboWidth / 2 - 10; i <= fboWidth / 2 + 10; ++i) {
        bins[i] = 80.0;
    }

    pan.render(m_gl, proj, panRect, bins, -30.0, -140.0, FilledLine,
               1.0f, 1.0f, fboHeight, colors, QSDR::DataEngineUp, true);

    QCOMPARE(m_gl->glGetError(), static_cast<GLenum>(GL_NO_ERROR));
    pan.release();

    QVERIFY(fbo.release());

    // Convert FBO to QImage and verify pixels
    QImage img = fbo.toImage();
    QCOMPARE(img.width(), fboWidth);
    QCOMPARE(img.height(), fboHeight);

    // Count non-black pixels rendered into the FBO
    int nonBlackPixels = 0;
    for (int y = 0; y < fboHeight; ++y) {
        const QRgb *scanline = reinterpret_cast<const QRgb*>(img.constScanLine(y));
        for (int x = 0; x < fboWidth; ++x) {
            QRgb pixel = scanline[x];
            if (qRed(pixel) > 20 || qGreen(pixel) > 20 || qBlue(pixel) > 20) {
                ++nonBlackPixels;
            }
        }
    }

    qDebug() << "Offscreen FBO non-black rasterized pixels:" << nonBlackPixels;
    QVERIFY2(nonBlackPixels > 0, "FBO spectrum rendering produced 0 rasterized pixels");
}

void OpenglOffscreenTests::testOverlayRendererLifecycle()
{
    if (!m_glAvailable)
        QSKIP("OpenGL offscreen context not supported on this platform");

    OverlayRenderer overlay;
    overlay.initialize();
    QCOMPARE(m_gl->glGetError(), static_cast<GLenum>(GL_NO_ERROR));

    QMatrix4x4 proj;
    proj.ortho(0, 512, 256, 0, -10.0f, 10.0f);
    QRect panRect(0, 0, 512, 128);
    QRect dBmScaleRect(0, 0, 45, 128);

    // Test AGC control geometry calculation and drawing
    float threshPx = 0.0f, hangPx = 0.0f, fixedPx = 0.0f;
    overlay.drawAGCControl(proj, panRect, dBmScaleRect, agcMED, true,
                           -100.0f, -80.0f, 20.0f, -30.0, -140.0,
                           1.0f, 256, threshPx, hangPx, fixedPx);
    QCOMPARE(m_gl->glGetError(), static_cast<GLenum>(GL_NO_ERROR));
    QVERIFY(threshPx > 0.0f);

    // Test Crosshair rendering
    overlay.drawCrossHair(proj, panRect, dBmScaleRect, QPoint(256, 64), 1.0f, 256);
    QCOMPARE(m_gl->glGetError(), static_cast<GLenum>(GL_NO_ERROR));
}

void OpenglOffscreenTests::testOverlayFilterBandwidthRasterization()
{
    if (!m_glAvailable)
        QSKIP("OpenGL offscreen context not supported on this platform");

    constexpr int fboWidth = 512;
    constexpr int fboHeight = 256;

    QOpenGLFramebufferObjectFormat fboFormat;
    fboFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    QOpenGLFramebufferObject fbo(fboWidth, fboHeight, fboFormat);
    QVERIFY(fbo.isValid());

    QVERIFY(fbo.bind());
    m_gl->glViewport(0, 0, fboWidth, fboHeight);
    m_gl->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    m_gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    OverlayRenderer overlay;
    overlay.initialize();

    QMatrix4x4 proj;
    proj.ortho(0, fboWidth, fboHeight, 0, -10.0f, 10.0f);
    QRect panRect(0, 0, fboWidth, fboHeight / 2);
    QRect waterfallRect(0, fboHeight / 2, fboWidth, fboHeight / 2);
    QRect freqScaleRect(0, fboHeight / 2 - 20, fboWidth, 20);

    // Filter passband from -0.1 to +0.1 relative to center
    int fLeft = 0, fRight = 0, fTop = 0, fBottom = 0;
    QColor filterColor(255, 180, 0, 160);
    overlay.drawFilter(proj, panRect, waterfallRect, -0.1f, 0.1f, 0.0f, 1.0f,
                       filterColor, false, false, true, true,
                       fLeft, fRight, fTop, fBottom);
    QCOMPARE(m_gl->glGetError(), static_cast<GLenum>(GL_NO_ERROR));
    QVERIFY(fRight > fLeft);

    // Center tuning line
    overlay.drawCenterLine(proj, panRect, freqScaleRect, waterfallRect,
                           panRect.bottom(), 0.0f, 1.0f,
                           QColor(255, 255, 255), QColor(255, 0, 0), false, false);
    QCOMPARE(m_gl->glGetError(), static_cast<GLenum>(GL_NO_ERROR));

    QVERIFY(fbo.release());

    // Verify non-black rasterized pixels in FBO
    QImage img = fbo.toImage();
    int nonBlackPixels = 0;
    for (int y = 0; y < fboHeight; ++y) {
        const QRgb *scanline = reinterpret_cast<const QRgb*>(img.constScanLine(y));
        for (int x = 0; x < fboWidth; ++x) {
            QRgb pixel = scanline[x];
            if (qRed(pixel) > 10 || qGreen(pixel) > 10 || qBlue(pixel) > 10) {
                ++nonBlackPixels;
            }
        }
    }

    qDebug() << "Offscreen FBO filter passband non-black pixels:" << nonBlackPixels;
    QVERIFY2(nonBlackPixels > 0, "OverlayRenderer drawFilter produced 0 rasterized pixels");
}

void OpenglOffscreenTests::testOverlayGridRasterization()
{
    if (!m_glAvailable)
        QSKIP("OpenGL offscreen context not supported on this platform");

    constexpr int fboWidth = 512;
    constexpr int fboHeight = 256;

    QOpenGLFramebufferObjectFormat fboFormat;
    fboFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    QOpenGLFramebufferObject fbo(fboWidth, fboHeight, fboFormat);
    QVERIFY(fbo.isValid());

    QVERIFY(fbo.bind());
    m_gl->glViewport(0, 0, fboWidth, fboHeight);
    m_gl->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    m_gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    OverlayRenderer overlay;
    overlay.initialize();

    QMatrix4x4 proj;
    proj.ortho(0, fboWidth, fboHeight, 0, -10.0f, 10.0f);
    QRect panRect(0, 0, fboWidth, fboHeight);
    QRect freqScaleRect(0, fboHeight - 20, fboWidth, 20);

    TScale freqScale{};
    freqScale.mainPointPositions << 50 << 100 << 150 << 200 << 250 << 300;

    TScale dBmScale{};
    dBmScale.mainPointPositions << 30 << 60 << 90 << 120 << 150 << 180;

    overlay.drawGrid(proj, panRect, freqScaleRect, freqScale, dBmScale,
                     0, 0.5f, 0.5f, 0.5f, 0.5f, true);
    QCOMPARE(m_gl->glGetError(), static_cast<GLenum>(GL_NO_ERROR));

    QVERIFY(fbo.release());

    // Verify non-black rasterized grid lines in FBO
    QImage img = fbo.toImage();
    int nonBlackPixels = 0;
    for (int y = 0; y < fboHeight; ++y) {
        const QRgb *scanline = reinterpret_cast<const QRgb*>(img.constScanLine(y));
        for (int x = 0; x < fboWidth; ++x) {
            QRgb pixel = scanline[x];
            if (qRed(pixel) > 10 || qGreen(pixel) > 10 || qBlue(pixel) > 10) {
                ++nonBlackPixels;
            }
        }
    }

    qDebug() << "Offscreen FBO overlay grid non-black pixels:" << nonBlackPixels;
    QVERIFY2(nonBlackPixels > 0, "OverlayRenderer drawGrid produced 0 rasterized pixels");
}

QTEST_MAIN(OpenglOffscreenTests)
#include "opengl_offscreen_tests.moc"
