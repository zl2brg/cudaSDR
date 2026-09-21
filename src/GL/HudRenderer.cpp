#include "HudRenderer.h"
#include "cusdr_oglReceiverPanel.h"
#include "cusdr_oglUtils.h"
#include "cusdr_hamDatabase.h"
#include "OverlayRenderer.h"
#include "Models/SliceModel.h"
#include "cusdr_glDraw.h"

#include <QMatrix4x4>
#include <QVector>
#include <algorithm>
#include <cmath>

namespace {

QStringList wrapRttyTerminal(const QString &text, int cols)
{
    cols = qMax(8, cols);
    QStringList lines;
    QString line;
    line.reserve(cols);

    auto flush = [&]() {
        lines.append(line);
        line.clear();
    };

    for (int i = 0; i < text.size(); ++i) {
        QChar c = text.at(i);
        if (c == QLatin1Char('\r')) {
            if (i + 1 < text.size() && text.at(i + 1) == QLatin1Char('\n'))
                ++i;
            flush();
            continue;
        }
        if (c == QLatin1Char('\n')) {
            flush();
            continue;
        }
        if (c < QLatin1Char(' '))
            c = QLatin1Char(' ');
        line.append(c);
        if (line.size() >= cols)
            flush();
    }
    lines.append(line);
    return lines;
}

} // namespace

HudRenderer::HudRenderer(QGLReceiverPanel *panel)
    : m_panel(panel)
    , m_glReady(false)
{
}

HudRenderer::~HudRenderer()
{
}

void HudRenderer::ensureGL()
{
    if (!m_glReady) {
        initializeOpenGLFunctions();
        m_glReady = true;
    }
}

void HudRenderer::drawVFOControl() {

	// lock Panadapter
	QString str = "PAN LOCKED";
	int x1 = (m_panel->m_panSMeterRect.isValid() &&
	          m_panel->m_panSMeterRect.left() < m_panel->m_dBmScalePanRect.right() + 50 &&
	          m_panel->m_panSMeterRect.top() < m_panel->m_panRect.top() + 80)
	             ? (m_panel->m_panSMeterRect.right() + 8)
	             : (m_panel->m_dBmScalePanRect.right() + 5);
	if (m_panel->m_panFreqRect.isValid() && m_panel->m_panFreqRect.left() < x1 + 150 && m_panel->m_panFreqRect.right() >= x1) {
		x1 = m_panel->m_panFreqRect.right() + 8;
	}
	int y1 = 3;

	if (m_panel->m_panLocked) {
		
		if (m_panel->m_dataEngineState == QSDR::DataEngineUp) {
				
			m_panel->m_glTextColor = QColor(0, 0, 0, 255);
			m_panel->renderPanelText(m_panel->m_oglTextSmall, x1 + 3, y1, 0.0f, str);
			m_panel->m_glTextColor = QColor(255, 170, 90, 200);
			m_panel->renderPanelText(m_panel->m_oglTextSmall, x1 + 1, y1 - 2, 1.0f, str);
		}
		else {

			m_panel->m_glTextColor = QColor(0, 0, 0, 255);
			m_panel->renderPanelText(m_panel->m_oglTextSmall, x1 + 3, y1, 0.0f, str);
			m_panel->m_glTextColor = QColor(150, 150, 150, 100);
			m_panel->renderPanelText(m_panel->m_oglTextSmall, x1 + 1, y1 - 2, 1.0f, str);
		}
	}
	
	// click VFO
    x1 += m_panel->m_fonts.smallFontMetrics->horizontalAdvance(str) + 12;
	str = "CLICK VFO";

	if (m_panel->m_clickVFO) {

		if (m_panel->m_dataEngineState == QSDR::DataEngineUp) {
				
			m_panel->m_glTextColor = QColor(0, 0, 0, 255);
			m_panel->renderPanelText(m_panel->m_oglTextSmall, x1 + 3, y1, 0.0f, str);
			m_panel->m_glTextColor = QColor(255, 170, 90, 200);
			m_panel->renderPanelText(m_panel->m_oglTextSmall, x1 + 1, y1 - 2, 1.0f, str);
		}
		else {

			m_panel->m_glTextColor = QColor(0, 0, 0, 255);
			m_panel->renderPanelText(m_panel->m_oglTextSmall, x1 + 3, y1, 0.0f, str);
			m_panel->m_glTextColor = QColor(150, 150, 150, 100);
			m_panel->renderPanelText(m_panel->m_oglTextSmall, x1 + 1, y1 - 2, 1.0f, str);
		}
	}

	// FFT size
		str = "sample size: %1";
        x1 = m_panel->m_panRect.right() - m_panel->m_fonts.smallFontMetrics->horizontalAdvance(str) - 65;

		if (m_panel->m_dataEngineState == QSDR::DataEngineUp) {
				
			m_panel->m_glTextColor = QColor(0, 0, 0, 255);
			m_panel->renderPanelText(m_panel->m_oglTextSmall, x1 + 3, y1, 0.0f, str.arg(m_panel->m_sampleSize));
			m_panel->m_glTextColor = QColor(255, 170, 90, 200);
			m_panel->renderPanelText(m_panel->m_oglTextSmall, x1 + 1, y1 - 2, 1.0f, str.arg(m_panel->m_sampleSize));
		}
		
		str = "FFT: %1";
		//float res;
		QString s;

		switch (m_panel->m_fftMult) {

			case 1:
				s = "4k";
				break;

			case 2:
				s = "8k";
				break;

			case 4:
				s = "16k";
				break;

			case 8:
				s = "32k";
				break;

			case 16:
				s = "64k";
				break;
		}
        x1 = m_panel->m_panRect.right() - m_panel->m_fonts.smallFontMetrics->horizontalAdvance(str) - 5;

		if (m_panel->m_dataEngineState == QSDR::DataEngineUp) {
				
			m_panel->m_glTextColor = QColor(0, 0, 0, 255);
			m_panel->renderPanelText(m_panel->m_oglTextSmall, x1 + 3, y1, 0.0f, str.arg(s));
			m_panel->m_glTextColor = QColor(255, 170, 90, 200);
			m_panel->renderPanelText(m_panel->m_oglTextSmall, x1 + 1, y1 - 2, 1.0f, str.arg(s));
		}


	int delta = qRound((m_panel->m_deltaF * m_panel->m_panRect.width()) / m_panel->displayedZoomFactor());
	//GRAPHICS_DEBUG << "delta = " << delta;

	if (delta > m_panel->m_panRect.width()/2) {
	
		QColor col = QColor(255, 40, 40, 255);
		str = "<< VFO %1";
		str = str.arg(frequencyString(m_panel->m_vfoFrequency, false));

		int x = m_panel->m_dBmScalePanRect.right();
		int y = 25;

        QRect rect = QRect(x, y, m_panel->m_fonts.smallFontMetrics->horizontalAdvance(str) + 4, m_panel->m_fonts.fontHeightSmallFont + 2);
		m_panel->drawPanelRect(rect, col, 2.0f);
		m_panel->m_glTextColor = QColor(255, 255, 255, 255);
		m_panel->renderPanelText(m_panel->m_oglTextSmall, x + 1, y - 2, 3.0f, str);
	}

	if (delta < -m_panel->m_panRect.width()/2) {
		
		QColor col = QColor(255, 40, 40, 255);
		str = "%1 VFO >>";
		str = str.arg(frequencyString(m_panel->m_vfoFrequency, false));

        int x = m_panel->m_panRect.right() - m_panel->m_fonts.smallFontMetrics->horizontalAdvance(str);
		int y = 25;

        QRect rect = QRect(x, y, m_panel->m_fonts.smallFontMetrics->horizontalAdvance(str) + 4, m_panel->m_fonts.fontHeightSmallFont + 2);
		m_panel->drawPanelRect(rect, col, 2.0f);
		m_panel->m_glTextColor = QColor(255, 255, 255, 255);
		m_panel->renderPanelText(m_panel->m_oglTextSmall, x + 1, y - 2, 3.0f, str);
	}

	//qglColor(QColor(0, 0, 0));
	//m_panel->m_oglTextSmall->renderFreqText(x1+1, y1-2, 3.0f, str);

	// m_panel->set Center = VFO frequency button
	/*QColor col;
    x1 += m_panel->m_fonts.smallFontMetrics->horizontalAdvance(str) + 7;
	str = "mid = vfo";

	if (m_panel->m_dataEngineState == QSDR::DataEngineUp) {

		if (m_panel->m_receiver == m_panel->m_currentReceiver) {
		
			if (m_panel->m_panLocked)
				col = QColor(1, 150, 140, 140);
			else
				col = QColor(1, 230, 220, 140);
		}
		else
			col = QColor(90, 100, 90, 140);
	}
	else
		col = m_panel->m_darkColor;
	
    m_panel->m_midToVfoButtonRect = QRect(x1, y1, m_panel->m_fonts.smallFontMetrics->horizontalAdvance(str) + 5, m_panel->m_fonts.fontHeightSmallFont + 2);
	m_panel->drawPanelRect(m_panel->m_midToVfoButtonRect, col, 2.0f);
	qglColor(QColor(0, 0, 0));
	m_panel->m_oglTextSmall->renderFreqText(x1+1, y1-2, 3.0f, str);*/


	// m_panel->set VFO = Center frequency button
    /*x1 += m_panel->m_fonts.smallFontMetrics->horizontalAdvance(str) + 7;
	str = "vfo = mid";
	
	if (m_panel->m_dataEngineState == QSDR::DataEngineUp) {

		if (m_panel->m_receiver == m_panel->m_currentReceiver)
			col = QColor(1, 230, 220, 140);
		else
			col = QColor(90, 100, 90, 140);
	}
	else
		col = m_panel->m_darkColor;

    m_panel->m_vfoToMidButtonRect = QRect(x1, y1, m_panel->m_fonts.smallFontMetrics->horizontalAdvance(str) + 5, m_panel->m_fonts.fontHeightSmallFont + 2);
	m_panel->drawPanelRect(m_panel->m_vfoToMidButtonRect, col, 2.0f);
	qglColor(QColor(0, 0, 0));
	m_panel->m_oglTextSmall->renderFreqText(x1+1, y1-2, 3.0f, str);*/
}

void HudRenderer::drawPanadapterSMeter() {
    ensureGL();
    if (!m_panel || !m_panel->set || !m_panel->set->getShowPanadapterSMeter()) {
        m_panel->m_panSMeterRect = QRect();
        return;
    }

    const int sizePref = m_panel->set->getPanadapterSMeterSize();
    bool isLarge = false;
    if (sizePref == 1) {
        isLarge = false; // Forced Compact
    } else if (sizePref == 2) {
        isLarge = true;  // Forced Large
    } else {
        // Auto: Large only on high-DPI with sufficient height, or spacious panadapter panel
        const bool highDpi = (m_panel->m_panelDpr >= 1.4);
        const bool largePanel = (m_panel->m_panRect.height() >= 240 && m_panel->m_panRect.width() >= 900);
        isLarge = (highDpi && m_panel->m_panRect.height() >= 170) || largePanel;
    }

    int cardW = isLarge ? 360 : 300;
    int cardH = isLarge ? 74 : 57;

    // In auto mode, if the panadapter slice is too narrow or short for large, fallback to compact
    if (isLarge && sizePref == 0 && (m_panel->m_panRect.width() < 420 || m_panel->m_panRect.height() < 120)) {
        isLarge = false;
        cardW = 300;
        cardH = 57;
    }

    if (m_panel->m_panRect.width() < (cardW + 50) || m_panel->m_panRect.height() < 85) {
        m_panel->m_panSMeterRect = QRect();
        return;
    }

    int x0 = m_panel->m_dBmScalePanRect.right() + 8;
    int y0 = m_panel->m_panRect.top() + 6;

    if (m_panel->m_hasCustomPanSMeterPos && m_panel->m_panSMeterPos.x() >= 0 && m_panel->m_panSMeterPos.y() >= 0) {
        x0 = qBound(m_panel->m_panRect.left() + 4, m_panel->m_panSMeterPos.x(), m_panel->m_panRect.right() - cardW - 4);
        y0 = qBound(m_panel->m_panRect.top() + 4, m_panel->m_panSMeterPos.y(), m_panel->m_panRect.bottom() - cardH - 4);
    }
    m_panel->m_panSMeterRect = QRect(x0, y0, cardW, cardH);

    const QMatrix4x4 proj = m_panel->panelProjection();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    // 1. Background Card & Outline Border
    m_panel->drawPanelRect(m_panel->m_panSMeterRect, QColor(14, 18, 24, 220), 3.0f);

    const float x1f = float(x0), y1f = float(y0);
    const float x2f = float(x0 + cardW - 1), y2f = float(y0 + cardH - 1);
    const QColor borderCol(50, 62, 75, 200);
    const float br = borderCol.redF(), bg = borderCol.greenF(), bb = borderCol.blueF();
    const GlDraw::Vec3Rgb borderLines[8] = {
        { x1f, y1f, 3.1f, br, bg, bb }, { x2f, y1f, 3.1f, br, bg, bb },
        { x2f, y1f, 3.1f, br, bg, bb }, { x2f, y2f, 3.1f, br, bg, bb },
        { x2f, y2f, 3.1f, br, bg, bb }, { x1f, y2f, 3.1f, br, bg, bb },
        { x1f, y2f, 3.1f, br, bg, bb }, { x1f, y1f, 3.1f, br, bg, bb }
    };
    m_panel->m_vao.bind();
    glLineWidth(1.0f);
    GlDraw::drawColoredLines(this, m_panel->m_shaderProgram, m_panel->m_vbo, proj, borderLines, 8);

    // 2. Digital Readout Row (top of card)
    OGLText *fontBadge = isLarge ? m_panel->m_oglTextNormal : m_panel->m_oglTextSmall;
    OGLText *fontReadout = isLarge ? m_panel->m_oglTextBig2 : m_panel->m_oglTextSmall;
    const int badgeH = isLarge ? 18 : 16;
    const int badgePad = isLarge ? 10 : 8;
    const int rowY = isLarge ? (y0 + 5) : (y0 + 3);
    const int badgeX = isLarge ? (x0 + 8) : (x0 + 6);
    const int badgeTextX = isLarge ? (x0 + 13) : (x0 + 10);

    const QString rxBadge = QStringLiteral("RX%1").arg(m_panel->m_receiver + 1);
    const int badgeW = fontBadge->fontMetrics().horizontalAdvance(rxBadge) + badgePad;
    m_panel->drawPanelRect(QRect(badgeX, rowY, badgeW, badgeH), QColor(28, 38, 50, 230), 3.1f);
    m_panel->m_glTextColor = QColor(180, 205, 225);
    m_panel->renderPanelText(fontBadge, float(badgeTextX), float(rowY), 3.2f, rxBadge);

    const float rawDbm = m_panel->m_sMeterOrgValue;
    QString sUnitStr;
    QColor sUnitCol;
    if (rawDbm >= -73.0f) {
        const int over = qRound(rawDbm - (-73.0f));
        sUnitStr = (over > 0) ? QStringLiteral("S9+%1").arg(over) : QStringLiteral("S9");
        sUnitCol = (over >= 40) ? QColor(255, 60, 60) :
                   (over >= 10) ? QColor(255, 200, 50) : QColor(255, 255, 255);
    } else {
        const int s = qBound(0, static_cast<int>(9.0f + (rawDbm - (-73.0f)) / 6.0f + 0.5f), 9);
        sUnitStr = QStringLiteral("S%1").arg(s);
        sUnitCol = QColor(56, 242, 115);
    }
    m_panel->m_glTextColor = sUnitCol;
    const float sUnitX = float(x0 + badgeW + (isLarge ? 18 : 14));
    m_panel->renderPanelText(fontReadout, sUnitX, float(rowY), 3.2f, sUnitStr);

    const QString dbmStr = QString::asprintf("%.1f dBm", rawDbm);
    const int dbmW = fontReadout->fontMetrics().horizontalAdvance(dbmStr);
    m_panel->m_glTextColor = QColor(210, 220, 230);
    const float dbmX = float(x0 + cardW - dbmW - (isLarge ? 10 : 8));
    m_panel->renderPanelText(fontReadout, dbmX, float(rowY), 3.2f, dbmStr);

    // 3. Scale Geometry: -140 dBm to 0 dBm (140 dB span)
    const int xStart = x0 + (isLarge ? 12 : 10);
    const int xEnd = x0 + cardW - (isLarge ? 12 : 10);
    const int scaleW = xEnd - xStart;
    const float unit = float(scaleW) / 140.0f;
    const float yRailTop = float(y0 + (isLarge ? 29 : 23));
    const float yRailBottom = float(y0 + (isLarge ? 46 : 34));

    QVector<GlDraw::Vec3Rgb> scaleLines;
    scaleLines.reserve(10 + scaleW * 2 + 40);

    // Dual horizontal rails
    const QColor railCol(110, 140, 155, 220);
    const float rr = railCol.redF(), rg = railCol.greenF(), rb = railCol.blueF();
    scaleLines.append({ float(xStart), yRailTop, 3.2f, rr, rg, rb });
    scaleLines.append({ float(xEnd), yRailTop, 3.2f, rr, rg, rb });
    scaleLines.append({ float(xStart), yRailBottom, 3.2f, rr, rg, rb });
    scaleLines.append({ float(xEnd), yRailBottom, 3.2f, rr, rg, rb });

    // Vertical ladder grille every 3 px
    const QColor grilleCol(60, 80, 95, 180);
    const float gr = grilleCol.redF(), gg = grilleCol.greenF(), gb = grilleCol.blueF();
    for (int x = xStart + 2; x < xEnd; x += 3) {
        scaleLines.append({ float(x), yRailTop + 1.5f, 3.1f, gr, gg, gb });
        scaleLines.append({ float(x), yRailBottom - 1.5f, 3.1f, gr, gg, gb });
    }

    // Top dBm ticks (-120 dBm to 0 dBm)
    const float majorTickLen = isLarge ? 5.0f : 4.5f;
    const float minorTickLen = isLarge ? 3.0f : 2.5f;
    const QColor tickCol(160, 180, 195, 230);
    const float tr = tickCol.redF(), tg = tickCol.greenF(), tb = tickCol.blueF();
    for (int db = 20; db <= 140; db += 20) {
        const float xt = float(xStart) + float(db) * unit;
        scaleLines.append({ xt, yRailTop - majorTickLen, 3.2f, tr, tg, tb });
        scaleLines.append({ xt, yRailTop, 3.2f, tr, tg, tb });
    }
    for (int db = 10; db < 140; db += 20) {
        const float xt = float(xStart) + float(db) * unit;
        scaleLines.append({ xt, yRailTop - minorTickLen, 3.2f, tr, tg, tb });
        scaleLines.append({ xt, yRailTop, 3.2f, tr, tg, tb });
    }

    // Bottom S-Unit ticks (S1..S9, +20, +40, +60)
    struct SMark {
        int dbFromBase;
        int zone; // 0=green, 1=yellow, 2=red
        bool major;
    };
    static const SMark sMarks[] = {
        { 19, 0, true },   // S1
        { 31, 0, true },   // S3
        { 43, 0, true },   // S5
        { 55, 0, true },   // S7
        { 67, 0, true },   // S9
        { 87, 1, true },   // +20
        { 107, 2, true },  // +40
        { 127, 2, true }   // +60
    };

    for (const auto &mark : sMarks) {
        const float xt = float(xStart) + float(mark.dbFromBase) * unit;
        float mr, mg, mb;
        if (mark.zone == 0) {
            mr = 56.0f / 255.0f; mg = 242.0f / 255.0f; mb = 115.0f / 255.0f;
        } else if (mark.zone == 1) {
            mr = 255.0f / 255.0f; mg = 200.0f / 255.0f; mb = 50.0f / 255.0f;
        } else {
            mr = 255.0f / 255.0f; mg = 60.0f / 255.0f; mb = 60.0f / 255.0f;
        }
        const float tickLen = mark.major ? majorTickLen : minorTickLen;
        scaleLines.append({ xt, yRailBottom, 3.2f, mr, mg, mb });
        scaleLines.append({ xt, yRailBottom + tickLen, 3.2f, mr, mg, mb });
    }

    // Colored bottom guide rails at yRailBottom
    const float xS9 = float(xStart) + 67.0f * unit;
    const float xP30 = float(xStart) + 97.0f * unit;
    scaleLines.append({ float(xStart), yRailBottom, 3.2f, 56.0f / 255.0f, 242.0f / 255.0f, 115.0f / 255.0f });
    scaleLines.append({ xS9, yRailBottom, 3.2f, 56.0f / 255.0f, 242.0f / 255.0f, 115.0f / 255.0f });
    scaleLines.append({ xS9, yRailBottom, 3.2f, 255.0f / 255.0f, 200.0f / 255.0f, 50.0f / 255.0f });
    scaleLines.append({ xP30, yRailBottom, 3.2f, 255.0f / 255.0f, 200.0f / 255.0f, 50.0f / 255.0f });
    scaleLines.append({ xP30, yRailBottom, 3.2f, 255.0f / 255.0f, 60.0f / 255.0f, 60.0f / 255.0f });
    scaleLines.append({ float(xEnd), yRailBottom, 3.2f, 255.0f / 255.0f, 60.0f / 255.0f, 60.0f / 255.0f });

    m_panel->m_vao.bind();
    glLineWidth(1.0f);
    GlDraw::drawColoredLines(this, m_panel->m_shaderProgram, m_panel->m_vbo, proj,
                             scaleLines.constData(), scaleLines.size());

    // 4. Active Signal Gradient Fill Bar
    const float avgVal = qBound(0.0f, m_panel->m_sMeterAvgVal, 140.0f);
    const int barWidth = int(avgVal * unit);
    if (barWidth > 0 && m_panel->m_dataEngineState == QSDR::DataEngineUp) {
        const QRect barRect(xStart, int(yRailTop) + 2, barWidth, int(yRailBottom - yRailTop) - 3);
        const QColor cLeft(40, 180, 100);
        const QColor cRight = (avgVal > 97.0f) ? QColor(255, 50, 50) :
                              (avgVal > 67.0f) ? QColor(255, 200, 50) : QColor(56, 242, 115);
        m_panel->m_vao.bind();
        GlDraw::drawGradientRect(this, m_panel->m_shaderProgram, m_panel->m_vbo, proj,
                                 barRect, cLeft, cRight, true, 3.3f);
    }

    // 5. Needles
    if (m_panel->m_dataEngineState == QSDR::DataEngineUp && avgVal > 0.0f) {
        QVector<GlDraw::Vec3Rgb> needleLines;
        needleLines.reserve(4);

        const float needleExt = isLarge ? 4.0f : 3.0f;
        const float peakPipExt = isLarge ? 8.0f : 6.0f;

        // Main signal needle (bright white line)
        const float xNeedle = float(xStart) + avgVal * unit;
        needleLines.append({ xNeedle, yRailTop - needleExt, 3.5f, 1.0f, 1.0f, 1.0f });
        needleLines.append({ xNeedle, yRailBottom + needleExt, 3.5f, 1.0f, 1.0f, 1.0f });

        // Peak hold needle (amber/red pip at top)
        const float peakVal = qBound(0.0f, m_panel->m_sMeterHoldMax, 140.0f);
        if (peakVal > avgVal + 0.5f) {
            const float xPeak = float(xStart) + peakVal * unit;
            needleLines.append({ xPeak, yRailTop - needleExt, 3.5f, 1.0f, 0.4f, 0.4f });
            needleLines.append({ xPeak, yRailTop + peakPipExt, 3.5f, 1.0f, 0.4f, 0.4f });
        }

        m_panel->m_vao.bind();
        glLineWidth(2.0f);
        GlDraw::drawColoredLines(this, m_panel->m_shaderProgram, m_panel->m_vbo, proj,
                                 needleLines.constData(), needleLines.size());
    }

    // 6. Bottom S-Unit Labels
    struct SLabel {
        int db;
        const char *txt;
        QColor col;
    };
    static const SLabel sLabels[] = {
        { 19, "1", QColor(56, 242, 115) },
        { 31, "3", QColor(56, 242, 115) },
        { 43, "5", QColor(56, 242, 115) },
        { 55, "7", QColor(56, 242, 115) },
        { 67, "9", QColor(255, 255, 255) },
        { 87, "+20", QColor(255, 200, 50) },
        { 107, "+40", QColor(255, 80, 80) },
        { 127, "+60", QColor(255, 80, 80) }
    };

    OGLText *fontLabels = isLarge ? m_panel->m_oglTextNormal : m_panel->m_oglTextSmall;
    const float labelY = float(y0 + (isLarge ? 51 : 39));
    const QFontMetrics fm = fontLabels->fontMetrics();
    for (const auto &lbl : sLabels) {
        const QString markStr = QString::fromLatin1(lbl.txt);
        const int tw = fm.horizontalAdvance(markStr);
        const float xl = float(xStart) + float(lbl.db) * unit - float(tw) / 2.0f;
        m_panel->m_glTextColor = (m_panel->m_dataEngineState == QSDR::DataEngineUp)
                                     ? lbl.col
                                     : QColor(120, 130, 140);
        m_panel->renderPanelText(fontLabels, xl, labelY, 3.4f, markStr);
    }
}

void HudRenderer::drawPanadapterFreq() {
    ensureGL();
    if (!m_panel || !m_panel->set) {
        m_panel->m_panFreqRect = QRect();
        m_panel->m_panFreqVfoRect = QRect();
        return;
    }

    const bool sMeterAtTopLeft = m_panel->m_panSMeterRect.isValid() &&
                                 (m_panel->m_panSMeterRect.left() < m_panel->m_dBmScalePanRect.right() + 50) &&
                                 (m_panel->m_panSMeterRect.top() < m_panel->m_panRect.top() + 80);
    const bool isLargeSMeter = (m_panel->m_panSMeterRect.width() > 320);
    const int minPanWidth = sMeterAtTopLeft ? (isLargeSMeter ? 650 : 550) : 350;
    if (m_panel->m_panRect.width() < minPanWidth || m_panel->m_panRect.height() < 90) {
        m_panel->m_panFreqRect = QRect();
        m_panel->m_panFreqVfoRect = QRect();
        return;
    }

    const int cardW = 270;
    const int cardH = 57;
    const int y0 = m_panel->m_panRect.top() + 6;

    const int vfoX = m_panel->m_panRect.left() + qRound((qreal)(m_panel->m_panRect.width() / 2.0f) - m_panel->m_deltaF * m_panel->m_panRect.width() / m_panel->displayedZoomFactor());

    const int leftLimit = sMeterAtTopLeft ? (m_panel->m_panSMeterRect.right() + 8)
                                          : (m_panel->m_dBmScalePanRect.right() + 8);
    const int rightLimit = m_panel->m_panRect.right() - cardW - 6;

    int x0 = vfoX + 12;
    if (x0 > rightLimit)
        x0 = vfoX - cardW - 12;
    if (x0 < leftLimit)
        x0 = leftLimit;
    if (x0 > rightLimit && rightLimit >= leftLimit)
        x0 = rightLimit;

    m_panel->m_panFreqRect = QRect(x0, y0, cardW, cardH);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    // Helper to render text with a subtle drop shadow for crisp readability over any panadapter background
    auto renderShadowedText = [this](OGLText *font, float x, float y, float z, const QString &str, const QColor &color) {
        m_panel->m_glTextColor = QColor(0, 0, 0, 200);
        m_panel->renderPanelText(font, x + 1.0f, y + 1.0f, z, str);
        m_panel->m_glTextColor = color;
        m_panel->renderPanelText(font, x, y, z + 0.1f, str);
    };

    // 1. Top Meta Row: VFO button, DSP mode, Filter width, TX indicator, LO center, Band tag
    SliceModel *slice = m_panel->m_sliceModel;
    const bool isVfoB = (slice && slice->activeVfo() == SliceModel::VfoB);
    const QString vfoLetter = isVfoB ? QStringLiteral("B") : QStringLiteral("A");
    const int vfoBadgeW = 24;
    m_panel->m_panFreqVfoRect = QRect(x0 + 6, y0 + 3, vfoBadgeW, 16);
    const QColor vfoBg = (m_panel->m_dataEngineState == QSDR::DataEngineUp)
                             ? QColor(31, 111, 235)
                             : QColor(70, 75, 80);
    m_panel->drawPanelRect(m_panel->m_panFreqVfoRect, vfoBg, 3.1f);
    m_panel->m_glTextColor = (m_panel->m_dataEngineState == QSDR::DataEngineUp)
                                 ? QColor(255, 255, 255)
                                 : QColor(160, 170, 180);
    const int vfoLetterW = m_panel->m_oglTextSmall->fontMetrics().horizontalAdvance(vfoLetter);
    const float vfoLetterX = float(x0 + 6 + (vfoBadgeW - vfoLetterW) / 2);
    m_panel->renderPanelText(m_panel->m_oglTextSmall, vfoLetterX, float(y0 + 3), 3.2f, vfoLetter);

    // Mode
    const QString modeStr = m_panel->set->getDSPModeString(m_panel->m_sliceModel ? m_panel->m_sliceModel->dspMode() : m_panel->set->getDSPMode(m_panel->m_receiver));
    const int modeTextW = m_panel->m_oglTextSmall->fontMetrics().horizontalAdvance(modeStr);
    const int modeX = x0 + 6 + vfoBadgeW + 8;
    const QColor modeCol = (m_panel->m_dataEngineState == QSDR::DataEngineUp)
                               ? QColor(180, 215, 245)
                               : QColor(120, 130, 140);
    renderShadowedText(m_panel->m_oglTextSmall, float(modeX), float(y0 + 3), 3.2f, modeStr, modeCol);

    int nextBadgeX = modeX + modeTextW + 8;

    // Filter width (e.g. "2.8k")
    const QString fltStr = m_panel->m_filterWidthString;
    if (!fltStr.isEmpty()) {
        const int fltTextW = m_panel->m_oglTextSmall->fontMetrics().horizontalAdvance(fltStr);
        const QColor fltCol = (m_panel->m_dataEngineState == QSDR::DataEngineUp)
                                     ? QColor(205, 230, 120)
                                     : QColor(130, 140, 90);
        renderShadowedText(m_panel->m_oglTextSmall, float(nextBadgeX), float(y0 + 3), 3.2f, fltStr, fltCol);
        nextBadgeX += fltTextW + 8;
    }

    // TX indicator badge
    const bool isTx = (m_panel->set->getRadioState() != RadioState::RX);
    if (isTx) {
        const QString txStr = QStringLiteral("TX");
        const int txTextW = m_panel->m_oglTextSmall->fontMetrics().horizontalAdvance(txStr);
        const int txBadgeW = txTextW + 8;
        m_panel->drawPanelRect(QRect(nextBadgeX, y0 + 3, txBadgeW, 16), QColor(220, 40, 40, 230), 3.1f);
        m_panel->m_glTextColor = QColor(255, 255, 255);
        m_panel->renderPanelText(m_panel->m_oglTextSmall, float(nextBadgeX + 4), float(y0 + 3), 3.2f, txStr);
        nextBadgeX += txBadgeW + 8;
    }

    // Center LO frequency (when VFO is offset from LO center)
    if (m_panel->m_deltaFrequency != 0) {
        const qint64 cf = m_panel->m_centerFrequency;
        const qint64 cfMhz = cf / 1000000LL;
        const qint64 cfKhz = (cf / 1000LL) % 1000LL;
        const qint64 cfHz  = cf % 1000LL;
        const QString loStr = (cfHz == 0)
            ? QStringLiteral("LO %1.%2").arg(cfMhz).arg(cfKhz, 3, 10, QLatin1Char('0'))
            : QStringLiteral("LO %1.%2.%3").arg(cfMhz).arg(cfKhz, 3, 10, QLatin1Char('0')).arg(cfHz, 3, 10, QLatin1Char('0'));
        const int loTextW = m_panel->m_oglTextSmall->fontMetrics().horizontalAdvance(loStr);
        const QColor loCol = (m_panel->m_dataEngineState == QSDR::DataEngineUp)
                                     ? QColor(80, 180, 240)
                                     : QColor(100, 130, 150);
        renderShadowedText(m_panel->m_oglTextSmall, float(nextBadgeX), float(y0 + 3), 3.2f, loStr, loCol);
        nextBadgeX += loTextW + 8;
    }

    // Band Tag (right-aligned on top row)
    const qint64 activeFreq = m_panel->m_vfoFrequency;
    QString bandTag;
    if (m_panel->set) {
        bandTag = getHamBandTextString(m_panel->set->getHamBandTextList(), true, activeFreq);
    }
    if (!bandTag.isEmpty() && bandTag != QStringLiteral("Out of Band")) {
        const int bandTextW = m_panel->m_oglTextSmall->fontMetrics().horizontalAdvance(bandTag);
        const float bandX = float(x0 + cardW - bandTextW - 8);
        const QColor bandCol = (m_panel->m_dataEngineState == QSDR::DataEngineUp)
                                     ? QColor(239, 209, 110)
                                     : QColor(130, 120, 90);
        renderShadowedText(m_panel->m_oglTextSmall, bandX, float(y0 + 3), 3.2f, bandTag, bandCol);
    }

    // 2. Main Frequency Row
    const qint64 freq = activeFreq;
    const qint64 ghz = freq / 1000000000LL;
    const qint64 mhz = (freq / 1000000LL) % 1000LL;
    const qint64 khz = (freq / 1000LL) % 1000LL;
    const qint64 hz  = freq % 1000LL;

    QString freqStr;
    if (ghz > 0) {
        freqStr = QStringLiteral("%1.%2.%3.%4")
                      .arg(ghz)
                      .arg(mhz, 3, 10, QLatin1Char('0'))
                      .arg(khz, 3, 10, QLatin1Char('0'))
                      .arg(hz, 3, 10, QLatin1Char('0'));
    } else {
        freqStr = QStringLiteral("%1.%2.%3")
                      .arg(mhz)
                      .arg(khz, 3, 10, QLatin1Char('0'))
                      .arg(hz, 3, 10, QLatin1Char('0'));
    }

    const float yBaseline = float(y0 + cardH - 7);
    const float yFreq = yBaseline - float(m_panel->m_oglTextFreq2->fontMetrics().ascent());
    const float xFreq = float(x0 + 8);

    const QColor digitCol = isTx ? QColor(255, 65, 65)
                                : (m_panel->m_dataEngineState == QSDR::DataEngineUp)
                                      ? QColor(255, 255, 255)
                                      : QColor(130, 140, 150);
    renderShadowedText(m_panel->m_oglTextFreq2, xFreq, yFreq, 3.2f, freqStr, digitCol);

    const int freqW = m_panel->m_oglTextFreq2->fontMetrics().horizontalAdvance(freqStr);
    const float xUnit = xFreq + float(freqW) + 5.0f;
    const float yUnit = yBaseline - float(m_panel->m_oglTextNormal->fontMetrics().ascent());
    const QColor unitCol = (m_panel->m_dataEngineState == QSDR::DataEngineUp)
                                 ? QColor(130, 155, 175)
                                 : QColor(100, 110, 120);
    renderShadowedText(m_panel->m_oglTextNormal, xUnit, yUnit, 3.2f, QStringLiteral("MHz"), unitCol);

    // Mouse wheel step indicator (right-aligned on main row)
    const QString stepStr = m_panel->set->getValue1000(m_panel->set->getMouseWheelFreqStep(m_panel->m_receiver), 0, "Hz");
    if (!stepStr.isEmpty()) {
        const int stepW = m_panel->m_oglTextSmall->fontMetrics().horizontalAdvance(stepStr);
        const float xStep = float(x0 + cardW - stepW - 8);
        const float yStep = yBaseline - float(m_panel->m_oglTextSmall->fontMetrics().ascent());
        const QColor stepCol = (m_panel->m_dataEngineState == QSDR::DataEngineUp)
                                     ? QColor(110, 135, 155)
                                     : QColor(90, 100, 110);
        renderShadowedText(m_panel->m_oglTextSmall, xStep, yStep, 3.2f, stepStr, stepCol);
    }
}

void HudRenderer::drawCwDecoderHUD() {
    ensureGL();
    const DSPMode mode = m_panel->m_sliceModel ? m_panel->m_sliceModel->dspMode() : m_panel->m_dspMode;
    const DSPMode setMode = m_panel->m_sliceModel ? m_panel->m_sliceModel->dspMode() : static_cast<DSPMode>(m_panel->set->getDSPMode(m_panel->m_receiver));
    const bool isCw = (mode == DSPMode::CWL || mode == DSPMode::CWU ||
                       m_panel->m_dspMode == DSPMode::CWL || m_panel->m_dspMode == DSPMode::CWU ||
                       setMode == DSPMode::CWL || setMode == DSPMode::CWU);
    if (!isCw || (m_panel->m_sliceModel && !m_panel->m_sliceModel->cwDecodeEnabled())) {
        m_panel->m_cwTextRect = QRect();
        return;
    }

    const QString text = m_panel->m_sliceModel ? m_panel->m_sliceModel->cwDecodedText() : QString();
    const int wpm = m_panel->m_sliceModel ? m_panel->m_sliceModel->cwWpm() : 20;
    const bool toneOn = m_panel->m_sliceModel ? m_panel->m_sliceModel->cwToneActive() : false;

    m_panel->ensurePanelViewport();

    // 1. Calculate the exact target CW tone frequency (+pitch for CWU, -pitch for CWL)
    const int cwPitch = m_panel->set->getCwSidetoneFreq();
    const int trackedPitch = m_panel->m_sliceModel ? m_panel->m_sliceModel->cwTrackedPitch() : cwPitch;
    const qint64 vfoFreq = m_panel->m_sliceModel ? m_panel->m_sliceModel->frequency() : m_panel->m_vfoFrequency;
    const qint64 centerFreq = m_panel->m_sliceModel ? m_panel->m_sliceModel->centerFrequency() : m_panel->m_centerFrequency;

    // Tracked tone frequency for the dynamic green marker line
    const qint64 targetToneFreq = (mode == DSPMode::CWL) ? (vfoFreq - trackedPitch) : (vfoFreq + trackedPitch);
    // Nominal center frequency for steady, non-jittering text box placement
    const qint64 nominalToneFreq = (mode == DSPMode::CWL) ? (vfoFreq - cwPitch) : (vfoFreq + cwPitch);

    // 2. Compute screen X positions
    const float zoomFactor = m_panel->displayedZoomFactor();
    const float sampleRate = (m_panel->m_sampleRate > 0) ? (float)m_panel->m_sampleRate : 48000.0f;
    const float targetDeltaF = (float)(targetToneFreq - centerFreq) / sampleRate;
    const float nominalDeltaF = (float)(nominalToneFreq - centerFreq) / sampleRate;

    const float cwX = (float)m_panel->m_panRect.left() + ((float)m_panel->m_panRect.width() / 2.0f) + (targetDeltaF * (float)m_panel->m_panRect.width() / zoomFactor);
    const float nominalX = (float)m_panel->m_panRect.left() + ((float)m_panel->m_panRect.width() / 2.0f) + (nominalDeltaF * (float)m_panel->m_panRect.width() / zoomFactor);

    const int panLeft = m_panel->m_panRect.left();
    const int panRight = m_panel->m_panRect.right();
    const int panTop = m_panel->m_panRect.top();
    const int panBottom = m_panel->m_panRect.bottom();

    const bool lineVisible = (cwX >= (float)panLeft && cwX <= (float)panRight);

    const int badgeH = m_panel->m_fonts.fontHeightNormalFont + 6;
    const QString badgeText = (toneOn && std::abs(trackedPitch - cwPitch) > 6)
        ? QStringLiteral("CW %1W %2H").arg(wpm > 0 ? wpm : 20).arg(trackedPitch)
        : QStringLiteral("CW %1 WPM").arg(wpm > 0 ? wpm : 20);
    // Fixed constant badge width to prevent horizontal jitter
    const int badgeW = m_panel->m_oglTextSmall->fontMetrics().horizontalAdvance(QStringLiteral("CW 99W 9999H")) + 16;

    QString displayStr = text;
    if (displayStr.isEmpty()) {
        displayStr = (toneOn && std::abs(trackedPitch - cwPitch) > 6)
            ? QStringLiteral("<%1Hz CW>").arg(trackedPitch)
            : QStringLiteral("<%1Hz CW>").arg(cwPitch);
    }

    const int maxChars = 40;
    const int maxTextW = m_panel->m_oglTextNormal 
        ? qMax(280, m_panel->m_oglTextNormal->fontMetrics().averageCharWidth() * maxChars + 24)
        : 320;
    const int defaultBoxW = badgeW + maxTextW + 14;

    // Determine CW Box coordinates: custom user-dragged or default anchored position
    int textX = qRound(nominalX) + 6;
    int textY = panTop + (m_panel->m_panRect.height() / 2) - (badgeH / 2);

    if (m_panel->m_hasCustomCwBoxPos) {
        textX = m_panel->m_cwBoxPos.x();
        textY = m_panel->m_cwBoxPos.y();
    } else if (textX + defaultBoxW > panRight) {
        // Shift to left of nominal line if near right panadapter border
        textX = qRound(nominalX) - 6 - defaultBoxW;
    }

    textX = qBound(panLeft + 4, textX, panRight - 80);
    textY = qBound(panTop + 4, textY, panBottom - badgeH - 4);

    const int availableW = panRight - textX - 8;
    if (availableW > 60) {
        const int textAvailableW = qBound(20, availableW - badgeW - 10, maxTextW);
        QString trimmedText = displayStr;
        if (trimmedText.length() > maxChars) {
            trimmedText = trimmedText.right(maxChars);
        }
        while (!trimmedText.isEmpty() && m_panel->m_oglTextNormal->fontMetrics().horizontalAdvance(trimmedText) > textAvailableW) {
            trimmedText.remove(0, 1);
        }

        const int totalW = badgeW + (trimmedText.isEmpty() ? 0 : m_panel->m_oglTextNormal->fontMetrics().horizontalAdvance(trimmedText) + 8) + 6;
        m_panel->m_cwTextRect = QRect(textX, textY, totalW, badgeH);

        const int boxMidY = textY + (badgeH / 2);
        const int boxLeftX = textX;
        const int boxRightX = textX + totalW;

        // Dynamic pitch marker color (glowing green on tone, soft emerald when idle)
        const QColor lineColor = toneOn ? QColor(50, 240, 130, 230) : QColor(35, 170, 110, 140);

        if (lineVisible) {
            const int lineX = qRound(cwX);

            // Find the spectrum curve/peak Y coordinate at lineX
            int signalPeakY = panBottom - 12;
            const int relX = lineX - panLeft;
            if (!m_panel->m_panadapterBins.isEmpty() && relX >= 0 && relX < m_panel->m_panadapterBins.size()) {
                const qreal dBmRange = qMax(10.0, m_panel->m_dBmPanMax - m_panel->m_dBmPanMin);
                const qreal yScale = (qreal)m_panel->m_panRect.height() / dBmRange;

                // Sample a 3-bin window around the line to find the local peak Y
                qreal maxBinVal = m_panel->m_panadapterBins.at(relX);
                if (relX > 0) maxBinVal = qMax(maxBinVal, m_panel->m_panadapterBins.at(relX - 1));
                if (relX < m_panel->m_panadapterBins.size() - 1) maxBinVal = qMax(maxBinVal, m_panel->m_panadapterBins.at(relX + 1));

                const int curveY = panBottom - qRound(yScale * maxBinVal);
                signalPeakY = qBound(panTop + 10, curveY, panBottom - 4);
            }

            // Downward arrow tip stops 8px above the spectrum peak
            const int arrowTipY = qBound(panTop + 16, signalPeakY - 8, panBottom - 6);

            // 1. Vertical Arrow Shaft extending from box level down to arrow tip
            if (boxMidY < arrowTipY - 6) {
                m_panel->drawPanelRect(QRect(lineX - 1, boxMidY, 2, arrowTipY - 6 - boxMidY), lineColor, 3.5f);
            } else if (boxMidY > arrowTipY + 6) {
                m_panel->drawPanelRect(QRect(lineX - 1, arrowTipY + 6, 2, boxMidY - (arrowTipY + 6)), lineColor, 3.5f);
            }

            // 2. Downward Arrowhead pointing directly at the CW RF peak (at lineX, arrowTipY)
            m_panel->drawPanelRect(QRect(lineX - 4, arrowTipY - 6, 9, 2), lineColor, 3.6f);
            m_panel->drawPanelRect(QRect(lineX - 3, arrowTipY - 4, 7, 2), lineColor, 3.6f);
            m_panel->drawPanelRect(QRect(lineX - 2, arrowTipY - 2, 5, 2), lineColor, 3.6f);
            m_panel->drawPanelRect(QRect(lineX - 1, arrowTipY,     3, 2), lineColor, 3.6f);

            // 3. Horizontal Green Guide / Leader Line extending to the LHS of the CW display box
            if (boxLeftX > lineX) {
                // Box is to the right of the signal: leader line extends from lineX to boxLeftX
                m_panel->drawPanelRect(QRect(lineX, boxMidY - 1, qMax(1, boxLeftX - lineX), 2), lineColor, 3.5f);
            } else if (boxRightX < lineX) {
                // Box is to the left of the signal: leader line extends from boxRightX to lineX
                m_panel->drawPanelRect(QRect(boxRightX, boxMidY - 1, qMax(1, lineX - boxRightX), 2), lineColor, 3.5f);
            }
            // Small guide pip at the attachment point on the LHS of the box
            m_panel->drawPanelRect(QRect(boxLeftX - 3, boxMidY - 2, 4, 4), lineColor, 3.6f);
        }

        // Background container (stable dark slate)
        m_panel->drawPanelRect(m_panel->m_cwTextRect, QColor(10, 14, 20, 215), 3.4f);

        // WPM Badge (stable dark badge)
        m_panel->drawPanelRect(QRect(textX + 2, textY + 2, badgeW, badgeH - 4), QColor(28, 38, 48, 230), 3.5f);

        // Tone indicator dot (lights green on active mark tone)
        m_panel->drawPanelRect(QRect(textX + 5, textY + (badgeH / 2) - 2, 5, 5),
                      toneOn ? QColor(60, 240, 130) : QColor(90, 110, 130), 3.6f);

        m_panel->m_glTextColor = Qt::white;
        m_panel->renderPanelText(m_panel->m_oglTextSmall, float(textX + 14), float(textY + 3), 3.6f, badgeText);

        // Decoded text in high-contrast gold
        if (!trimmedText.isEmpty()) {
            m_panel->m_glTextColor = text.isEmpty() ? QColor(130, 160, 180, 190) : QColor(255, 235, 130, 255);
            m_panel->renderPanelText(m_panel->m_oglTextNormal, float(textX + badgeW + 6), float(textY + 2), 3.6f, trimmedText);
        }
    } else {
        m_panel->m_cwTextRect = QRect();
    }
}

void HudRenderer::drawRttyDecoderHUD() {
    ensureGL();
    auto clearRttyHudRects = [this]() {
        m_panel->m_rttyTextRect = QRect();
        m_panel->m_rttyConfigBtnRect = QRect();
        m_panel->m_rttyScopeRect = QRect();
        m_panel->m_rttyNudgeLeftRect = QRect();
        m_panel->m_rttyNudgeRightRect = QRect();
        m_panel->m_rttyDecodeTextRect = QRect();
        m_panel->m_rttyScrollBarRect = QRect();
        m_panel->m_rttyJumpLatestRect = QRect();
    };
    if (!m_panel->m_sliceModel || !m_panel->m_sliceModel->rttyDecodeEnabled()) {
        clearRttyHudRects();
        m_panel->m_rttyFollowLatest = true;
        m_panel->m_rttyScrollFromBottom = 0;
        m_panel->m_rttyWrappedLineCount = 0;
        return;
    }
    if (!m_panel->m_oglTextNormal || !m_panel->m_oglTextSmall) {
        return;
    }

    const QString text = m_panel->m_sliceModel->rttyDecodedText();
    const float baud = m_panel->m_sliceModel->rttyBaudRate();
    const float shift = m_panel->m_sliceModel->rttyShiftHz();
    const float centerAudioFreq = m_panel->m_sliceModel->rttyCenterFreq();
    const bool locked = m_panel->m_sliceModel->rttyToneLocked();
    const float snr = m_panel->m_sliceModel->rttySnrDb();

    m_panel->ensurePanelViewport();

    const DSPMode mode = m_panel->m_sliceModel->dspMode();
    const qint64 vfoFreq = m_panel->m_sliceModel->frequency();
    const qint64 centerFreq = m_panel->m_sliceModel->centerFrequency();

    // Amateur high tones: Mark AF = center - shift/2 (2125), Space AF = center + shift/2 (2295).
    // USB: higher AF is higher RF. LSB inverts the spectrum, so Mark RF is closer to the VFO.
    const bool isLsb = (mode == DSPMode::LSB || mode == DSPMode::DIGL || mode == DSPMode::CWL);
    const bool rev = m_panel->m_sliceModel->rttyReverse();
    const qint64 halfShift = qRound(shift * 0.5f);
    const qint64 centerRf = isLsb ? (vfoFreq - qRound(centerAudioFreq)) : (vfoFreq + qRound(centerAudioFreq));
    qint64 markRf = isLsb ? (centerRf + halfShift) : (centerRf - halfShift);
    qint64 spaceRf = isLsb ? (centerRf - halfShift) : (centerRf + halfShift);
    if (rev)
        std::swap(markRf, spaceRf);

    // Screen X positions
    const float zoomFactor = m_panel->displayedZoomFactor();
    const float sampleRate = (m_panel->m_sampleRate > 0) ? (float)m_panel->m_sampleRate : 48000.0f;
    const float markDeltaF = (float)(markRf - centerFreq) / sampleRate;
    const float spaceDeltaF = (float)(spaceRf - centerFreq) / sampleRate;
    const float nominalDeltaF = (float)(centerRf - centerFreq) / sampleRate;

    const float markX = (float)m_panel->m_panRect.left() + ((float)m_panel->m_panRect.width() / 2.0f) + (markDeltaF * (float)m_panel->m_panRect.width() / zoomFactor);
    const float spaceX = (float)m_panel->m_panRect.left() + ((float)m_panel->m_panRect.width() / 2.0f) + (spaceDeltaF * (float)m_panel->m_panRect.width() / zoomFactor);
    const float nominalX = (float)m_panel->m_panRect.left() + ((float)m_panel->m_panRect.width() / 2.0f) + (nominalDeltaF * (float)m_panel->m_panRect.width() / zoomFactor);

    const int panLeft = m_panel->m_panRect.left();
    const int panRight = m_panel->m_panRect.right();
    const int panTop = m_panel->m_panRect.top();
    const int panBottom = m_panel->m_panRect.bottom();

    // Draw Dual-Tone Tuning Crosshairs (Mark Cyan/Green, Space Orange/Amber)
    const bool markVisible = (markX >= (float)panLeft && markX <= (float)panRight);
    const bool spaceVisible = (spaceX >= (float)panLeft && spaceX <= (float)panRight);

    const QColor markColor = locked ? QColor(50, 240, 200, 220) : QColor(35, 180, 160, 150);
    const QColor spaceColor = locked ? QColor(255, 165, 50, 220) : QColor(200, 130, 40, 150);

    if (markVisible && spaceVisible) {
        const int mX = qRound(markX);
        const int sX = qRound(spaceX);

        // Tuning scope crossbar
        const int crossbarY = panBottom - 24;
        const int minX = qMin(mX, sX);
        const int maxX = qMax(mX, sX);
        m_panel->drawPanelRect(QRect(minX, crossbarY, maxX - minX, 1), QColor(160, 180, 200, 100), 3.4f);

        // Mark arrow & 'M' indicator
        m_panel->drawPanelRect(QRect(mX - 1, crossbarY - 8, 2, 8), markColor, 3.5f);
        m_panel->drawPanelRect(QRect(mX - 3, crossbarY, 7, 2), markColor, 3.5f);
        m_panel->m_glTextColor = markColor;
        m_panel->renderPanelText(m_panel->m_oglTextSmall, float(mX - 4), float(crossbarY - 18), 3.6f, QStringLiteral("M"));

        // Space arrow & 'S' indicator
        m_panel->drawPanelRect(QRect(sX - 1, crossbarY - 8, 2, 8), spaceColor, 3.5f);
        m_panel->drawPanelRect(QRect(sX - 3, crossbarY, 7, 2), spaceColor, 3.5f);
        m_panel->m_glTextColor = spaceColor;
        m_panel->renderPanelText(m_panel->m_oglTextSmall, float(sX - 4), float(crossbarY - 18), 3.6f, QStringLiteral("S"));
    }

    const QFontMetrics &fm = m_panel->m_oglTextNormal->fontMetrics();
    const int colW = qMax(6, fm.horizontalAdvance(QLatin1Char('0')));
    const int lineH = (fm.height() > 0 ? fm.height() : m_panel->m_fonts.fontHeightNormalFont) + 2;
    const int headerH = (m_panel->m_oglTextSmall ? m_panel->m_oglTextSmall->fontMetrics().height() : m_panel->m_fonts.fontHeightSmallFont) + 6;
    const int numLines = 10;
    const int scrollW = 8;
    const int scopeGap = 8;
    const int sidePad = 8;

    int cols = 80;
    int scopeSize = 96;
    int textW = colW * cols;
    int totalW = sidePad + scopeSize + scopeGap + textW + scrollW + sidePad;
    if (totalW > m_panel->m_panRect.width() - 8) {
        const int budget = m_panel->m_panRect.width() - 8 - sidePad - scopeGap - scrollW - sidePad;
        if (budget < 280) {
            scopeSize = 0;
            cols = qBound(24, (m_panel->m_panRect.width() - 8 - sidePad * 2 - scrollW) / colW, 80);
        } else {
            const int forText = budget - scopeSize;
            cols = qBound(24, forText / colW, 80);
            if (cols < 40) {
                scopeSize = 0;
                cols = qBound(24, (m_panel->m_panRect.width() - 8 - sidePad * 2 - scrollW) / colW, 80);
            }
        }
        textW = colW * cols;
        totalW = sidePad + scopeSize + (scopeSize > 0 ? scopeGap : 0) + textW + scrollW + sidePad;
    }

    if (totalW < 140 || m_panel->m_panRect.width() < 140) {
        clearRttyHudRects();
        return;
    }

    const int textBlockH = numLines * lineH;
    const int bodyH = qMax(textBlockH, scopeSize);
    const int totalH = headerH + bodyH + 10;

    int textX = qRound(nominalX) + 8;
    int textY = panTop + (m_panel->m_panRect.height() / 4);

    if (m_panel->m_hasCustomRttyBoxPos) {
        textX = m_panel->m_rttyBoxPos.x();
        textY = m_panel->m_rttyBoxPos.y();
    } else if (textX + totalW > panRight - 4) {
        textX = qRound(nominalX) - 8 - totalW;
    }

    textX = qBound(panLeft + 4, textX, qMax(panLeft + 4, panRight - totalW - 4));
    textY = qBound(panTop + 4, textY, qMax(panTop + 4, panBottom - totalH - 4));

    if (m_panel->m_hasCustomRttyBoxPos) {
        m_panel->m_rttyBoxPos = QPoint(textX, textY);
    }

    m_panel->m_rttyTextRect = QRect(textX, textY, totalW, totalH);

    // Card background (dark slate with soft transparency)
    m_panel->drawPanelRect(m_panel->m_rttyTextRect, QColor(10, 16, 22, 225), 3.4f);

    // Subtle outer border
    m_panel->drawPanelRect(QRect(textX, textY, totalW, 1), QColor(45, 65, 85, 200), 3.45f);
    m_panel->drawPanelRect(QRect(textX, textY + totalH - 1, totalW, 1), QColor(45, 65, 85, 200), 3.45f);
    m_panel->drawPanelRect(QRect(textX, textY, 1, totalH), QColor(45, 65, 85, 200), 3.45f);
    m_panel->drawPanelRect(QRect(textX + totalW - 1, textY, 1, totalH), QColor(45, 65, 85, 200), 3.45f);

    // Header divider line
    const int headerY = textY + 3;
    m_panel->drawPanelRect(QRect(textX + 4, headerY + headerH, totalW - 8, 1), QColor(40, 60, 80, 140), 3.5f);

    // Badge formatting
    const bool autoDetect = m_panel->m_sliceModel->rttyAutoDetect();
    const bool weather = m_panel->m_sliceModel->rttyWeatherProfile();
    const QString modePrefix = autoDetect ? QStringLiteral("AUTO")
        : (weather ? QStringLiteral("WX") : QStringLiteral("RTTY"));
    const QString polStr = rev ? QStringLiteral("REV") : QStringLiteral("NOR");
    QString badgeText = QStringLiteral("%1 %2/%3 [%4]").arg(modePrefix).arg(qRound(baud)).arg(qRound(shift)).arg(polStr);
    if (locked && snr > 0.0f) {
        badgeText.append(QStringLiteral(" %1dB").arg(qRound(snr)));
    }
    badgeText.append(QStringLiteral(" \u2699"));

    const int badgeTextW = m_panel->m_oglTextSmall->fontMetrics().horizontalAdvance(badgeText);
    const int badgeW = badgeTextW + 22;

    // Mode badge / config button container (highlight on hover)
    m_panel->m_rttyConfigBtnRect = QRect(textX + 4, headerY, badgeW, headerH - 2);
    const bool configHover = m_panel->m_rttyConfigBtnRect.contains(m_panel->m_mousePos);
    const QColor badgeBg = configHover ? QColor(40, 62, 88, 240) : QColor(22, 34, 46, 230);
    m_panel->drawPanelRect(m_panel->m_rttyConfigBtnRect, badgeBg, 3.5f);
    m_panel->drawPanelRect(QRect(textX + 4, headerY, badgeW, 1), QColor(60, 85, 115, 180), 3.55f);

    // Tone lock pip (green on lock)
    m_panel->drawPanelRect(QRect(textX + 8, headerY + (headerH - 2) / 2 - 2, 5, 5),
                           locked ? QColor(50, 240, 150) : QColor(90, 110, 130), 3.6f);

    m_panel->m_glTextColor = Qt::white;
    m_panel->renderPanelText(m_panel->m_oglTextSmall, float(textX + 18), float(headerY + 2), 3.6f, badgeText);

    const int bodyY = headerY + headerH + 4;
    QRect scopeRect;
    int textLeft = textX + sidePad;
    if (scopeSize > 0) {
        scopeRect = QRect(textX + 4, bodyY, scopeSize, scopeSize);
        m_panel->m_rttyScopeRect = scopeRect;
        float markHz = m_panel->m_sliceModel->rttyMarkFreq();
        if (markHz < 100.0f) {
            const float half = m_panel->m_sliceModel->rttyShiftHz() * 0.5f;
            markHz = m_panel->m_sliceModel->rttyCenterFreq() + (rev ? half : -half);
        }
        drawRttyTuningScope(scopeRect, markHz);
        textLeft = scopeRect.right() + scopeGap;
    } else {
        m_panel->m_rttyScopeRect = QRect();
        m_panel->m_rttyNudgeLeftRect = QRect();
        m_panel->m_rttyNudgeRightRect = QRect();
    }

    const QRect decodeRect(textLeft, bodyY, textW, textBlockH);
    m_panel->m_rttyDecodeTextRect = decodeRect;
    m_panel->m_rttyVisibleLines = numLines;
    m_panel->drawPanelRect(decodeRect, QColor(12, 14, 16, 240), 3.5f);

    QStringList wrappedLines;
    if (text.isEmpty()) {
        wrappedLines.append(QStringLiteral("<Listening for RTTY signals...>"));
    } else {
        wrappedLines = wrapRttyTerminal(text, cols);
        while (wrappedLines.size() > 1 && wrappedLines.last().isEmpty()
               && wrappedLines.at(wrappedLines.size() - 2).isEmpty()) {
            wrappedLines.removeLast();
        }
        if (wrappedLines.isEmpty())
            wrappedLines.append(QString());
    }

    m_panel->m_rttyWrappedLineCount = wrappedLines.size();
    const int maxScroll = qMax(0, wrappedLines.size() - numLines);
    if (m_panel->m_rttyFollowLatest)
        m_panel->m_rttyScrollFromBottom = 0;
    m_panel->m_rttyScrollFromBottom = qBound(0, m_panel->m_rttyScrollFromBottom, maxScroll);
    if (m_panel->m_rttyScrollFromBottom == 0)
        m_panel->m_rttyFollowLatest = true;

    const int first = qMax(0, wrappedLines.size() - numLines - m_panel->m_rttyScrollFromBottom);
    const QStringList displayLines = wrappedLines.mid(first, numLines);

    m_panel->m_rttyScrollBarRect = QRect(decodeRect.right() + 1, decodeRect.top(), scrollW, decodeRect.height());
    m_panel->drawPanelRect(m_panel->m_rttyScrollBarRect, QColor(18, 22, 26, 220), 3.52f);
    if (maxScroll > 0) {
        const int trackH = decodeRect.height();
        const int thumbH = qBound(12, (numLines * trackH) / wrappedLines.size(), trackH);
        const int travel = qMax(0, trackH - thumbH);
        const int thumbY = decodeRect.top() + qRound(float(maxScroll - m_panel->m_rttyScrollFromBottom) / float(maxScroll) * travel);
        m_panel->drawPanelRect(QRect(decodeRect.right() + 2, thumbY, scrollW - 2, thumbH),
                               QColor(74, 90, 110, 220), 3.55f);
    }

    for (int i = 0; i < displayLines.size(); ++i) {
        const QString &line = displayLines.at(i);
        if (line.isEmpty())
            continue;
        const int lineY = decodeRect.top() + i * lineH;
        m_panel->m_glTextColor = text.isEmpty() ? QColor(130, 160, 180, 180) : QColor(197, 209, 222, 255);
        m_panel->renderPanelText(m_panel->m_oglTextNormal, float(decodeRect.left() + 2), float(lineY), 3.6f, line);
    }

    if (!m_panel->m_rttyFollowLatest) {
        const QString jump = QStringLiteral("\u2193 latest");
        const int jw = m_panel->m_oglTextSmall->fontMetrics().horizontalAdvance(jump) + 14;
        const int jh = m_panel->m_oglTextSmall->fontMetrics().height() + 4;
        m_panel->m_rttyJumpLatestRect = QRect(decodeRect.right() - jw - 4, decodeRect.bottom() - jh - 4, jw, jh);
        const bool jumpHover = m_panel->m_rttyJumpLatestRect.contains(m_panel->m_mousePos);
        m_panel->drawPanelRect(m_panel->m_rttyJumpLatestRect,
                               jumpHover ? QColor(53, 80, 122, 240) : QColor(42, 63, 95, 230), 3.62f);
        m_panel->m_glTextColor = QColor(230, 230, 230);
        m_panel->renderPanelText(m_panel->m_oglTextSmall,
                                 float(m_panel->m_rttyJumpLatestRect.left() + 7),
                                 float(m_panel->m_rttyJumpLatestRect.top() + 2), 3.64f, jump);
    } else {
        m_panel->m_rttyJumpLatestRect = QRect();
    }
}

void HudRenderer::drawRttyTuningScope(const QRect &scopeRect, float markHz)
{
    if (scopeRect.isEmpty())
        return;

    const int x0 = scopeRect.left();
    const int y0 = scopeRect.top();
    const int size = scopeRect.width();
    const float cx = float(x0) + float(size) * 0.5f;
    const float cy = float(y0) + float(size) * 0.5f;

    m_panel->drawPanelRect(scopeRect, QColor(8, 12, 14, 255), 3.55f);
    m_panel->drawPanelRect(QRect(x0, y0, size, 1), QColor(38, 43, 48, 220), 3.56f);
    m_panel->drawPanelRect(QRect(x0, y0 + size - 1, size, 1), QColor(38, 43, 48, 220), 3.56f);
    m_panel->drawPanelRect(QRect(x0, y0, 1, size), QColor(38, 43, 48, 220), 3.56f);
    m_panel->drawPanelRect(QRect(x0 + size - 1, y0, 1, size), QColor(38, 43, 48, 220), 3.56f);

    const QMatrix4x4 proj = m_panel->panelProjection();
    const float cr = 120.0f / 255.0f, cg = 130.0f / 255.0f, cb = 140.0f / 255.0f;
    const GlDraw::Vec3Rgb cross[4] = {
        { float(x0 + 1), cy, 3.57f, cr, cg, cb }, { float(x0 + size - 1), cy, 3.57f, cr, cg, cb },
        { cx, float(y0 + 1), 3.57f, cr, cg, cb }, { cx, float(y0 + size - 1), 3.57f, cr, cg, cb },
    };
    m_panel->m_vao.bind();
    glLineWidth(1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    GlDraw::drawColoredLines(this, m_panel->m_shaderProgram, m_panel->m_vbo, proj, cross, 4);

    const QVector<float> xs = m_panel->m_sliceModel->rttyScopeXs();
    const QVector<float> ys = m_panel->m_sliceModel->rttyScopeYs();
    const int n = qMin(xs.size(), ys.size());
    if (n >= 2) {
        float frameMax = 0.0f;
        for (int i = 0; i < n; ++i) {
            frameMax = qMax(frameMax, qAbs(xs.at(i)));
            frameMax = qMax(frameMax, qAbs(ys.at(i)));
        }
        if (frameMax > m_rttyScopePeak)
            m_rttyScopePeak = frameMax;
        else
            m_rttyScopePeak = m_rttyScopePeak * 0.95f + frameMax * 0.05f;
        const float scale = (float(size) * 0.5f * 0.9f) / (m_rttyScopePeak + 1.0e-6f);

        QVector<GlDraw::Vec3Rgb> verts;
        verts.resize((n - 1) * 2);
        const float tr = 74.0f / 255.0f, tg = 222.0f / 255.0f, tb = 128.0f / 255.0f;
        int v = 0;
        float px = cx + xs.at(0) * scale;
        float py = cy - ys.at(0) * scale;
        for (int i = 1; i < n; ++i) {
            const float nx = cx + xs.at(i) * scale;
            const float ny = cy - ys.at(i) * scale;
            verts[v++] = { px, py, 3.58f, tr, tg, tb };
            verts[v++] = { nx, ny, 3.58f, tr, tg, tb };
            px = nx;
            py = ny;
        }
        GlDraw::drawColoredLines(this, m_panel->m_shaderProgram, m_panel->m_vbo, proj,
                                 verts.constData(), verts.size());
    }

    m_panel->m_glTextColor = QColor(90, 99, 108, 220);
    m_panel->renderPanelText(m_panel->m_oglTextSmall, float(x0 + 5), float(y0 + 3), 3.62f, QStringLiteral("TUNING"));

    const int btnW = 18;
    const int btnH = 14;
    const int nudgeY = y0 + size - btnH - 4;
    const int midX = x0 + size / 2;
    m_panel->m_rttyNudgeLeftRect = QRect(midX - 40, nudgeY, btnW, btnH);
    m_panel->m_rttyNudgeRightRect = QRect(midX + 22, nudgeY, btnW, btnH);

    const auto drawNudgeBtn = [&](const QRect &r, const QString &label) {
        const bool hover = r.contains(m_panel->m_mousePos);
        m_panel->drawPanelRect(r, hover ? QColor(42, 63, 95, 230) : QColor(24, 28, 31, 220), 3.6f);
        m_panel->drawPanelRect(QRect(r.left(), r.top(), r.width(), 1), QColor(58, 68, 82, 200), 3.61f);
        m_panel->m_glTextColor = hover ? QColor(230, 230, 230) : QColor(197, 209, 222);
        const int tw = m_panel->m_oglTextSmall->fontMetrics().horizontalAdvance(label);
        m_panel->renderPanelText(m_panel->m_oglTextSmall,
                                 float(r.left() + (r.width() - tw) / 2),
                                 float(r.top() + 1), 3.62f, label);
    };
    drawNudgeBtn(m_panel->m_rttyNudgeLeftRect, QStringLiteral("\u25C0"));
    drawNudgeBtn(m_panel->m_rttyNudgeRightRect, QStringLiteral("\u25B6"));

    const QString hzText = QString::number(qRound(markHz));
    m_panel->m_glTextColor = QColor(197, 209, 222);
    const int hzW = m_panel->m_oglTextSmall->fontMetrics().horizontalAdvance(hzText);
    m_panel->renderPanelText(m_panel->m_oglTextSmall,
                             float(midX - hzW / 2), float(nudgeY + 1), 3.62f, hzText);
}

void HudRenderer::drawCrossHair() {
    ensureGL();
    if (!m_panel->m_overlayRenderer) return;

    m_panel->ensurePanelViewport();

    int mouseX = m_panel->m_mousePos.x();
    int mouseY = m_panel->m_mousePos.y();
    const int textOffset = 20;
    const int spacing = 6;

    QMatrix4x4 projection;
    projection.ortho(0, m_panel->size().width(), m_panel->size().height(), 0, -10, 10);
    m_panel->m_overlayRenderer->drawCrossHair(projection, m_panel->m_panRect, m_panel->m_dBmScalePanRect, m_panel->m_mousePos,
                                     (float)m_panel->devicePixelRatioF(), m_panel->size().height());

    // text	
    QString dFstr;
    QString fstr;
    QString dBstr;

    int dx = m_panel->m_panRect.width()/2 - mouseX;
    qreal unit = m_panel->displayedFrequencySpanHz() / m_panel->m_panRect.width();
    qreal df = unit * dx;
    qreal frequency = m_panel->m_centerFrequency - df;
    
    dFstr = frequencyString(m_panel->m_deltaFrequency - df, true);
    fstr = frequencyString(frequency);

    qreal dBm = glPixelTodBm(m_panel->m_panRect, m_panel->m_dBmPanMax, m_panel->m_dBmPanMin, mouseY);
    dBstr = QString::number(dBm, 'f', 1) + " dBm";

    int rectWidth;
    int fontHeight;
    if (m_panel->m_smallSize) {
        rectWidth = m_panel->m_fonts.smallFontMetrics->boundingRect(fstr).width();
        fontHeight = m_panel->m_fonts.smallFontMetrics->tightBoundingRect("0").height() + spacing;
    } else {
        rectWidth = m_panel->m_fonts.bigFont1Metrics->horizontalAdvance(fstr);
        fontHeight = m_panel->m_fonts.bigFont1Metrics->tightBoundingRect("0").height() + spacing;
    }

    m_panel->m_haircrossMaxRight = rectWidth + textOffset;
    m_panel->m_smallSize ? m_panel->m_haircrossMinTop = 40 : m_panel->m_haircrossMinTop = 60;

    int tx, ty;
    if (mouseX > m_panel->m_panRect.width() - m_panel->m_haircrossMaxRight) {
        tx = mouseX - m_panel->m_haircrossMaxRight;
        if (mouseY > m_panel->m_haircrossMinTop)
            ty = m_panel->m_smallSize ? mouseY - 42 : mouseY - 62;
        else
            ty = m_panel->m_smallSize ? mouseY + 10 : mouseY + 30;
    } else {
        tx = mouseX + textOffset;
        if (mouseY > m_panel->m_haircrossMinTop)
            ty = m_panel->m_smallSize ? mouseY - 42 : mouseY - 62;
        else
            ty = m_panel->m_smallSize ? mouseY + 10 : mouseY + 30;
    }

    m_panel->m_glTextColor = QColor(200, 200, 200, 255);
    if (m_panel->m_smallSize) {
        m_panel->renderPanelText(m_panel->m_oglTextSmall, tx, ty, 5.0f, dFstr);
        m_panel->renderPanelText(m_panel->m_oglTextSmall, tx, ty + fontHeight, 5.0f, fstr);
    } else {
        m_panel->renderPanelText(m_panel->m_oglTextBig1, tx, ty, 5.0f, dFstr);
        m_panel->renderPanelText(m_panel->m_oglTextBig1, tx, ty + fontHeight, 5.0f, fstr);
    }

    if (m_panel->m_mouseRegion == QGLReceiverPanel::panadapterRegion) {
        if (m_panel->m_smallSize)
            m_panel->renderPanelText(m_panel->m_oglTextSmall, tx, ty + 2 * fontHeight, 5.0f, dBstr);
        else
            m_panel->renderPanelText(m_panel->m_oglTextBig1, tx, ty + 2 * fontHeight, 5.0f, dBstr);
    }

    if (m_panel->m_oldMousePosX != mouseX) {
        m_panel->m_bandText = getHamBandTextString(m_panel->set->getHamBandTextList(), true, frequency);
        m_panel->m_oldMousePosX = mouseX;
    }

    m_panel->m_glTextColor = QColor(239, 209, 110, 255);
    if (m_panel->m_smallSize)
        m_panel->renderPanelText(m_panel->m_oglTextSmall, tx, ty + 4 * fontHeight, 5.0f, m_panel->m_bandText);
    else
        m_panel->renderPanelText(m_panel->m_oglTextBig1, tx, ty + 5 * fontHeight, 5.0f, m_panel->m_bandText);
}

void HudRenderer::drawFilterLabels() {
    m_panel->ensurePanelViewport();
    // Re-render text using the original logic which is already texture-based
    if (m_panel->m_showFilterLeftBoundary) {
		QString str1 = QString("Filter Lo");
		QString str2 = frequencyString(m_panel->m_filterLowerFrequency, true);
		m_panel->m_glTextColor = QColor(0, 0, 0, 255);
		if (m_panel->m_smallSize) {
			m_panel->renderPanelText(m_panel->m_oglTextSmall, m_panel->m_filterLeft + 5, m_panel->m_filterTop + 44, 4.0f, str1);
			m_panel->renderPanelText(m_panel->m_oglTextSmall, m_panel->m_filterLeft + 5, m_panel->m_filterTop + 64, 4.0f, str2);
		} else {
			m_panel->renderPanelText(m_panel->m_oglTextBig1, m_panel->m_filterLeft + 5, m_panel->m_filterTop + 44, 4.0f, str1);
			m_panel->renderPanelText(m_panel->m_oglTextBig1, m_panel->m_filterLeft + 5, m_panel->m_filterTop + 64, 4.0f, str2);
		}
		m_panel->m_glTextColor = QColor(255, 255, 255, 255);
		if (m_panel->m_smallSize) {
			m_panel->renderPanelText(m_panel->m_oglTextSmall, m_panel->m_filterLeft + 3, m_panel->m_filterTop + 42, 5.0f, str1);
			m_panel->renderPanelText(m_panel->m_oglTextSmall, m_panel->m_filterLeft + 3, m_panel->m_filterTop + 62, 5.0f, str2);
		} else {
			m_panel->renderPanelText(m_panel->m_oglTextBig1, m_panel->m_filterLeft + 3, m_panel->m_filterTop + 42, 5.0f, str1);
			m_panel->renderPanelText(m_panel->m_oglTextBig1, m_panel->m_filterLeft + 3, m_panel->m_filterTop + 62, 5.0f, str2);
		}
    }
    if (m_panel->m_showFilterRightBoundary) {
		QString str1 = QString("Filter Hi");
		QString str2 = frequencyString(m_panel->m_filterUpperFrequency, true);
		m_panel->m_glTextColor = QColor(0, 0, 0, 255);
		if (m_panel->m_smallSize) {
			m_panel->renderPanelText(m_panel->m_oglTextSmall, m_panel->m_filterRight + 5, m_panel->m_filterTop + 44, 4.0f, str1);
			m_panel->renderPanelText(m_panel->m_oglTextSmall, m_panel->m_filterRight + 5, m_panel->m_filterTop + 64, 4.0f, str2);
		} else {
			m_panel->renderPanelText(m_panel->m_oglTextBig1, m_panel->m_filterRight + 5, m_panel->m_filterTop + 44, 4.0f, str1);
			m_panel->renderPanelText(m_panel->m_oglTextBig1, m_panel->m_filterRight + 5, m_panel->m_filterTop + 64, 4.0f, str2);
		}
		m_panel->m_glTextColor = QColor(255, 255, 255, 255);
		if (m_panel->m_smallSize) {
			m_panel->renderPanelText(m_panel->m_oglTextSmall, m_panel->m_filterRight + 3, m_panel->m_filterTop + 42, 5.0f, str1);
			m_panel->renderPanelText(m_panel->m_oglTextSmall, m_panel->m_filterRight + 3, m_panel->m_filterTop + 62, 5.0f, str2);
		} else {
			m_panel->renderPanelText(m_panel->m_oglTextBig1, m_panel->m_filterRight + 3, m_panel->m_filterTop + 42, 5.0f, str1);
			m_panel->renderPanelText(m_panel->m_oglTextBig1, m_panel->m_filterRight + 3, m_panel->m_filterTop + 62, 5.0f, str2);
		}
    }
}

void HudRenderer::drawAGCLabels() {
        if (m_panel->m_agcMode == (AGCMode) agcOFF) {
            QString str = "AGC-F";
            m_panel->m_glTextColor = QColor(0, 0, 0, 255);
            m_panel->renderPanelText(m_panel->m_oglTextSmall, m_panel->m_panRect.right() - 32, m_panel->m_agcFixedGainLevelPixel - 13, 4.0f, str);
            m_panel->m_glTextColor = QColor(225, 125, 225, 255);
            m_panel->renderPanelText(m_panel->m_oglTextSmall, m_panel->m_panRect.right() - 34, m_panel->m_agcFixedGainLevelPixel - 15, 5.0f, str);
        } else {
            QString str = "AGC-T";
            m_panel->m_glTextColor = QColor(0, 0, 0, 255);
            m_panel->renderPanelText(m_panel->m_oglTextSmall, m_panel->m_panRect.right() - 32, m_panel->m_agcThresholdPixel - 13, 4.0f, str);
            m_panel->m_glTextColor = QColor(225, 125, 125, 255);
            m_panel->renderPanelText(m_panel->m_oglTextSmall, m_panel->m_panRect.right() - 34, m_panel->m_agcThresholdPixel - 15, 5.0f, str);
            if (m_panel->m_agcHangEnabled) {
                str = "AGC-H";
                m_panel->m_glTextColor = QColor(0, 0, 0, 255);
                m_panel->renderPanelText(m_panel->m_oglTextSmall, m_panel->m_panRect.right() - 32, m_panel->m_agcHangLevelPixel - 13, 4.0f, str);
                m_panel->m_glTextColor = QColor(125, 225, 125, 255);
                m_panel->renderPanelText(m_panel->m_oglTextSmall, m_panel->m_panRect.right() - 34, m_panel->m_agcHangLevelPixel - 15, 5.0f, str);
            }
        }
}
