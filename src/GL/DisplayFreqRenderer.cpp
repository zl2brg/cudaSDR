/**
 * @file  DisplayFreqRenderer.cpp
 * @brief Renderer for VFO frequency rows, digits, and RX region in OGLDisplayPanel.
 * @author Simon Eatough <simon.eatough@gmail.com>
 * @date 2026-09-08
 */

#include "DisplayFreqRenderer.h"
#include "cusdr_oglDisplayPanel.h"
#include "Models/RadioModel.h"
#include "Models/SliceModel.h"
#include "Models/BandPlanManager.h"
#include "cusdr_fonts.h"
#include "cusdr_oglText.h"

DisplayFreqRenderer::DisplayFreqRenderer(OGLDisplayPanel *panel)
    : m_panel(panel)
{
}

QString DisplayFreqRenderer::freqMhzDisplayString(qint64 frequencyHz) const
{
    const int f1 = static_cast<int>(frequencyHz / 1000);
    const long ghz = f1 / 1000000;
    const long mhz = (f1 / 1000) % 1000;
    const long khz = f1 % 1000;

    QString f1str = QString("%1.%2.%3")
            .arg(ghz)
            .arg(mhz, 3, 10, QLatin1Char('0'))
            .arg(khz, 3, 10, QLatin1Char('0'));

    for (int i = 0; i < f1str.length() - 1; ++i) {
        if (f1str[i] == '0' || f1str[i] == '.')
            f1str[i] = ' ';
        else
            break;
    }

    return f1str;
}

void DisplayFreqRenderer::splitFreqDisplay(qint64 frequencyHz, QString *f1str, QString *f2str) const
{
    if (f1str)
        *f1str = freqMhzDisplayString(frequencyHz);
    if (f2str)
        *f2str = QString("%1").arg(static_cast<int>(frequencyHz % 1000), 3, 10, QLatin1Char('0'));
}

void DisplayFreqRenderer::renderFreqText(OGLText *text, int &x1, int y1, const QColor &fontcolor,
                                         const QString &freqstr, int digit, int digit_pos, int fixed_width)
{
    if (!text || !m_panel)
        return;

    const QFontMetrics fontMetrics = text->fontMetrics();
    const int len = freqstr.length();

    for (int x = 0; x < len; x++) {
        const int current_pos = x + digit;
        const bool isDot = (current_pos == OGLDisplayPanel::dp0 ||
                            current_pos == OGLDisplayPanel::dp1 ||
                            current_pos == OGLDisplayPanel::dp2);

        QColor freqdigitcolor;
        if (m_panel->set->getRadioState() > RadioState::RX)
            freqdigitcolor = m_panel->m_txdigitColor;
        else if (current_pos == digit_pos)
            freqdigitcolor = QColor(106, 236, 248);
        else
            freqdigitcolor = fontcolor;

        // Blanked leading slots (GHz / 100 MHz on HF) collapse instead of
        // reserving width, so the first significant digit starts at the margin.
        if (freqstr.at(x) == ' ')
            continue;

        m_panel->qglColor(freqdigitcolor);
        m_panel->renderPanelText(text, x1, y1, QString(freqstr.at(x)));

        if (isDot)
            x1 += m_panel->m_pointStringWidth;
        else if (fixed_width > 0)
            x1 += fixed_width;
        else
            x1 += fontMetrics.horizontalAdvance(freqstr.at(x));
    }
}

void DisplayFreqRenderer::paintVfoFrequencyRow(int which, bool active, int yBaseline, int originX,
                                               const QString &f1str, const QString &f2str,
                                               const QColor &fontcolor)
{
    if (!m_panel)
        return;

    OGLText *text1 = active ? m_panel->m_oglTextFreq1 : m_panel->m_oglTextFreqInactive1;
    OGLText *text2 = active ? m_panel->m_oglTextFreq2 : m_panel->m_oglTextFreqInactive2;
    const int digitW1 = active ? m_panel->m_blankWidthf1 : m_panel->m_blankWidthInactive1;
    const int pointW = active ? m_panel->m_pointStringWidth : m_panel->m_pointStringWidthInactive;

    const int freq1Ascent = text1->fontMetrics().ascent();
    const int freq2Ascent = text2->fontMetrics().ascent();
    const GLint yFreq1 = yBaseline - freq1Ascent;
    const GLint yFreq2 = yBaseline - freq2Ascent;

    QColor rowColor = fontcolor;
    if (m_panel->m_dataEngineState == QSDR::DataEngineUp && !active)
        rowColor = QColor(96, 118, 128);

    const bool highlight = (m_panel->m_digitVfo == which);
    const int digitPos = highlight ? m_panel->m_digitPosition : OGLDisplayPanel::None;

    // A/B chip: bright blue behind the active letter, dark slab behind the inactive one.
    const QString label = (which == OGLDisplayPanel::DigitVfoB) ? QStringLiteral("B") : QStringLiteral("A");
    const QRect labelBox = m_panel->vfoLabelRect(yBaseline);
    const bool live = (m_panel->m_dataEngineState == QSDR::DataEngineUp);
    if (active && live)
        m_panel->drawPanelRoundedRect(labelBox, QColor(31, 111, 235), 4, -2.3f);
    else
        m_panel->drawPanelRoundedRect(labelBox, QColor(38, 38, 38), 4, -2.3f);

    const QFontMetrics labelMetrics = m_panel->m_oglTextBig->fontMetrics();
    const GLint labelX = labelBox.left()
                         + (labelBox.width() - labelMetrics.horizontalAdvance(label)) / 2;
    const GLint labelY = yBaseline - labelMetrics.ascent();
    if (active)
        m_panel->qglColor(live ? QColor(255, 255, 255) : rowColor);
    else
        m_panel->qglColor(rowColor);
    m_panel->renderPanelText(m_panel->m_oglTextBig, labelX, labelY, label);

    GLint x1 = originX;
    renderFreqText(text1, x1, yFreq1, rowColor, f1str, 0, digitPos, digitW1);
    m_panel->qglColor(rowColor);
    m_panel->renderPanelText(text1, x1, yFreq1, QStringLiteral("."));

    x1 += pointW;
    const int digitW2 = active ? m_panel->m_blankWidthf2 : m_panel->m_blankWidthInactive2;
    renderFreqText(text2, x1, yFreq2, rowColor, f2str, 10, digitPos, digitW2);
    x1 += 2 * m_panel->m_blankWidth;

    m_panel->qglColor(rowColor);
    m_panel->renderPanelText(text2, x1, yFreq2 - 1, QStringLiteral("MHz"));
}

void DisplayFreqRenderer::paintRxRegion()
{
    if (!m_panel)
        return;

    QColor fontcolor;

    if (m_panel->m_dataEngineState == QSDR::DataEngineUp) {
        m_panel->drawPanelGradientRect(m_panel->m_rect, Qt::black, m_panel->m_bkgColor2, false, -3.0f);
        fontcolor = m_panel->m_activeTextColor;
    } else {
        m_panel->drawPanelRect(m_panel->m_rect, QColor(0, 0, 0, 255), -3.0f);
        fontcolor = QColor(68, 68, 68);
    }

    const qint64 freqA = m_panel->vfoMemoryHz(OGLDisplayPanel::DigitVfoA);
    const qint64 freqB = m_panel->vfoMemoryHz(OGLDisplayPanel::DigitVfoB);
    SliceModel *slice = m_panel->currentSlice();
    const bool bActive = slice && slice->activeVfo() == SliceModel::VfoB;

    splitFreqDisplay(freqA, &m_panel->m_f1strA, &m_panel->m_f2strA);
    splitFreqDisplay(freqB, &m_panel->m_f1strB, &m_panel->m_f2strB);

    const int originX = m_panel->m_rxRect.left() + 12 + m_panel->m_vfoLabelWidth;
    m_panel->m_freqStringLeftPos = originX;
    m_panel->rebuildAllFreqDigitHitRegions();

    const int activeBaseline = m_panel->m_rxRect.top() + (bActive ? m_panel->m_freqDigitsPosYB : m_panel->m_freqDigitsPosYA);

    // Advance of the active row's MHz digits; the active row always uses the large face.
    const QString &activeF1 = bActive ? m_panel->m_f1strB : m_panel->m_f1strA;
    int f1Advance = 0;
    for (int i = 0; i < activeF1.length(); ++i) {
        if (activeF1.at(i) == QLatin1Char(' '))
            continue;
        if (activeF1.at(i) == QLatin1Char('.'))
            f1Advance += m_panel->m_pointStringWidth;
        else
            f1Advance += m_panel->m_blankWidthf1;
    }
    const int rowRight = originX + f1Advance + m_panel->m_pointStringWidth + 3 * m_panel->m_blankWidthf2
                         + 2 * m_panel->m_blankWidth + m_panel->m_fUnitStringWidth;

    // Selection box around the active VFO, mirroring the web client's blue outline. It starts
    // clear of the A/B chip and uses cap height so it hugs the digits below the status badges.
    const int capH = m_panel->m_oglTextFreq1->fontMetrics().capHeight();
    const int boxLeft = m_panel->vfoLabelRect(activeBaseline).right() + 5;
    const QRect activeBox(boxLeft, activeBaseline - capH - 5,
                          rowRight + 6 - boxLeft, capH + 10);
    // Outline only — drawSolidRect drops alpha, so a fill would paint solid blue.
    if (m_panel->m_dataEngineState == QSDR::DataEngineUp)
        m_panel->drawPanelRoundedRectOutline(activeBox, QColor(31, 111, 235), 5, -2.4f);
    else
        m_panel->drawPanelRoundedRectOutline(activeBox, QColor(40, 70, 120), 5, -2.4f);

    paintVfoFrequencyRow(OGLDisplayPanel::DigitVfoA, !bActive, m_panel->m_rxRect.top() + m_panel->m_freqDigitsPosYA,
                         originX, m_panel->m_f1strA, m_panel->m_f2strA, fontcolor);
    paintVfoFrequencyRow(OGLDisplayPanel::DigitVfoB, bActive, m_panel->m_rxRect.top() + m_panel->m_freqDigitsPosYB,
                         originX, m_panel->m_f1strB, m_panel->m_f2strB, fontcolor);

    const GLint yNormal = activeBaseline - m_panel->m_oglTextNormal->fontMetrics().ascent();
    const GLint yBig = activeBaseline - m_panel->m_fonts.fontHeightBigFont
                       - m_panel->m_oglTextBig->fontMetrics().ascent();

    const int metaX = originX + f1Advance + m_panel->m_pointStringWidth + 3 * m_panel->m_blankWidthf2
                      + m_panel->m_fUnitStringWidth + 3 * m_panel->m_blankWidthf2;

    QString str = QStringLiteral("step: %1");
    m_panel->qglColor(fontcolor);
    m_panel->renderPanelText(m_panel->m_oglTextNormal, metaX, yNormal,
                             str.arg(m_panel->set->getValue1000(m_panel->m_mouseWheelFreqStep, 0, "Hz")));

    SliceModel* curSlice = m_panel->currentSlice();
    QString dspModeName = m_panel->set->getDSPModeString(curSlice ? curSlice->dspMode() : m_panel->set->getDSPMode(m_panel->m_currentReceiver));
    if (m_panel->set->getRadioState() == RadioState::RX) {
        m_panel->qglColor(fontcolor);
        m_panel->renderPanelText(m_panel->m_oglTextBig, metaX, yBig,
                                 QStringLiteral("Rx: %1 %2").arg(m_panel->m_currentReceiver + 1).arg(dspModeName));
    } else {
        m_panel->qglColor(m_panel->m_txdigitColor);
        m_panel->renderPanelText(m_panel->m_oglTextBig, metaX, yBig,
                                 QStringLiteral("Tx: %1 %2").arg(m_panel->m_currentReceiver + 1).arg(dspModeName));
    }

    const qint64 activeFreq = bActive ? freqB : freqA;
    if (m_panel->m_oldFreq != activeFreq) {
        QString planLabel;
        if (m_panel->m_radioModel && m_panel->m_radioModel->bandPlan())
            planLabel = m_panel->m_radioModel->bandPlan()->labelAt(activeFreq);
        if (!planLabel.isEmpty()) {
            const int pipe = planLabel.indexOf(QLatin1Char('|'));
            m_panel->m_bandText = (pipe >= 0) ? planLabel.left(pipe).trimmed() : planLabel;
        } else {
            m_panel->m_bandText = getHamBandTextString(m_panel->set->getHamBandTextList(), false, activeFreq);
        }
        m_panel->m_oldFreq = activeFreq;
    }

    const GLint yBand = m_panel->m_rxRect.height() - m_panel->m_lowerRectY
                        - m_panel->m_oglTextSmall->fontMetrics().height() - 2;
    m_panel->qglColor(fontcolor);
    m_panel->renderPanelText(m_panel->m_oglTextSmall, originX, yBand, m_panel->m_bandText);
}
