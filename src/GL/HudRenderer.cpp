#include "HudRenderer.h"
#include "cusdr_oglReceiverPanel.h"
#include "cusdr_oglUtils.h"
#include "cusdr_hamDatabase.h"
#include "OverlayRenderer.h"
#include "Models/SliceModel.h"
#include "cusdr_glDraw.h"

#include <QMatrix4x4>
#include <QVector>
#include <cmath>

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
	int x1 = (m_panel->m_panFreqRect.isValid())
	             ? (m_panel->m_panFreqRect.right() + 8)
	             : (m_panel->m_panSMeterRect.isValid())
	                 ? (m_panel->m_panSMeterRect.right() + 8)
	                 : (m_panel->m_dBmScalePanRect.right() + 5);
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

void HudRenderer::drawReceiverInfo() {
    ensureGL();

	QString str;
    const int badgeH = m_panel->m_fonts.fontHeightSmallFont + 2;

    auto drawRxBadge = [&](int &x, int y, const QString &label, const QColor &bg) {
        if (label.isEmpty())
            return;
        const int w = m_panel->m_oglTextSmall->fontMetrics().horizontalAdvance(label) + 3;
        m_panel->drawPanelRect(QRect(x, y, w, badgeH), bg, 2.0f);
        m_panel->m_glTextColor = QColor(0, 0, 0, 255);
        m_panel->renderPanelText(m_panel->m_oglTextSmall, x + 1, y + 1, 2.0f, label);
        x += w + 4;
    };
	// mouse wheel freq step size
	/*if (m_panel->m_dataEngineState == QSDR::DataEngineUp) {

		if (m_panel->m_receiver == m_panel->m_currentReceiver)
			col = QColor(1, 190, 180, 180);
		else
			col = QColor(1, 100, 90, 180);
	}
	else
		col = m_panel->m_darkColor;

	str = "%1";
	str = str.arg(m_panel->set->getValue1000(m_panel->m_mouseWheelFreqStep, 0, "Hz"));

	int x1 = m_panel->m_panRect.width() - (m_panel->m_fonts.smallFontMetrics->tightBoundingRect(str).width() + 9);
	int y1 = 3;

	rect = QRect(x1+2, y1, m_panel->m_fonts.smallFontMetrics->tightBoundingRect(str).width() + 5, m_panel->m_fonts.fontHeightSmallFont + 2);
	m_panel->drawPanelRect(rect, col, 2.0f);
	qglColor(QColor(0, 0, 0));
	m_panel->m_oglTextSmall->renderFreqText(x1+3, y1-2, 3.0f, str);*/


	// AGC mode
	//if (m_panel->m_dataEngineState == QSDR::DataEngineUp) {

	//	if (m_panel->m_receiver == m_panel->m_currentReceiver) {

	//		if (m_panel->m_showAGCLines)
	//			col = QColor(255, 170, 90, 180);
	//		else
	//			col = QColor(215, 130, 50, 180);
	//	}
	//	else
	//		col = QColor(165, 80, 1);
	//}
	//else
	//	col = m_panel->m_darkColor;

	//str = "%1";
	////str = str.arg(m_panel->set->getAGCModeString(m_panel->m_receiver));
	//str = str.arg(m_panel->m_agcModeString);

	//x1 -= m_panel->m_fonts.smallFontMetrics->tightBoundingRect(str).width() + 7;
	//y1 = 3;

	//m_panel->m_agcButtonRect = QRect(x1+2, y1, m_panel->m_fonts.smallFontMetrics->tightBoundingRect(str).width() + 5, m_panel->m_fonts.fontHeightSmallFont + 2);
	//m_panel->drawPanelRect(m_panel->m_agcButtonRect, col, 2.0f);
	//qglColor(QColor(0, 0, 0));
	//m_panel->m_oglTextSmall->renderFreqText(x1+3, y1-2, 3.0f, str);


    // main frequency display
    glDisable(GL_MULTISAMPLE);
    if (m_panel->m_panRect.height() > 15) {

        QColor colFlt;

        if (m_panel->m_dataEngineState == QSDR::DataEngineUp) {

            if (m_panel->m_receiver == m_panel->set->getCurrentReceiver()) {

                colFlt = QColor(200, 190, 50, 180);
            }
            else {

                colFlt = QColor(110, 100, 1, 180);
            }
        }
        else {

            colFlt = m_panel->m_darkColor;
        }

        const int vfoX = m_panel->m_panRect.left() + qRound((qreal)(m_panel->m_panRect.width() / 2.0f) - m_panel->m_deltaF * m_panel->m_panRect.width() / m_panel->displayedZoomFactor());
        const int centerX = m_panel->m_panRect.left() + m_panel->m_panRect.width() / 2;

        const int f1 = (int)(m_panel->m_vfoFrequency / 1000);
        const int f2 = (int)(m_panel->m_vfoFrequency % 1000);
        const QString vfoText = (f2 == 0)
            ? QStringLiteral("%1.%2 MHz").arg(f1 / 1000).arg(f1 - 1000 * (int)(f1 / 1000), 3, 10, QLatin1Char('0'))
            : QStringLiteral("%1.%2.%3 MHz").arg(f1 / 1000).arg(f1 - 1000 * (int)(f1 / 1000), 3, 10, QLatin1Char('0')).arg(f2, 3, 10, QLatin1Char('0'));
        const int vfoTextWidth = m_panel->m_oglTextBig2->fontMetrics().horizontalAdvance(vfoText);

        int x = vfoX + 10;
        if (x > m_panel->m_panRect.right() - vfoTextWidth - 10)
            x = vfoX - vfoTextWidth - 10;
        else if (m_panel->m_deltaFrequency != 0 && vfoX < centerX && x + vfoTextWidth >= centerX - 5)
            x = vfoX - vfoTextWidth - 10;

        int x1 = x;
        const int y1 = 3;
        drawRxBadge(x1, y1, m_panel->m_filterWidthString, colFlt);

        const int freqY = y1 + badgeH + 3;
        m_panel->m_glTextColor = QColor(255, 255, 255, 255);
        m_panel->renderPanelText(m_panel->m_oglTextBig2, x, freqY, vfoText);

        if (m_panel->m_panRect.height() > 15 && m_panel->m_deltaFrequency != 0) {

            const int cf1 = (int)(m_panel->m_centerFrequency / 1000);
            const int cf2 = (int)(m_panel->m_centerFrequency % 1000);
            const QString centerText = (cf2 == 0)
                ? QStringLiteral("%1.%2 MHz").arg(cf1 / 1000).arg(cf1 - 1000 * (int)(cf1 / 1000), 3, 10, QLatin1Char('0'))
                : QStringLiteral("%1.%2.%3 MHz").arg(cf1 / 1000).arg(cf1 - 1000 * (int)(cf1 / 1000), 3, 10, QLatin1Char('0')).arg(cf2, 3, 10, QLatin1Char('0'));
            const int centerWidth = m_panel->m_oglTextBig2->fontMetrics().horizontalAdvance(centerText);

            int cx = centerX + 10;
            if (vfoX >= centerX && vfoX < centerX + centerWidth + 20)
                cx = centerX - centerWidth - 10;
            else if (cx + centerWidth > m_panel->m_panRect.right() - 5)
                cx = centerX - centerWidth - 10;

            m_panel->m_glTextColor = QColor(80, 180, 240, 220);
            m_panel->renderPanelText(m_panel->m_oglTextBig2, cx, freqY, centerText);
        }

    }
}

void HudRenderer::drawPanadapterSMeter() {
    ensureGL();
    if (!m_panel || !m_panel->set || !m_panel->set->getShowPanadapterSMeter()) {
        m_panel->m_panSMeterRect = QRect();
        return;
    }
    if (m_panel->m_panRect.width() < 320 || m_panel->m_panRect.height() < 80) {
        m_panel->m_panSMeterRect = QRect();
        return;
    }

    const int cardW = 200;
    const int cardH = 38;
    const int x0 = m_panel->m_dBmScalePanRect.right() + 8;
    const int y0 = m_panel->m_panRect.top() + 6;
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
    const QString rxBadge = QStringLiteral("RX%1").arg(m_panel->m_receiver + 1);
    const int badgeW = m_panel->m_oglTextTiny->fontMetrics().horizontalAdvance(rxBadge) + 6;
    m_panel->drawPanelRect(QRect(x0 + 4, y0 + 2, badgeW, 11), QColor(28, 38, 50, 230), 3.1f);
    m_panel->m_glTextColor = QColor(180, 205, 225);
    m_panel->renderPanelText(m_panel->m_oglTextTiny, float(x0 + 7), float(y0 + 2), 3.2f, rxBadge);

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
    m_panel->renderPanelText(m_panel->m_oglTextTiny, float(x0 + badgeW + 10), float(y0 + 2), 3.2f, sUnitStr);

    const QString dbmStr = QString::asprintf("%.1f dBm", rawDbm);
    const int dbmW = m_panel->m_oglTextTiny->fontMetrics().horizontalAdvance(dbmStr);
    m_panel->m_glTextColor = QColor(210, 220, 230);
    m_panel->renderPanelText(m_panel->m_oglTextTiny, float(x0 + cardW - dbmW - 6), float(y0 + 2), 3.2f, dbmStr);

    // 3. Scale Geometry: -140 dBm to 0 dBm (140 dB span)
    const int xStart = x0 + 7;
    const int xEnd = x0 + cardW - 7;
    const int scaleW = xEnd - xStart;
    const float unit = float(scaleW) / 140.0f;
    const float yRailTop = float(y0 + 16);
    const float yRailBottom = float(y0 + 23);

    QVector<GlDraw::Vec3Rgb> scaleLines;
    scaleLines.reserve(10 + scaleW * 2 + 40);

    // Dual horizontal rails
    const QColor railCol(110, 140, 155, 220);
    const float rr = railCol.redF(), rg = railCol.greenF(), rb = railCol.blueF();
    scaleLines.append({ float(xStart), yRailTop, 3.2f, rr, rg, rb });
    scaleLines.append({ float(xEnd), yRailTop, 3.2f, rr, rg, rb });
    scaleLines.append({ float(xStart), yRailBottom, 3.2f, rr, rg, rb });
    scaleLines.append({ float(xEnd), yRailBottom, 3.2f, rr, rg, rb });

    // Vertical ladder grille every 2 px
    const QColor grilleCol(60, 80, 95, 180);
    const float gr = grilleCol.redF(), gg = grilleCol.greenF(), gb = grilleCol.blueF();
    for (int x = xStart + 2; x < xEnd; x += 2) {
        scaleLines.append({ float(x), yRailTop + 1.5f, 3.1f, gr, gg, gb });
        scaleLines.append({ float(x), yRailBottom - 1.5f, 3.1f, gr, gg, gb });
    }

    // Top dBm ticks (-120 dBm to 0 dBm)
    const QColor tickCol(160, 180, 195, 230);
    const float tr = tickCol.redF(), tg = tickCol.greenF(), tb = tickCol.blueF();
    for (int db = 20; db <= 140; db += 20) {
        const float xt = float(xStart) + float(db) * unit;
        scaleLines.append({ xt, yRailTop - 3.0f, 3.2f, tr, tg, tb });
        scaleLines.append({ xt, yRailTop, 3.2f, tr, tg, tb });
    }
    for (int db = 10; db < 140; db += 20) {
        const float xt = float(xStart) + float(db) * unit;
        scaleLines.append({ xt, yRailTop - 1.5f, 3.2f, tr, tg, tb });
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
        const float tickLen = mark.major ? 3.0f : 1.5f;
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

        // Main signal needle (bright white line)
        const float xNeedle = float(xStart) + avgVal * unit;
        needleLines.append({ xNeedle, yRailTop - 2.0f, 3.5f, 1.0f, 1.0f, 1.0f });
        needleLines.append({ xNeedle, yRailBottom + 2.0f, 3.5f, 1.0f, 1.0f, 1.0f });

        // Peak hold needle (amber/red pip at top)
        const float peakVal = qBound(0.0f, m_panel->m_sMeterHoldMax, 140.0f);
        if (peakVal > avgVal + 0.5f) {
            const float xPeak = float(xStart) + peakVal * unit;
            needleLines.append({ xPeak, yRailTop - 2.0f, 3.5f, 1.0f, 0.4f, 0.4f });
            needleLines.append({ xPeak, yRailTop + 4.0f, 3.5f, 1.0f, 0.4f, 0.4f });
        }

        m_panel->m_vao.bind();
        glLineWidth(1.5f);
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

    const QFontMetrics fm = m_panel->m_oglTextTiny->fontMetrics();
    for (const auto &lbl : sLabels) {
        const QString markStr = QString::fromLatin1(lbl.txt);
        const int tw = fm.horizontalAdvance(markStr);
        const float xl = float(xStart) + float(lbl.db) * unit - float(tw) / 2.0f;
        m_panel->m_glTextColor = (m_panel->m_dataEngineState == QSDR::DataEngineUp)
                                     ? lbl.col
                                     : QColor(120, 130, 140);
        m_panel->renderPanelText(m_panel->m_oglTextTiny, xl, float(y0 + 26), 3.4f, markStr);
    }
}

void HudRenderer::drawPanadapterFreq() {
    ensureGL();
    if (!m_panel || !m_panel->set || !m_panel->set->getShowPanadapterFreq()) {
        m_panel->m_panFreqRect = QRect();
        m_panel->m_panFreqVfoRect = QRect();
        return;
    }

    const bool sMeterVisible = m_panel->m_panSMeterRect.isValid();
    const int minPanWidth = sMeterVisible ? 440 : 250;
    if (m_panel->m_panRect.width() < minPanWidth || m_panel->m_panRect.height() < 80) {
        m_panel->m_panFreqRect = QRect();
        m_panel->m_panFreqVfoRect = QRect();
        return;
    }

    const int cardW = 180;
    const int cardH = 38;
    const int x0 = sMeterVisible ? (m_panel->m_panSMeterRect.right() + 8)
                                 : (m_panel->m_dBmScalePanRect.right() + 8);
    const int y0 = m_panel->m_panRect.top() + 6;
    m_panel->m_panFreqRect = QRect(x0, y0, cardW, cardH);

    const QMatrix4x4 proj = m_panel->panelProjection();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    // 1. Background Card & Outline Border
    m_panel->drawPanelRect(m_panel->m_panFreqRect, QColor(14, 18, 24, 220), 3.0f);

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

    // 2. Top Meta Row: VFO chip, DSP mode badge, TX indicator, Band tag
    SliceModel *slice = m_panel->m_sliceModel;
    const bool isVfoB = (slice && slice->activeVfo() == SliceModel::VfoB);
    const QString vfoLetter = isVfoB ? QStringLiteral("B") : QStringLiteral("A");
    const int vfoBadgeW = 16;
    m_panel->m_panFreqVfoRect = QRect(x0 + 4, y0 + 3, vfoBadgeW, 11);
    const QColor vfoBg = (m_panel->m_dataEngineState == QSDR::DataEngineUp)
                             ? QColor(31, 111, 235)
                             : QColor(38, 38, 38);
    m_panel->drawPanelRect(m_panel->m_panFreqVfoRect, vfoBg, 3.1f);
    m_panel->m_glTextColor = (m_panel->m_dataEngineState == QSDR::DataEngineUp)
                                 ? QColor(255, 255, 255)
                                 : QColor(140, 150, 160);
    const int vfoLetterW = m_panel->m_oglTextTiny->fontMetrics().horizontalAdvance(vfoLetter);
    const float vfoLetterX = float(x0 + 4 + (vfoBadgeW - vfoLetterW) / 2);
    m_panel->renderPanelText(m_panel->m_oglTextTiny, vfoLetterX, float(y0 + 2), 3.2f, vfoLetter);

    // Mode Badge
    const QString modeStr = m_panel->set->getDSPModeString(m_panel->set->getDSPMode(m_panel->m_receiver));
    const int modeTextW = m_panel->m_oglTextTiny->fontMetrics().horizontalAdvance(modeStr);
    const int modeBadgeW = modeTextW + 6;
    const int modeBadgeX = x0 + 4 + vfoBadgeW + 4;
    m_panel->drawPanelRect(QRect(modeBadgeX, y0 + 3, modeBadgeW, 11), QColor(28, 38, 50, 230), 3.1f);
    m_panel->m_glTextColor = (m_panel->m_dataEngineState == QSDR::DataEngineUp)
                                 ? QColor(180, 205, 225)
                                 : QColor(120, 130, 140);
    m_panel->renderPanelText(m_panel->m_oglTextTiny, float(modeBadgeX + 3), float(y0 + 2), 3.2f, modeStr);

    // TX indicator badge
    const bool isTx = (m_panel->set->getRadioState() != RadioState::RX);
    int nextBadgeX = modeBadgeX + modeBadgeW + 4;
    if (isTx) {
        const QString txStr = QStringLiteral("TX");
        const int txTextW = m_panel->m_oglTextTiny->fontMetrics().horizontalAdvance(txStr);
        const int txBadgeW = txTextW + 6;
        m_panel->drawPanelRect(QRect(nextBadgeX, y0 + 3, txBadgeW, 11), QColor(220, 40, 40, 230), 3.1f);
        m_panel->m_glTextColor = QColor(255, 255, 255);
        m_panel->renderPanelText(m_panel->m_oglTextTiny, float(nextBadgeX + 3), float(y0 + 2), 3.2f, txStr);
        nextBadgeX += txBadgeW + 4;
    }

    // Band Tag (right-aligned on top row)
    const qint64 activeFreq = m_panel->m_vfoFrequency;
    QString bandTag;
    if (m_panel->set) {
        bandTag = getHamBandTextString(m_panel->set->getHamBandTextList(), true, activeFreq);
    }
    if (!bandTag.isEmpty() && bandTag != QStringLiteral("Out of Band")) {
        const int bandTextW = m_panel->m_oglTextTiny->fontMetrics().horizontalAdvance(bandTag);
        const float bandX = float(x0 + cardW - bandTextW - 6);
        m_panel->m_glTextColor = (m_panel->m_dataEngineState == QSDR::DataEngineUp)
                                     ? QColor(239, 209, 110)
                                     : QColor(130, 120, 90);
        m_panel->renderPanelText(m_panel->m_oglTextTiny, bandX, float(y0 + 2), 3.2f, bandTag);
    }

    // 3. Main Frequency Row
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

    const float yBaseline = float(y0 + cardH - 6);
    const float yFreq = yBaseline - float(m_panel->m_oglTextBig2->fontMetrics().ascent());
    const float xFreq = float(x0 + 6);

    const QColor digitCol = isTx ? QColor(255, 65, 65)
                                : (m_panel->m_dataEngineState == QSDR::DataEngineUp)
                                      ? QColor(255, 255, 255)
                                      : QColor(130, 140, 150);
    m_panel->m_glTextColor = digitCol;
    m_panel->renderPanelText(m_panel->m_oglTextBig2, xFreq, yFreq, 3.2f, freqStr);

    const int freqW = m_panel->m_oglTextBig2->fontMetrics().horizontalAdvance(freqStr);
    const float xUnit = xFreq + float(freqW) + 4.0f;
    const float yUnit = yBaseline - float(m_panel->m_oglTextSmall->fontMetrics().ascent());
    m_panel->m_glTextColor = (m_panel->m_dataEngineState == QSDR::DataEngineUp)
                                 ? QColor(130, 155, 175)
                                 : QColor(100, 110, 120);
    m_panel->renderPanelText(m_panel->m_oglTextSmall, xUnit, yUnit, 3.2f, QStringLiteral("MHz"));

    // Mouse wheel step indicator (right-aligned on main row)
    const QString stepStr = m_panel->set->getValue1000(m_panel->set->getMouseWheelFreqStep(m_panel->m_receiver), 0, "Hz");
    if (!stepStr.isEmpty()) {
        const int stepW = m_panel->m_oglTextTiny->fontMetrics().horizontalAdvance(stepStr);
        const float xStep = float(x0 + cardW - stepW - 6);
        const float yStep = yBaseline - float(m_panel->m_oglTextTiny->fontMetrics().ascent());
        m_panel->m_glTextColor = (m_panel->m_dataEngineState == QSDR::DataEngineUp)
                                     ? QColor(110, 135, 155)
                                     : QColor(90, 100, 110);
        m_panel->renderPanelText(m_panel->m_oglTextTiny, xStep, yStep, 3.2f, stepStr);
    }
}

void HudRenderer::drawCwDecoderHUD() {
    ensureGL();
    const DSPMode mode = m_panel->m_sliceModel ? m_panel->m_sliceModel->dspMode() : m_panel->m_dspMode;
    const DSPMode setMode = static_cast<DSPMode>(m_panel->set->getDSPMode(m_panel->m_receiver));
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
