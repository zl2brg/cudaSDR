/**
 * @file  SMeterRenderer.cpp
 * @brief Renderer for the analog S-Meter arc, needle, labels, and FBO caching in OGLDisplayPanel.
 * @author Simon Eatough <simon.eatough@gmail.com>
 * @date 2026-09-08
 */

#include "SMeterRenderer.h"
#include "cusdr_oglDisplayPanel.h"
#include "cusdr_glDraw.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

SMeterRenderer::SMeterRenderer(OGLDisplayPanel *panel)
    : m_panel(panel)
{
}

SMeterRenderer::~SMeterRenderer()
{
    invalidateFBO();
}

void SMeterRenderer::invalidateFBO()
{
    if (m_smeterFBO) {
        delete m_smeterFBO;
        m_smeterFBO = nullptr;
    }
}

void SMeterRenderer::drawSMeterNeedle(const QMatrix4x4 &projection, int x1)
{
    if (!m_panel || m_panel->m_sMeterValue <= 0 || !m_panel->m_shaderProgram || !m_panel->m_shaderProgram->isLinked())
        return;

    m_panel->m_vao.bind();

    // Main signal needle (bright white)
    const float x = float(x1 + int(m_panel->m_sMeterValue * m_panel->m_unit));
    const GlDraw::Vec3Rgb needle[2] = {
        { x, float(m_panel->m_sMeterPosY) - 15.0f, 1.0f, 1.0f, 1.0f, 1.0f },
        { x, float(m_panel->m_sMeterPosY) + 26.0f, 1.0f, 1.0f, 1.0f, 1.0f },
    };

    m_panel->glLineWidth(2.0f);
    GlDraw::drawColoredLines(m_panel, m_panel->m_shaderProgram, m_panel->m_vbo, projection, needle, 2);

    // Peak hold needle (amber/red pip at top of scale)
    if (m_panel->m_sMeterMaxValueB > m_panel->m_sMeterValue + 0.5f) {
        const float xPeak = float(x1 + int(m_panel->m_sMeterMaxValueB * m_panel->m_unit));
        const GlDraw::Vec3Rgb peakNeedle[2] = {
            { xPeak, float(m_panel->m_sMeterPosY) - 15.0f, 1.0f, 0.4f, 0.4f, 1.0f },
            { xPeak, float(m_panel->m_sMeterPosY) + 12.0f, 1.0f, 0.4f, 0.4f, 1.0f },
        };
        GlDraw::drawColoredLines(m_panel, m_panel->m_shaderProgram, m_panel->m_vbo, projection, peakNeedle, 2);
    }
}

void SMeterRenderer::paintSMeter() {
    if (!m_panel) return;

    GLint width = m_panel->m_smeterRect.width();
    GLint height = m_panel->m_smeterRect.height();
    GLint x1 = m_panel->m_smeterRect.left();
    GLint y1 = m_panel->m_smeterRect.top();
    GLint y2 = y1 + height;

    m_panel->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_panel->glEnable(GL_BLEND);

    // Only recreate FBO if needed
    if (!m_smeterFBO || m_panel->m_smeterRenew) {
        if (m_smeterFBO) {
            delete m_smeterFBO;
            m_smeterFBO = nullptr;
        }
        m_smeterFBO = new QOpenGLFramebufferObject(m_panel->m_sMeterWidth, height);
        m_panel->m_smeterUpdate = true; // Need to re-render after FBO recreation
        m_panel->m_smeterRenew = false;
    }

    // Only re-render scale if needed
    if (m_panel->m_smeterUpdate) {
        m_smeterFBO->bind();
        renderSMeterScale();
        m_smeterFBO->release();
        QOpenGLFramebufferObject::bindDefault();
        m_panel->m_smeterUpdate = false;
    }

    const QMatrix4x4 projection = m_panel->panelProjection();
    const int smeterX = m_panel->m_rxRect.right() + m_panel->m_sMeterOffset;

    m_panel->glDisable(GL_DEPTH_TEST);

    const QRect texRect(smeterX, 0, m_panel->m_sMeterWidth, height);
    if (m_panel->m_textureProgram && m_panel->m_textureProgram->isLinked()) {
        m_panel->m_vao.bind();
        GlDraw::renderTexturedQuad(m_panel, m_panel->m_textureProgram, m_panel->m_vbo, projection,
                                   texRect, m_smeterFBO->texture(), -2.0f);
    }

    drawSMeterScaleLabels(projection, smeterX);

    m_panel->glScissor(int(x1 * m_panel->dpr), int((m_panel->size().height() - y2) * m_panel->dpr),
                       int(width * m_panel->dpr), int(height * m_panel->dpr));
    m_panel->glEnable(GL_SCISSOR_TEST);

    if (m_panel->m_dataEngineState == QSDR::DataEngineUp) {
        // Text rendering above releases its VAO; Core 3.3 requires a bound VAO
        m_panel->m_vao.bind();
        m_panel->glDisable(GL_DEPTH_TEST);

        // Signal level bar filled from baseline (left) to current S-meter value
        const int barWidth = int(m_panel->m_sMeterValue * m_panel->m_unit);
        if (barWidth > 0 && m_panel->m_shaderProgram && m_panel->m_shaderProgram->isLinked()) {
            const QRect bar(x1, m_panel->m_sMeterPosY + 3, barWidth, 6);
            const QColor cLeft(40, 180, 100);
            const QColor cRight = (m_panel->m_sMeterValue > 97.0f) ? QColor(255, 50, 50) :
                                  (m_panel->m_sMeterValue > 67.0f) ? QColor(255, 200, 50) : QColor(56, 242, 115);
            GlDraw::drawGradientRect(m_panel, m_panel->m_shaderProgram, m_panel->m_vbo, projection, bar,
                                     cLeft, cRight, true, 1.0f);
        }

        drawSMeterNeedle(projection, x1);

        m_panel->qglColor(m_panel->m_activeTextColor);
        m_panel->m_sMeterNumValueString = QString::number(m_panel->m_sMeterOrgValue, 'f', 1);

        // Calculate standard S-Unit display string
        QString sUnitStr;
        QColor sUnitColor;
        if (m_panel->m_sMeterOrgValue >= -73.0) {
            const int over = qRound(m_panel->m_sMeterOrgValue - (-73.0));
            sUnitStr = (over > 0) ? QStringLiteral("S9+%1").arg(over) : QStringLiteral("S9");
            sUnitColor = (over >= 40) ? QColor(255, 60, 60) :
                         (over >= 10) ? QColor(255, 200, 50) : QColor(255, 255, 255);
        } else {
            const int s = qBound(0, static_cast<int>(9.0 + (m_panel->m_sMeterOrgValue - (-73.0)) / 6.0 + 0.5), 9);
            sUnitStr = QStringLiteral("S%1").arg(s);
            sUnitColor = QColor(56, 242, 115);
        }

        const QString rxBadge = QStringLiteral("RX%1").arg(m_panel->m_currentReceiver + 1);
        m_panel->m_oglTextSmall->renderText(projection, x1 + m_panel->m_sMeterWidth - 195, 8, rxBadge, m_panel->m_activeTextColor);
        m_panel->m_oglTextBig->renderText(projection, x1 + m_panel->m_sMeterWidth - 148, 2, sUnitStr, sUnitColor);
        m_panel->m_oglTextBig->renderText(projection, x1 + m_panel->m_sMeterWidth - 85, 2, m_panel->m_sMeterNumValueString, Qt::white);
        m_panel->m_oglTextNormal->renderText(projection, x1 + m_panel->m_sMeterWidth - 28, 9, QStringLiteral("dBm"), m_panel->m_activeTextColor);
    }

    m_panel->glDisable(GL_SCISSOR_TEST);
    m_panel->glEnable(GL_DEPTH_TEST);
    m_panel->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_panel->glEnable(GL_BLEND);
}

void SMeterRenderer::renderSMeterScale() {
    if (!m_panel) return;

    m_panel->m_vao.bind();
    const GLint width = m_panel->m_sMeterWidth;
    const GLint height = m_panel->m_smeterRect.height();

    const qreal dBmRange = qAbs(m_panel->m_dBmPanMax - m_panel->m_dBmPanMin);
    m_panel->m_unit = (dBmRange > 0) ? qreal(m_panel->m_sMeterWidth / dBmRange) : 0;

    GLint savedViewport[4] = { 0, 0, 0, 0 };
    m_panel->glGetIntegerv(GL_VIEWPORT, savedViewport);

    const int fboW = m_smeterFBO ? m_smeterFBO->width() : width;
    const int fboH = m_smeterFBO ? m_smeterFBO->height() : height;
    if (m_smeterFBO)
        m_panel->glViewport(0, 0, fboW, fboH);

    const QRect rect(0, 0, fboW, fboH);

    QMatrix4x4 projection;
    projection.ortho(0, fboW, fboH, 0, -10, 10);

    m_panel->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    m_panel->glClear(GL_COLOR_BUFFER_BIT);

    m_panel->glDisable(GL_MULTISAMPLE);
    m_panel->glDisable(GL_LINE_SMOOTH);
    m_panel->glLineWidth(1.0f);

    if (m_panel->m_shaderProgram && m_panel->m_shaderProgram->isLinked()) {
        if (m_panel->m_dataEngineState == QSDR::DataEngineUp)
            GlDraw::drawGradientRect(m_panel, m_panel->m_shaderProgram, m_panel->m_vbo, projection, rect,
                                     Qt::black, m_panel->m_bkgColor2, false, -3.0f);
        else
            GlDraw::drawSolidRect(m_panel, m_panel->m_shaderProgram, m_panel->m_vbo, projection, rect, Qt::black, -3.0f);

        QColor col = m_panel->m_activeTextColor;
        const float r = col.redF(), g = col.greenF(), b = col.blueF();
        const float posY = float(m_panel->m_sMeterPosY);

        QVector<GlDraw::Vec3Rgb> scaleLines;
        scaleLines.reserve(4 + m_panel->m_sMeterWidth * 2);
        scaleLines.append({ 0.0f, posY, 0.0f, r, g, b });
        scaleLines.append({ float(width - 1), posY, 0.0f, r, g, b });
        scaleLines.append({ 0.0f, posY + 12.0f, 0.0f, r, g, b });
        scaleLines.append({ float(width - 1), posY + 12.0f, 0.0f, r, g, b });

        const QColor stepCol = (m_panel->m_dataEngineState == QSDR::DataEngineUp)
                                   ? QColor(126, 156, 168)
                                   : m_panel->m_activeTextColor;
        const float sr = stepCol.redF(), sg = stepCol.greenF(), sb = stepCol.blueF();

        int vertexArrayLength = m_panel->m_sMeterWidth;
        vertexArrayLength += vertexArrayLength % 2;
        for (int i = 0; i < vertexArrayLength; ++i) {
            scaleLines.append({ 2.0f * float(i), posY + 4.0f, 0.0f, sr, sg, sb });
            scaleLines.append({ 2.0f * float(i), posY + 9.0f, 0.0f, sr, sg, sb });
        }

        GlDraw::drawColoredLines(m_panel, m_panel->m_shaderProgram, m_panel->m_vbo, projection,
                                 scaleLines.constData(), scaleLines.size());

        const QColor tickCol = (m_panel->m_dataEngineState == QSDR::DataEngineUp) ? Qt::white : m_panel->m_inactiveTextColor;
        const float tr = tickCol.redF(), tg = tickCol.greenF(), tb = tickCol.blueF();

        QVector<GlDraw::Vec3Rgb> tickLines;
        tickLines.reserve(64);
        for (int z = -130; z <= 0; z += 10) {
            const float xMajor = float((z - (-140)) * m_panel->m_unit);
            const float xMinor = float((z - (-140) - 5) * m_panel->m_unit);
            tickLines.append({ xMajor, posY - 4.0f, 0.0f, tr, tg, tb });
            tickLines.append({ xMajor, posY, 0.0f, tr, tg, tb });
            if (z > -130) {
                tickLines.append({ xMinor, posY - 2.0f, 0.0f, tr, tg, tb });
                tickLines.append({ xMinor, posY, 0.0f, tr, tg, tb });
            }
        }
        GlDraw::drawColoredLines(m_panel, m_panel->m_shaderProgram, m_panel->m_vbo, projection,
                                 tickLines.constData(), tickLines.size());

        struct SMark {
            int dbFromBase;
            int colorZone;
            bool isMajor;
        };
        static const SMark sMarks[] = {
            { 19, 0, true },   // S1
            { 25, 0, false },  // S2
            { 31, 0, true },   // S3
            { 37, 0, false },  // S4
            { 43, 0, true },   // S5
            { 49, 0, false },  // S6
            { 55, 0, true },   // S7
            { 61, 0, false },  // S8
            { 67, 0, true },   // S9
            { 77, 1, false },  // +10
            { 87, 1, true },   // +20
            { 97, 1, false },  // +30
            { 107, 2, true },  // +40
            { 117, 2, false }, // +50
            { 127, 2, true }   // +60
        };

        QVector<GlDraw::Vec3Rgb> sUnitLines;
        sUnitLines.reserve(48);
        auto appendLine = [&](float x, float y1, float y2, float lr, float lg, float lb) {
            sUnitLines.append({ x, y1, 0.0f, lr, lg, lb });
            sUnitLines.append({ x, y2, 0.0f, lr, lg, lb });
        };

        for (const auto &mark : sMarks) {
            const float x = float(mark.dbFromBase * m_panel->m_unit);
            float lr, lg, lb;
            if (m_panel->m_dataEngineState == QSDR::DataEngineUp) {
                if (mark.colorZone == 0) {
                    lr = 56.0f / 255.0f; lg = 242.0f / 255.0f; lb = 115.0f / 255.0f;
                } else if (mark.colorZone == 1) {
                    lr = 255.0f / 255.0f; lg = 200.0f / 255.0f; lb = 50.0f / 255.0f;
                } else {
                    lr = 255.0f / 255.0f; lg = 60.0f / 255.0f; lb = 60.0f / 255.0f;
                }
            } else {
                lr = m_panel->m_inactiveTextColor.redF();
                lg = m_panel->m_inactiveTextColor.greenF();
                lb = m_panel->m_inactiveTextColor.blueF();
            }
            const float tickH = mark.isMajor ? 6.0f : 4.0f;
            appendLine(x, posY + 12.0f, posY + 12.0f + tickH, lr, lg, lb);
        }

        // Colored bottom guide rails
        if (m_panel->m_dataEngineState == QSDR::DataEngineUp) {
            // Green line: 0 to S9 (67 dB)
            sUnitLines.append({ 0.0f, posY + 12.0f, 0.0f, 56.0f / 255.0f, 242.0f / 255.0f, 115.0f / 255.0f });
            sUnitLines.append({ float(67 * m_panel->m_unit), posY + 12.0f, 0.0f, 56.0f / 255.0f, 242.0f / 255.0f, 115.0f / 255.0f });
            // Yellow line: S9 to +30 dB (97 dB)
            sUnitLines.append({ float(67 * m_panel->m_unit), posY + 12.0f, 0.0f, 255.0f / 255.0f, 200.0f / 255.0f, 50.0f / 255.0f });
            sUnitLines.append({ float(97 * m_panel->m_unit), posY + 12.0f, 0.0f, 255.0f / 255.0f, 200.0f / 255.0f, 50.0f / 255.0f });
            // Red line: +30 dB to end
            sUnitLines.append({ float(97 * m_panel->m_unit), posY + 12.0f, 0.0f, 255.0f / 255.0f, 60.0f / 255.0f, 60.0f / 255.0f });
            sUnitLines.append({ float(width - 1), posY + 12.0f, 0.0f, 255.0f / 255.0f, 60.0f / 255.0f, 60.0f / 255.0f });
        }

        GlDraw::drawColoredLines(m_panel, m_panel->m_shaderProgram, m_panel->m_vbo, projection,
                                 sUnitLines.constData(), sUnitLines.size());
    } else {
        if (m_panel->m_dataEngineState == QSDR::DataEngineUp)
            GlDraw::drawGradientRect(m_panel, m_panel->m_shaderProgram, m_panel->m_vbo, projection, rect,
                                     Qt::black, m_panel->m_bkgColor2, false, -3.0f);
        else
            GlDraw::drawSolidRect(m_panel, m_panel->m_shaderProgram, m_panel->m_vbo, projection, rect, Qt::black, -3.0f);
    }

    m_panel->glViewport(savedViewport[0], savedViewport[1], savedViewport[2], savedViewport[3]);
}

void SMeterRenderer::drawSMeterScaleLabels(const QMatrix4x4 &projection, int xOffset)
{
    if (!m_panel) return;

    const QFontMetrics fm = m_panel->m_oglTextNormal->fontMetrics();

    // Top dBm labels (-120 to 0 dBm)
    for (int z = -120; z <= 0; z += 20) {
        const int dbFromBase = z - (-140);
        QString marker = QString::number(z);
        const int d = fm.horizontalAdvance(marker);
        const int x = xOffset + int(dbFromBase * m_panel->m_unit) - d / 2;
        m_panel->m_oglTextNormal->renderText(projection, float(x), float(m_panel->m_sMeterPosY - 18), marker, m_panel->m_activeTextColor);
    }

    m_panel->m_oglTextSmallItalic->renderText(projection, float(xOffset + m_panel->m_sMeterWidth - 25),
                                             float(m_panel->m_sMeterPosY - 16), QStringLiteral("dBm"), m_panel->m_activeTextColor);

    struct SLabel {
        int dbFromBase;
        const char *text;
        QColor color;
    };
    static const SLabel sLabels[] = {
        { 19, "S1", QColor(56, 242, 115) },
        { 31, "S3", QColor(56, 242, 115) },
        { 43, "S5", QColor(56, 242, 115) },
        { 55, "S7", QColor(56, 242, 115) },
        { 67, "S9", QColor(255, 255, 255) },
        { 87, "+20", QColor(255, 200, 50) },
        { 107, "+40", QColor(255, 80, 80) },
        { 127, "+60", QColor(255, 80, 80) }
    };

    for (const auto &lbl : sLabels) {
        QString marker = QString::fromLatin1(lbl.text);
        const int d = fm.horizontalAdvance(marker);
        const float x = float(xOffset + int(lbl.dbFromBase * m_panel->m_unit) - d / 2);
        const QColor c = (m_panel->m_dataEngineState == QSDR::DataEngineUp) ? lbl.color : m_panel->m_inactiveTextColor;
        m_panel->m_oglTextNormal->renderText(projection, x, float(m_panel->m_sMeterPosY + 18), marker, c);
    }
}
