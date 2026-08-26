#include "GridRenderer.h"
#include "cusdr_oglReceiverPanel.h"
#include "cusdr_oglUtils.h"

#include <QOpenGLPaintDevice>
#include <QOpenGLFramebufferObject>

GridRenderer::GridRenderer(QGLReceiverPanel *panel)
    : m_panel(panel)
    , m_frequencyScaleFBO(nullptr)
    , m_dBmScaleFBO(nullptr)
    , m_secScaleWaterfallFBO(nullptr)
    , m_glReady(false)
{
}

void GridRenderer::ensureGL()
{
    if (!m_glReady) {
        initializeOpenGLFunctions();
        m_glReady = true;
    }
}

GridRenderer::~GridRenderer()
{
    invalidateScaleFBOs();
}

void GridRenderer::invalidateScaleFBOs()
{
    delete m_frequencyScaleFBO;
    m_frequencyScaleFBO = nullptr;
    delete m_dBmScaleFBO;
    m_dBmScaleFBO = nullptr;
    delete m_secScaleWaterfallFBO;
    m_secScaleWaterfallFBO = nullptr;
}

void GridRenderer::updateFrequencyRuler()
{
    if (!m_panel->m_freqScalePanRect.isValid())
        return;

    const qreal freqSpan = m_panel->displayedFrequencySpanHz();
    if (freqSpan <= 0)
        return;

    const int plotLeft = m_panel->m_dBmScalePanRect.isValid() ? m_panel->m_dBmScalePanRect.width() : 0;
    const int plotWidth = m_panel->m_freqScalePanRect.width() - plotLeft;
    if (plotWidth <= 0)
        return;

    const int fullWidth = m_panel->m_freqScalePanRect.width();

    const qreal lowerFreq = qreal(m_panel->m_centerFrequency) - freqSpan / 2;
    const qreal upperFreq = qreal(m_panel->m_centerFrequency) + freqSpan / 2;

    // Ruler must stay locked to the panadapter centre (hardware LO). Do not
    // offset by m_panel->m_deltaF / VFO-NCO: the spectrum is LO-relative, and shifting
    // the scale with click-VFO made a signal at centre read as centre−Δf
    // (e.g. click +4 kHz → peak labelled centre−4 kHz).
    const qreal plotLo = lowerFreq + qreal(plotLeft) * freqSpan / qreal(fullWidth);
    const qreal plotFreqSpan = upperFreq - plotLo;
    const qreal visibleLo = plotLo;
    const qreal visibleHi = visibleLo + plotFreqSpan;
    const qreal unit = qreal(plotWidth) / plotFreqSpan;
    const int fontMaxWidth = m_panel->m_fonts.smallFontMetrics->boundingRect(QStringLiteral("000.000.0")).width();

    const QRect plotRect(0, 0, plotWidth, m_panel->m_freqScalePanRect.height());
    m_panel->m_frequencyScale = getXRuler(plotRect, fontMaxWidth, unit, visibleLo, visibleHi);
}

void GridRenderer::updateDBmRuler()
{
    if (!m_panel->m_dBmScalePanRect.isValid())
        return;

    const qreal dBmRange = qAbs(m_panel->m_dBmPanMax - m_panel->m_dBmPanMin);
    if (dBmRange <= 0)
        return;

    int spacing = 7;
    int fontHeight;
    if (m_panel->m_smallSize)
        fontHeight = m_panel->m_fonts.smallFontMetrics->tightBoundingRect(QStringLiteral(".0dBm")).height() + spacing;
    else
        fontHeight = m_panel->m_fonts.bigFont1Metrics->tightBoundingRect(QStringLiteral(".0dBm")).height() + spacing;

    const qreal unit = qreal(m_panel->m_dBmScalePanRect.height()) / dBmRange;
    m_panel->m_dBmScale = getYRuler2(m_panel->m_dBmScalePanRect, fontHeight, unit, m_panel->m_dBmPanMin, m_panel->m_dBmPanMax);
}

void GridRenderer::drawPanVerticalScale() {
    ensureGL();

    if (!m_panel->m_dBmScalePanRect.isValid()) return;

    // Use logical dimensions for consistent rendering
    int width = m_panel->m_dBmScalePanRect.width();
    int height = m_panel->m_dBmScalePanRect.height();
    
    // Safety check for valid dimensions
    if (width <= 0 || height <= 0) return;

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    const bool regenDBmScale = !m_dBmScaleFBO || m_panel->m_dBmScalePanadapterRenew
                               || (m_panel->m_dBmScalePanadapterUpdate && !m_panel->m_dragDBmScale);

    const qreal dpr = m_panel->devicePixelRatioF();
    const int devWidth = qMax(1, int(qRound(width * dpr)));
    const int devHeight = qMax(1, int(qRound(height * dpr)));

    if (regenDBmScale) {
        if (!m_dBmScaleFBO || m_dBmScaleFBO->size() != QSize(devWidth, devHeight) || m_panel->m_dBmScalePanadapterRenew) {
            if (m_dBmScaleFBO) {
                delete m_dBmScaleFBO;
                m_dBmScaleFBO = nullptr;
            }
            m_dBmScaleFBO = new QOpenGLFramebufferObject(devWidth, devHeight);
        }
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        glViewport(0, 0, devWidth, devHeight);

        m_dBmScaleFBO->bind();
        renderPanVerticalScale();
        m_dBmScaleFBO->release();
        QOpenGLFramebufferObject::bindDefault();

        if (!m_panel->m_dragDBmScale)
            m_panel->m_dBmScalePanadapterUpdate = false;
        m_panel->m_dBmScalePanadapterRenew = false;
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    }

    m_panel->drawCachedTexture(m_panel->m_dBmScalePanRect, m_dBmScaleFBO->texture(), 0.0f);

    if (m_panel->m_dragDBmScale && m_panel->m_overlayRenderer && m_dBmScaleFBO) {
        QMatrix4x4 projection;
        projection.ortho(0, m_panel->size().width(), m_panel->size().height(), 0, -10, 10);
        const float alpha = (m_panel->m_receiver == m_panel->m_currentReceiver) ? 1.0f : 0.8f;
        m_panel->m_overlayRenderer->drawDBmScaleTicks(projection, m_panel->m_dBmScalePanRect, m_panel->m_dBmScale,
                                            m_panel->m_redGrid, m_panel->m_greenGrid, m_panel->m_blueGrid, alpha);
    }
}

void GridRenderer::renderPanVerticalScale() {
    ensureGL();
    if (!m_dBmScaleFBO)
        return;

    const qreal dpr = m_panel->devicePixelRatioF();
    QOpenGLPaintDevice paintDevice(m_dBmScaleFBO->size());
    paintDevice.setDevicePixelRatio(dpr);

    painter.begin(&paintDevice);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const int width = m_panel->m_dBmScalePanRect.width();
    const int height = m_panel->m_dBmScalePanRect.height();

    // Background panel
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(0, 0, width, height, QColor(14, 18, 24, 210));
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    // Right border
    painter.setPen(QPen(QColor(45, 60, 75), 1.0));
    painter.drawLine(width - 1, 0, width - 1, height);

    const int len = m_panel->m_dBmScale.mainPointPositions.length();
    const int sublen = m_panel->m_dBmScale.subPointPositions.length();

    // Minor ticks
    if (sublen > 0) {
        painter.setPen(QPen(QColor(90, 125, 145), 1.0));
        for (int i = 0; i < sublen; i++) {
            const int y = m_panel->m_dBmScale.subPointPositions.at(i);
            painter.drawLine(width - 1, y, width - 3, y);
        }
    }

    // Major ticks
    if (len > 0) {
        painter.setPen(QPen(QColor(160, 195, 215), 1.0));
        for (int i = 0; i < len; i++) {
            const int y = m_panel->m_dBmScale.mainPointPositions.at(i);
            painter.drawLine(width - 1, y, width - 6, y);
        }
    }

    // Numbers
    QFont font = m_panel->m_smallSize ? m_panel->m_fonts.smallFont : m_panel->m_fonts.bigFont1;
    painter.setFont(font);
    QFontMetrics fm(font);
    const int fontHeight = fm.height();

    painter.setPen(QPen(QColor(200, 225, 240)));

    if (len > 0) {
        for (int i = 0; i < len; i++) {
            const int y = m_panel->m_dBmScale.mainPointPositions.at(i);
            const qreal val = m_panel->m_dBmScale.mainPoints.at(i);
            const QString str = (qAbs(val - qRound(val)) < 0.05) ? QString::number(qRound(val)) : QString::number(val, 'f', 1);

            const QRectF textRect(2, y - fontHeight / 2.0, width - 9, fontHeight);
            if (textRect.top() >= 2 && textRect.bottom() <= (height - 18)) {
                painter.drawText(textRect, Qt::AlignRight | Qt::AlignVCenter, str);
            }
        }
    }

    // Unit label "dBm" at bottom
    const QRectF unitRect(2, height - 16, width - 4, 14);
    painter.setFont(m_panel->m_fonts.smallFont);
    painter.setPen(QPen(QColor(255, 90, 130)));
    painter.drawText(unitRect, Qt::AlignCenter, QStringLiteral("dBm"));

    painter.end();
}

void GridRenderer::drawPanHorizontalScale() {
    ensureGL();

	if (!m_panel->m_freqScalePanRect.isValid()) return;

    int width = m_panel->m_freqScalePanRect.width();
    int height = m_panel->m_freqScalePanRect.height();
    
    // Safety check for valid dimensions
    if (width <= 0 || height <= 0) return;

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

    const qreal dpr = m_panel->devicePixelRatioF();
    const int devWidth = qMax(1, int(qRound(width * dpr)));
    const int devHeight = qMax(1, int(qRound(height * dpr)));

    const bool regenFreqScale = !m_frequencyScaleFBO || m_frequencyScaleFBO->size() != QSize(devWidth, devHeight)
                                || m_panel->m_freqScalePanadapterRenew
                                || m_panel->m_freqScalePanadapterUpdate
                                || m_panel->m_dragMouse
                                || (m_panel->m_dragFreqScale && !m_panel->m_dragFreqScaleZoom);

    if (regenFreqScale) {
        if (!m_frequencyScaleFBO || m_frequencyScaleFBO->size() != QSize(devWidth, devHeight) || m_panel->m_freqScalePanadapterRenew) {
            if (m_frequencyScaleFBO) {
                delete m_frequencyScaleFBO;
                m_frequencyScaleFBO = nullptr;
            }
            m_frequencyScaleFBO = new QOpenGLFramebufferObject(devWidth, devHeight);
        }

        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        glViewport(0, 0, devWidth, devHeight);

        m_frequencyScaleFBO->bind();
        renderPanHorizontalScale();
        m_frequencyScaleFBO->release();
        QOpenGLFramebufferObject::bindDefault();

        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        m_panel->m_freqScalePanadapterUpdate = false;
        m_panel->m_freqScalePanadapterRenew = false;
    }

    m_panel->drawCachedTexture(m_panel->m_freqScalePanRect, m_frequencyScaleFBO->texture(), 0.0f);
}

void GridRenderer::renderPanHorizontalScale() {
    ensureGL();
    if (!m_frequencyScaleFBO)
        return;

    const qreal dpr = m_panel->devicePixelRatioF();
    QOpenGLPaintDevice paintDevice(m_frequencyScaleFBO->size());
    paintDevice.setDevicePixelRatio(dpr);

    painter.begin(&paintDevice);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const int plotLeft = m_panel->m_dBmScalePanRect.isValid() ? m_panel->m_dBmScalePanRect.width() : 0;
    const int fullWidth = m_panel->m_freqScalePanRect.width();
    const int fullHeight = m_panel->m_freqScalePanRect.height();

    const qreal freqSpan = m_panel->displayedFrequencySpanHz();
    const qreal upperFreq = (qreal)m_panel->m_centerFrequency + freqSpan / 2;

    double freqScale = 1;
    QString fstr = QStringLiteral("Hz");
    if (upperFreq >= 1e6) { freqScale = 1e6; fstr = QStringLiteral("MHz"); }
    else if (upperFreq >= 1e3) { freqScale = 1e3; fstr = QStringLiteral("kHz"); }

    auto formatFreqLabel = [&](qreal freqHz, bool allowExtraDigit) -> QString {
        const int decimals = (allowExtraDigit && freqScale >= 1e3) ? 4 : 3;
        QString str = QString::number(freqHz / freqScale, 'f', decimals);
        if (freqScale == 1e3 && decimals == 3)
            while (str.endsWith('0')) str.remove(str.size() - 1, 1);
        if (str.endsWith('.')) str.remove(str.size() - 1, 1);
        return str;
    };

    // Full-width background
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(0, 0, fullWidth, fullHeight, QColor(10, 14, 20, 255));
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    // Top border line
    painter.setPen(QPen(QColor(45, 60, 75), 1.0));
    painter.drawLine(0, 0, fullWidth, 0);

    QFont font = m_panel->m_oglTextSmall->font();
    painter.setFont(font);
    QFontMetrics fm(font);
    const int fontHeight = fm.height();

    // Unit pill badge on right
    const int unitW = fm.horizontalAdvance(fstr) + 10;
    const int unitH = fontHeight + 2;
    const QRectF unitRect(fullWidth - unitW - 4, (fullHeight - unitH) / 2.0, unitW, unitH);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(28, 38, 48, 230));
    painter.drawRoundedRect(unitRect, 3.0, 3.0);
    painter.setPen(QColor(255, 90, 130));
    painter.drawText(unitRect, Qt::AlignCenter, fstr);

    // Minor sub-ticks
    const int sublen = m_panel->m_frequencyScale.subPointPositions.length();
    if (sublen > 0) {
        painter.setPen(QPen(QColor(90, 125, 145), 1.0));
        for (int i = 0; i < sublen; i++) {
            const int screenX = plotLeft + m_panel->m_frequencyScale.subPointPositions.at(i);
            if (screenX >= plotLeft && screenX < fullWidth) {
                painter.drawLine(screenX, 1, screenX, 3);
            }
        }
    }

    // Major ticks & labels
    const int len = m_panel->m_frequencyScale.mainPointPositions.length();
    int lastLabelRight = plotLeft;

    if (len > 0) {
        painter.setPen(QPen(QColor(160, 195, 215), 1.0));
        for (int i = 0; i < len; i++) {
            const int screenX = plotLeft + m_panel->m_frequencyScale.mainPointPositions.at(i);
            if (screenX >= plotLeft && screenX < fullWidth) {
                painter.drawLine(screenX, 1, screenX, 5);
            }
        }

        for (int i = 0; i < len; i++) {
            const int tickX = m_panel->m_frequencyScale.mainPointPositions.at(i);
            const int screenX = plotLeft + tickX;
            if (screenX < plotLeft || screenX > fullWidth)
                continue;

            const QString str = formatFreqLabel(m_panel->m_frequencyScale.mainPoints.at(i), tickX == 0);
            const int textWidth = fm.horizontalAdvance(str);
            int textX = (tickX == 0) ? (screenX + 2) : (screenX - textWidth / 2);

            if (textX < plotLeft + 2)
                textX = plotLeft + 2;

            if (textX < lastLabelRight + 4)
                continue;

            if (textX + textWidth >= unitRect.left() - 4)
                continue;

            const QRectF textRect(textX, 6, textWidth, fontHeight);
            painter.setPen(QColor(210, 230, 245));
            painter.drawText(textRect, Qt::AlignCenter, str);

            lastLabelRight = textX + textWidth;
        }
    }

    painter.end();
}

void GridRenderer::drawWaterfallVerticalScale() {
    ensureGL();

    if (!m_panel->m_secScaleWaterfallRect.isValid())
        return;

    const int width = m_panel->m_secScaleWaterfallRect.width();
    const int height = m_panel->m_secScaleWaterfallRect.height();

    if (width <= 0 || height <= 0 || width > 1000 || height > 10000)
        return;

    if (m_panel->m_resizeTime.elapsed() < 500 && (width > m_panel->size().width() || height > m_panel->size().height()))
        return;

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    const qreal dpr = m_panel->devicePixelRatioF();
    const int devWidth = qMax(1, int(qRound(width * dpr)));
    const int devHeight = qMax(1, int(qRound(height * dpr)));

    const bool regenSecScale = !m_secScaleWaterfallFBO || m_secScaleWaterfallFBO->size() != QSize(devWidth, devHeight)
                               || m_panel->m_secScaleWaterfallRenew || m_panel->m_secScaleWaterfallUpdate;

    if (regenSecScale) {
        if (!m_secScaleWaterfallFBO || m_secScaleWaterfallFBO->size() != QSize(devWidth, devHeight) || m_panel->m_secScaleWaterfallRenew) {
            if (m_secScaleWaterfallFBO) {
                delete m_secScaleWaterfallFBO;
                m_secScaleWaterfallFBO = nullptr;
            }
            m_secScaleWaterfallFBO = new QOpenGLFramebufferObject(devWidth, devHeight);
        }

        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        glViewport(0, 0, devWidth, devHeight);

        m_secScaleWaterfallFBO->bind();
        renderWaterfallVerticalScale();
        m_secScaleWaterfallFBO->release();
        QOpenGLFramebufferObject::bindDefault();

        m_panel->m_secScaleWaterfallUpdate = false;
        m_panel->m_secScaleWaterfallRenew = false;
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    }

    if (m_secScaleWaterfallFBO)
        m_panel->drawCachedTexture(m_panel->m_secScaleWaterfallRect, m_secScaleWaterfallFBO->texture(), 4.0f);
}

void GridRenderer::renderWaterfallVerticalScale() {
    ensureGL();
    if (!m_secScaleWaterfallFBO)
        return;

    const qreal dpr = m_panel->devicePixelRatioF();
    QOpenGLPaintDevice paintDevice(m_secScaleWaterfallFBO->size());
    paintDevice.setDevicePixelRatio(dpr);

    painter.begin(&paintDevice);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const int width = m_panel->m_secScaleWaterfallRect.width();
    const int height = m_panel->m_secScaleWaterfallRect.height();
    const qreal secRange = qAbs(m_panel->m_secWaterfallMax - m_panel->m_secWaterfallMin);
    if (secRange <= 0) {
        painter.end();
        return;
    }

    QFont font = m_panel->m_smallSize ? m_panel->m_fonts.smallFont : m_panel->m_fonts.bigFont1;
    painter.setFont(font);
    QFontMetrics fm(font);
    const int fontHeight = fm.height();

    const qreal unit = qreal(height) / secRange;
    m_panel->m_secScale = getYRuler2(m_panel->m_secScaleWaterfallRect, fontHeight + 6, unit, m_panel->m_secWaterfallMin, m_panel->m_secWaterfallMax);

    // Background panel
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(0, 0, width, height, QColor(14, 18, 24, 210));
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    // Right border
    painter.setPen(QPen(QColor(45, 60, 75), 1.0));
    painter.drawLine(width - 1, 0, width - 1, height);

    const int len = m_panel->m_secScale.mainPointPositions.length();
    const int sublen = m_panel->m_secScale.subPointPositions.length();

    // Minor ticks
    if (sublen > 0) {
        painter.setPen(QPen(QColor(90, 125, 145), 1.0));
        for (int i = 0; i < sublen; i++) {
            const int y = m_panel->m_secScale.subPointPositions.at(i);
            painter.drawLine(width - 1, y, width - 3, y);
        }
    }

    // Major ticks
    if (len > 0) {
        painter.setPen(QPen(QColor(160, 195, 215), 1.0));
        for (int i = 0; i < len; i++) {
            const int y = m_panel->m_secScale.mainPointPositions.at(i);
            painter.drawLine(width - 1, y, width - 6, y);
        }
    }

    // Numbers
    painter.setPen(QPen(QColor(200, 225, 240)));

    if (len > 0) {
        for (int i = 0; i < len; i++) {
            const int y = m_panel->m_secScale.mainPointPositions.at(i);
            const QString str = QString::number(m_panel->m_secScale.mainPoints.at(i), 'f', 1);
            const QRectF textRect(2, y - fontHeight / 2.0, width - 9, fontHeight);
            if (textRect.top() >= 2 && textRect.bottom() <= (height - 18)) {
                painter.drawText(textRect, Qt::AlignRight | Qt::AlignVCenter, str);
            }
        }
    }

    // Unit "sec" at bottom
    const QRectF unitRect(2, height - 16, width - 4, 14);
    painter.setFont(m_panel->m_fonts.smallFont);
    painter.setPen(QPen(QColor(255, 90, 130)));
    painter.drawText(unitRect, Qt::AlignCenter, QStringLiteral("sec"));

    painter.end();
}

void GridRenderer::drawPanadapterGrid() {
    ensureGL();
    if (m_panel->m_overlayRenderer) {
        QMatrix4x4 projection;
        projection.ortho(0, m_panel->size().width(), m_panel->size().height(), 0, -10, 10);
        float alpha = (m_panel->m_receiver == m_panel->m_currentReceiver) ? 1.0f : 0.8f;
        m_panel->m_overlayRenderer->drawGrid(projection, m_panel->m_panRect, m_panel->m_freqScalePanRect, m_panel->m_frequencyScale, m_panel->m_dBmScale,
                                  m_panel->m_dBmScalePanRect.width(),
                                  m_panel->m_redGrid, m_panel->m_greenGrid, m_panel->m_blueGrid, alpha, m_panel->m_panGrid);
    }
}
