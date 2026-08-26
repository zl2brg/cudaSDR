#include "TraceRenderer.h"
#include "cusdr_oglReceiverPanel.h"
#include "WaterfallRenderer.h"
#include "PanadapterRenderer.h"
#include "Models/RadioModel.h"
#include "Models/BandPlanManager.h"

#include <QDateTime>
#include <QMatrix4x4>

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

TraceRenderer::TraceRenderer(QGLReceiverPanel *panel)
    : m_panel(panel)
    , m_glReady(false)
{
}

TraceRenderer::~TraceRenderer()
{
}

void TraceRenderer::ensureGL()
{
    if (!m_glReady) {
        initializeOpenGLFunctions();
        m_glReady = true;
    }
}

void TraceRenderer::drawPanadapter() {
    ensureGL();

    const bool showSpectrum = (m_panel->m_dataEngineState == QSDR::DataEngineUp && !m_panel->m_panadapterBins.isEmpty());

    if (!showSpectrum) {
        if (m_panel->m_dataEngineState != QSDR::DataEngineUp)
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (m_panel->m_panadapterRenderer && m_panel->m_panRect.isValid()) {
            float dpr = (float)m_panel->devicePixelRatio();
            QMatrix4x4 projection;
            projection.ortho(0, m_panel->size().width(), m_panel->size().height(), 0, -10, 10);
            PanadapterRenderer::Colors colors = { m_panel->m_red, m_panel->m_green, m_panel->m_blue, m_panel->m_redF, m_panel->m_greenF, m_panel->m_blueF, m_panel->m_redST, m_panel->m_greenST, m_panel->m_blueST, m_panel->m_redSB, m_panel->m_greenSB, m_panel->m_blueSB, m_panel->m_bkgRed, m_panel->m_bkgGreen, m_panel->m_bkgBlue };
            m_panel->m_panadapterRenderer->renderIdleBackground(m_panel, projection, m_panel->m_panRect, dpr, m_panel->size().height(), colors, m_panel->m_dataEngineState, (m_panel->m_receiver == m_panel->m_currentReceiver));
        }
        return;
    }

    if (m_panel->m_dataEngineState == QSDR::DataEngineUp)
        glClear(GL_DEPTH_BUFFER_BIT);
    else
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_LINE_SMOOTH);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glLineWidth(1);

    if (m_panel->m_panadapterRenderer) {
        float dpr = (float)m_panel->devicePixelRatio();
        QMatrix4x4 projection;
        projection.ortho(0, m_panel->size().width(), m_panel->size().height(), 0, -10, 10);
        PanadapterRenderer::Colors colors = { m_panel->m_red, m_panel->m_green, m_panel->m_blue, m_panel->m_redF, m_panel->m_greenF, m_panel->m_blueF, m_panel->m_redST, m_panel->m_greenST, m_panel->m_blueST, m_panel->m_redSB, m_panel->m_greenSB, m_panel->m_blueSB, m_panel->m_bkgRed, m_panel->m_bkgGreen, m_panel->m_bkgBlue };
        const QVector<qreal> emptyPeakHold;
        const QVector<qreal>& peakHoldBins =
            (m_panel->m_peakHold && m_panel->m_panPeakHoldBins.size() == m_panel->m_panadapterBins.size())
                ? m_panel->m_panPeakHoldBins
                : emptyPeakHold;
        m_panel->m_panadapterRenderer->render(m_panel, projection, m_panel->m_panRect, m_panel->m_panadapterBins, m_panel->m_dBmPanMax, m_panel->m_dBmPanMin,
                                   m_panel->m_panMode, m_panel->m_scaleMult, dpr, m_panel->size().height(), colors, m_panel->m_dataEngineState,
                                   (m_panel->m_receiver == m_panel->m_currentReceiver), peakHoldBins);
        if (m_panel->m_panadapterRenderer->usesCompositePass())
            m_panel->m_panadapterRenderer->compositeToDefaultFramebuffer(m_panel, projection, m_panel->m_panRect, dpr, m_panel->size().height());
    }

    glEnable(GL_DEPTH_TEST);
}

void TraceRenderer::drawWaterfall() {
    ensureGL();
    if (!m_panel->m_waterfallRenderer || m_panel->m_waterfallRect.isEmpty())
        return;

    const float dpr = (float)m_panel->devicePixelRatioF();
    const int x1 = m_panel->m_waterfallRect.left();
    const int y1 = m_panel->m_waterfallRect.top();
    const int x2 = x1 + m_panel->m_waterfallRect.width();
    const int y2 = y1 + m_panel->m_waterfallRect.height();
    const int panelHeight = m_panel->size().height();

    glScissor(int(x1 * dpr), int((panelHeight - y2) * dpr),
              int((x2 - x1) * dpr), int(m_panel->m_waterfallRect.height() * dpr));
    glEnable(GL_SCISSOR_TEST);
    glDisable(GL_MULTISAMPLE);

    glDisable(GL_DEPTH_TEST);
    WaterfallMapping mapping;
    mapping.lowerThreshold = float(m_panel->m_dBmPanMin) - float(m_panel->m_waterfallOffsetLo);
    mapping.upperThreshold = float(m_panel->m_dBmPanMax) + float(m_panel->m_waterfallOffsetHi);
    mapping.colorRange = float(qAbs(m_panel->m_dBmPanMax - m_panel->m_dBmPanMin));
    mapping.mode = m_panel->m_waterfallMode;
    mapping.lo = m_panel->m_waterfallLoColor;
    mapping.mid = m_panel->m_waterfallMidColor;
    mapping.hi = m_panel->m_waterfallHiColor;
    mapping.alpha = float(m_panel->m_waterfallAlpha) / 255.0f;
    m_panel->m_waterfallRenderer->render(m_panel->panelProjection(), m_panel->m_waterfallRect, m_panel->m_waterfallPixel,
                                m_panel->m_dataEngineState, dpr, m_panel->m_waterfallDisplayUpdate, mapping);

    glDisable(GL_SCISSOR_TEST);
}

void TraceRenderer::drawBandPlanStrip()
{
    ensureGL();
	RadioModel *radioModel = qobject_cast<RadioModel *>(m_panel->m_sliceModel ? m_panel->m_sliceModel->parent() : nullptr);
	BandPlanManager *plan = radioModel ? radioModel->bandPlan() : nullptr;
	if (!plan || plan->isEmpty())
		return;

	const qreal span = m_panel->displayedFrequencySpanHz();
	if (span <= 0.0 || !m_panel->m_panRect.isValid())
		return;

	m_panel->ensurePanelViewport();

	const qint64 loHz = qint64(qreal(m_panel->m_centerFrequency) - span / 2.0);
	const qint64 hiHz = qint64(qreal(m_panel->m_centerFrequency) + span / 2.0);

	const QDateTime nowUtc = QDateTime::currentDateTimeUtc();
	const int utcMinOfDay = nowUtc.time().hour() * 60 + nowUtc.time().minute();
	const int dayOfWeek = nowUtc.date().dayOfWeek();

	QVector<BandSpot> spots = plan->spotsInSpan(loHz, hiHz, utcMinOfDay, dayOfWeek);
	if (spots.isEmpty())
		return;

	// When zoomed way QGLReceiverPanel::out, thin labels to digimode / time / beacon markers.
	if (span > 1.5e6 && spots.size() > 16) {
		QVector<BandSpot> priority;
		priority.reserve(spots.size());
		for (const BandSpot &s : spots) {
			const QString &l = s.label;
			if (l.contains(QLatin1String("FT8"))
			    || l.contains(QLatin1String("FT4"))
			    || l.contains(QLatin1String("WSPR"))
			    || l.contains(QLatin1String("JS8"))
			    || l.contains(QLatin1String("PSK"))
			    || l.contains(QLatin1String("IBP"))
			    || l.contains(QLatin1String("WWV"))
			    || l.contains(QLatin1String("WWVH"))
			    || l.contains(QLatin1String("CHU"))
			    || l.contains(QLatin1String("beacon"), Qt::CaseInsensitive))
				priority.append(s);
		}
		if (!priority.isEmpty())
			spots = priority;
	}

	OGLText *spotText = m_panel->m_oglTextNormal ? m_panel->m_oglTextNormal
	                                    : (m_panel->m_oglTextSmall ? m_panel->m_oglTextSmall : m_panel->m_oglTextTiny);
	if (!spotText)
		return;

	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	const int left = m_panel->m_panRect.left();
	const int right = m_panel->m_panRect.right();
	const int baseY = m_panel->m_panRect.bottom(); // sit just above the frequency ruler
	const qreal pxPerHz = qreal(m_panel->m_panRect.width()) / span;

	auto freqToX = [&](qint64 freqHz) -> int {
		return left + qRound((qreal(freqHz) - qreal(m_panel->m_centerFrequency) + span / 2.0) * pxPerHz);
	};

	auto spotColor = [](const QString &label) -> QColor {
		if (label.contains(QLatin1String("FT8")))
			return QColor(90, 200, 130);
		if (label.contains(QLatin1String("FT4")))
			return QColor(130, 190, 230);
		if (label.contains(QLatin1String("WSPR")))
			return QColor(230, 170, 70);
		if (label.contains(QLatin1String("JS8")))
			return QColor(190, 150, 230);
		if (label.contains(QLatin1String("[CW"), Qt::CaseInsensitive) || label.endsWith(QLatin1String("CW")))
			return QColor(255, 215, 0); // Gold for CW RBN spots
		if (label.contains(QLatin1String("[SSB"), Qt::CaseInsensitive) || label.contains(QLatin1String("[USB"), Qt::CaseInsensitive) || label.contains(QLatin1String("[LSB"), Qt::CaseInsensitive))
			return QColor(80, 190, 245); // Sky blue for SSB spots
		if (label.contains(QLatin1String("[RTTY"), Qt::CaseInsensitive) || label.contains(QLatin1String("RTTY")))
			return QColor(245, 90, 180); // Magenta for RTTY spots
		if (label.contains(QLatin1String("IBP")) || label.contains(QLatin1String("beacon"), Qt::CaseInsensitive))
			return QColor(110, 210, 210);
		return QColor(200, 205, 215);
	};

	const int textH = spotText->fontMetrics().height();
	const int lanePitch = textH + 3;
	int lane = 0;
	int lastLabelRight[3] = { left - 1000, left - 1000, left - 1000 };

	for (const BandSpot &spot : spots) {
		const int x = freqToX(spot.freqHz);
		if (x < left || x > right)
			continue;

		const QColor col = spotColor(spot.label);
		const int textW = spotText->fontMetrics().horizontalAdvance(spot.label);

		// Pick the first staggered lane that doesn't overlap the previous label.
		int chosen = 0;
		for (; chosen < 3; ++chosen) {
			if (x - 2 >= lastLabelRight[chosen] + 6)
				break;
		}
		if (chosen >= 3)
			chosen = lane % 3;
		lane = chosen + 1;
		lastLabelRight[chosen] = x - 2 + textW + 8;

		// Labels grow upward from the frequency ruler.
		const int labelY = baseY - (chosen + 1) * lanePitch;
		if (labelY < m_panel->m_panRect.top() + 2)
			continue;

		const int labelX = qBound(left, x - 2, right - textW - 6);
		m_panel->drawPanelRect(QRect(x, labelY, 1, baseY - labelY), QColor(col.red(), col.green(), col.blue(), 140), -1.25f);
		m_panel->drawPanelRect(QRect(x - 2, baseY - 3, 5, 5), col, -1.2f);
		m_panel->drawPanelRect(QRect(labelX, labelY, textW + 6, textH + 1), QColor(8, 10, 14, 190), -1.15f);

		m_panel->m_glTextColor = QColor(0, 0, 0, 160);
		m_panel->renderPanelText(spotText, float(labelX + 4), float(labelY + 1), -1.1f, spot.label);
		m_panel->m_glTextColor = col;
		m_panel->renderPanelText(spotText, float(labelX + 3), float(labelY), -1.05f, spot.label);
	}
}
