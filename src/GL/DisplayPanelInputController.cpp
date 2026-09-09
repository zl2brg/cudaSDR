/**
 * @file  DisplayPanelInputController.cpp
 * @brief Controller for mouse, wheel, keyboard, and digit hit-testing on OGLDisplayPanel.
 * @author Simon Eatough <simon.eatough@gmail.com>
 * @date 2026-09-09
 */

#include "DisplayPanelInputController.h"
#include "cusdr_oglDisplayPanel.h"
#include "Models/RadioModel.h"
#include "Models/SliceModel.h"
#include "cusdr_settings.h"
#include "cusdr_fonts.h"
#include "cusdr_oglText.h"
#include "UI/FrequencyEntryDialog.h"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QCursor>
#include <QDebug>

DisplayPanelInputController::DisplayPanelInputController(OGLDisplayPanel *panel)
    : QObject(panel)
    , m_panel(panel)
{
}

void DisplayPanelInputController::handleEnter(QEvent *event)
{
    Q_UNUSED(event);
}

void DisplayPanelInputController::handleLeave(QEvent *event)
{
    Q_UNUSED(event);
}

qint64 DisplayPanelInputController::calculateDigitDelta(int digitPosition)
{
    switch (digitPosition) {
        case OGLDisplayPanel::Freq1:          return 1;
        case OGLDisplayPanel::Freq10:         return 10;
        case OGLDisplayPanel::Freq100:        return 100;
        case OGLDisplayPanel::Freq1000:       return 1000;
        case OGLDisplayPanel::Freq10000:      return 10000;
        case OGLDisplayPanel::Freq100000:     return 100000;
        case OGLDisplayPanel::Freq1000000:    return 1000000;
        case OGLDisplayPanel::Freq10000000:   return 10000000;
        case OGLDisplayPanel::Freq100000000:  return 100000000;
        case OGLDisplayPanel::Freq1000000000: return 1000000000;
        default:                              return 0;
    }
}

qint64 DisplayPanelInputController::calculateNewFrequency(qint64 currentFreq, int digitPosition, int numSteps, qint64 maxFreq)
{
    const qint64 deltaF = calculateDigitDelta(digitPosition);
    if (deltaF == 0)
        return currentFreq;

    qint64 newFreq = currentFreq + (qint64)numSteps * deltaF;
    if (newFreq < 0)
        newFreq = 0;
    if (maxFreq > 0 && newFreq > maxFreq)
        newFreq = maxFreq;
    return newFreq;
}

qreal DisplayPanelInputController::cycleFreqStep(int digitPosition, qreal currentStep)
{
    switch (digitPosition) {
        case OGLDisplayPanel::Freq1:
            return (currentStep == 1.0) ? 5.0 : 1.0;
        case OGLDisplayPanel::Freq10:
            return (currentStep == 10.0) ? 50.0 : 10.0;
        case OGLDisplayPanel::Freq100:
            return (currentStep == 100.0) ? 500.0 : 100.0;
        case OGLDisplayPanel::Freq1000:
            if (currentStep == 1000.0) return 5000.0;
            if (currentStep == 5000.0) return 9000.0;
            return 1000.0;
        case OGLDisplayPanel::Freq10000:
            return (currentStep == 10000.0) ? 50000.0 : 10000.0;
        case OGLDisplayPanel::Freq100000:
            return (currentStep == 100000.0) ? 500000.0 : 100000.0;
        case OGLDisplayPanel::Freq1000000:
            return (currentStep == 1000000.0) ? 5000000.0 : 1000000.0;
        case OGLDisplayPanel::Freq10000000:
            return (currentStep == 10000000.0) ? 50000000.0 : 10000000.0;
        case OGLDisplayPanel::Freq100000000:
            return 100000000.0;
        case OGLDisplayPanel::Freq1000000000:
            return 1000000000.0;
        default:
            return currentStep;
    }
}

void DisplayPanelInputController::updateFreqDigitHitRegions(
    OGLDisplayPanel::FreqDigitHitRegions &out,
    int originX,
    int yBaseline,
    const QString &f1str,
    bool large,
    const QRect &labelRect,
    int digitW1,
    int digitW2,
    int pointW,
    int freq1Ascent,
    int freq2Ascent,
    int freq1Height,
    int freq2Height)
{
    Q_UNUSED(large);
    const int y1 = yBaseline - freq1Ascent;
    const int y2 = yBaseline - freq2Ascent;

    out.label = QRegion(labelRect);

    int x = originX;
    auto slot = [&](int strIndex, int width, int top, int height) {
        if (strIndex >= 0 && strIndex < f1str.length() && f1str.at(strIndex) == QLatin1Char(' '))
            return QRegion();
        const QRegion region(QRect(x, top, width, height));
        x += width;
        return region;
    };

    out.freg1000000000 = slot(0, digitW1, y1, freq1Height);
    out.point2         = slot(1, pointW, y1, freq1Height);
    out.freg100000000  = slot(2, digitW1, y1, freq1Height);
    out.freg10000000   = slot(3, digitW1, y1, freq1Height);
    out.freg1000000    = slot(4, digitW1, y1, freq1Height);
    out.point          = slot(5, pointW, y1, freq1Height);
    out.freg100000     = slot(6, digitW1, y1, freq1Height);
    out.freg10000      = slot(7, digitW1, y1, freq1Height);
    out.freg1000       = slot(8, digitW1, y1, freq1Height);
    out.point1         = slot(-1, pointW, y1, freq1Height);
    out.freg100        = slot(-1, digitW2, y2, freq2Height);
    out.freg10         = slot(-1, digitW2, y2, freq2Height);
    out.freg1          = slot(-1, digitW2, y2, freq2Height);
}

bool DisplayPanelInputController::hitTestDigit(
    const OGLDisplayPanel::FreqDigitHitRegions &regs,
    const QString &f1str,
    const QPoint &p,
    int *digitOut)
{
    int digit = OGLDisplayPanel::None;
    if (regs.freg1.contains(p))
        digit = OGLDisplayPanel::Freq1;
    else if (regs.freg10.contains(p))
        digit = OGLDisplayPanel::Freq10;
    else if (regs.freg100.contains(p))
        digit = OGLDisplayPanel::Freq100;
    else if (regs.point1.contains(p))
        digit = OGLDisplayPanel::dp2;
    else if (regs.freg1000.contains(p))
        digit = OGLDisplayPanel::Freq1000;
    else if (regs.freg10000.contains(p))
        digit = OGLDisplayPanel::Freq10000;
    else if (regs.freg100000.contains(p))
        digit = OGLDisplayPanel::Freq100000;
    else if (regs.point.contains(p))
        digit = OGLDisplayPanel::dp1;
    else if (regs.freg1000000.contains(p))
        digit = OGLDisplayPanel::Freq1000000;
    else if (regs.freg10000000.contains(p))
        digit = OGLDisplayPanel::Freq10000000;
    else if (regs.freg100000000.contains(p))
        digit = OGLDisplayPanel::Freq100000000;
    else if (regs.point2.contains(p))
        digit = OGLDisplayPanel::dp0;
    else if (regs.freg1000000000.contains(p))
        digit = OGLDisplayPanel::Freq1000000000;

    if (digit != OGLDisplayPanel::None && digit <= OGLDisplayPanel::Freq1000) {
        int idx = -1;
        switch (digit) {
            case OGLDisplayPanel::Freq1000000000: idx = 0; break;
            case OGLDisplayPanel::dp0:            idx = 1; break;
            case OGLDisplayPanel::Freq100000000:  idx = 2; break;
            case OGLDisplayPanel::Freq10000000:   idx = 3; break;
            case OGLDisplayPanel::Freq1000000:    idx = 4; break;
            case OGLDisplayPanel::dp1:            idx = 5; break;
            case OGLDisplayPanel::Freq100000:     idx = 6; break;
            case OGLDisplayPanel::Freq10000:      idx = 7; break;
            case OGLDisplayPanel::Freq1000:       idx = 8; break;
            default: break;
        }
        if (idx >= 0 && idx < f1str.length() && f1str[idx] == ' ')
            digit = OGLDisplayPanel::None;
    }

    if (digitOut)
        *digitOut = digit;
    return digit != OGLDisplayPanel::None;
}

void DisplayPanelInputController::rebuildAllFreqDigitHitRegions()
{
    if (!m_panel || !m_panel->m_oglTextFreq1 || !m_panel->m_oglTextFreqInactive1)
        return;

    SliceModel *slice = m_panel->currentSlice();
    const bool bActive = slice && slice->activeVfo() == SliceModel::VfoB;
    const int originX = m_panel->m_rxRect.left() + 12 + m_panel->m_vfoLabelWidth;

    const QRect labelA = vfoLabelRect(m_panel->m_rxRect.top() + m_panel->m_freqDigitsPosYA);
    const QRect labelB = vfoLabelRect(m_panel->m_rxRect.top() + m_panel->m_freqDigitsPosYB);

    QString f1A = m_panel->m_f1strA;
    QString f1B = m_panel->m_f1strB;
    if (f1A.isEmpty())
        f1A = m_panel->freqMhzDisplayString(vfoMemoryHz(OGLDisplayPanel::DigitVfoA));
    if (f1B.isEmpty())
        f1B = m_panel->freqMhzDisplayString(vfoMemoryHz(OGLDisplayPanel::DigitVfoB));

    OGLText *text1A = !bActive ? m_panel->m_oglTextFreq1 : m_panel->m_oglTextFreqInactive1;
    OGLText *text2A = !bActive ? m_panel->m_oglTextFreq2 : m_panel->m_oglTextFreqInactive2;
    const int digitW1A = !bActive ? m_panel->m_blankWidthf1 : m_panel->m_blankWidthInactive1;
    const int digitW2A = !bActive ? m_panel->m_blankWidthf2 : m_panel->m_blankWidthInactive2;
    const int pointWA = !bActive ? m_panel->m_pointStringWidth : m_panel->m_pointStringWidthInactive;

    updateFreqDigitHitRegions(
        m_hitA, originX, m_panel->m_rxRect.top() + m_panel->m_freqDigitsPosYA,
        f1A, !bActive, labelA,
        digitW1A, digitW2A, pointWA,
        text1A ? text1A->fontMetrics().ascent() : m_panel->m_fonts.fontHeightFreqFont1,
        text2A ? text2A->fontMetrics().ascent() : m_panel->m_fonts.fontHeightFreqFont2,
        text1A ? text1A->fontMetrics().height() : m_panel->m_fonts.fontHeightFreqFont1,
        text2A ? text2A->fontMetrics().height() : m_panel->m_fonts.fontHeightFreqFont2);

    OGLText *text1B = bActive ? m_panel->m_oglTextFreq1 : m_panel->m_oglTextFreqInactive1;
    OGLText *text2B = bActive ? m_panel->m_oglTextFreq2 : m_panel->m_oglTextFreqInactive2;
    const int digitW1B = bActive ? m_panel->m_blankWidthf1 : m_panel->m_blankWidthInactive1;
    const int digitW2B = bActive ? m_panel->m_blankWidthf2 : m_panel->m_blankWidthInactive2;
    const int pointWB = bActive ? m_panel->m_pointStringWidth : m_panel->m_pointStringWidthInactive;

    updateFreqDigitHitRegions(
        m_hitB, originX, m_panel->m_rxRect.top() + m_panel->m_freqDigitsPosYB,
        f1B, bActive, labelB,
        digitW1B, digitW2B, pointWB,
        text1B ? text1B->fontMetrics().ascent() : m_panel->m_fonts.fontHeightFreqFont1,
        text2B ? text2B->fontMetrics().ascent() : m_panel->m_fonts.fontHeightFreqFont2,
        text1B ? text1B->fontMetrics().height() : m_panel->m_fonts.fontHeightFreqFont1,
        text2B ? text2B->fontMetrics().height() : m_panel->m_fonts.fontHeightFreqFont2);
}

void DisplayPanelInputController::getSelectedDigit(const QPoint &p)
{
    static int pos;
    static int posVfo;
    m_digitPosition = OGLDisplayPanel::None;
    m_digitVfo = OGLDisplayPanel::DigitVfoNone;

    int digit = OGLDisplayPanel::None;
    if (hitTestDigit(m_hitA, m_panel->m_f1strA, p, &digit)) {
        m_digitPosition = digit;
        m_digitVfo = OGLDisplayPanel::DigitVfoA;
    } else if (hitTestDigit(m_hitB, m_panel->m_f1strB, p, &digit)) {
        m_digitPosition = digit;
        m_digitVfo = OGLDisplayPanel::DigitVfoB;
    } else if (m_hitA.label.contains(p)) {
        m_digitVfo = OGLDisplayPanel::DigitVfoA;
    } else if (m_hitB.label.contains(p)) {
        m_digitVfo = OGLDisplayPanel::DigitVfoB;
    }

    if (m_panel) {
        m_panel->m_digitPosition = m_digitPosition;
        m_panel->m_digitVfo = m_digitVfo;
    }

    if (pos != m_digitPosition || posVfo != m_digitVfo) {
        pos = m_digitPosition;
        posVfo = m_digitVfo;
        if (m_panel)
            m_panel->update();
    }
}

void DisplayPanelInputController::handleMousePress(QMouseEvent *event)
{
    if (!m_panel)
        return;

    const QPoint pos = event->pos();
    getSelectedDigit(pos);

    if (event->button() == Qt::LeftButton && m_digitVfo != OGLDisplayPanel::DigitVfoNone) {
        activateDigitVfo(static_cast<OGLDisplayPanel::DigitVfo>(m_digitVfo));
    }

    if (event->button() == Qt::LeftButton && m_digitPosition != OGLDisplayPanel::None) {
        const qreal currentStep = m_panel->set->getMouseWheelFreqStep(m_panel->m_currentReceiver);
        const qreal newStep = cycleFreqStep(m_digitPosition, currentStep);
        if (newStep != currentStep) {
            m_panel->set->setMouseWheelFreqStep(m_panel->m_currentReceiver, newStep);
        }
    }
}

void DisplayPanelInputController::handleMouseRelease(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void DisplayPanelInputController::handleMouseDoubleClick(QMouseEvent *event)
{
    if (!m_panel)
        return;

    if (event->button() == Qt::LeftButton) {
        const QPoint pos = event->pos();
        getSelectedDigit(pos);

        if (m_digitVfo != OGLDisplayPanel::DigitVfoNone) {
            if (m_panel->m_currentReceiver < 0 || m_panel->m_currentReceiver >= m_panel->m_frequencyList.size()) {
                qWarning() << "DisplayPanelInputController::handleMouseDoubleClick invalid receiver index" << m_panel->m_currentReceiver;
                return;
            }

            const OGLDisplayPanel::DigitVfo which = static_cast<OGLDisplayPanel::DigitVfo>(m_digitVfo);
            activateDigitVfo(which);
            const qint64 currentFreq = vfoMemoryHz(which);
            FrequencyEntryDialog dlg(currentFreq, m_panel);
            if (dlg.exec() == QDialog::Accepted) {
                const qint64 newFreq = dlg.frequency();
                if (newFreq < static_cast<qint64>(m_panel->set->getMaxFrequency()) && newFreq >= 0)
                    tuneDigitVfoTo(which, newFreq);
            }
        }
    }
}

void DisplayPanelInputController::handleMouseMove(QMouseEvent *event)
{
    if (!m_panel)
        return;

    const QPoint pos = event->pos();
    const int oldDigit = m_digitPosition;
    const int oldVfo = m_digitVfo;

    if (m_panel->m_dataEngineState != QSDR::DataEngineUp) {
        m_panel->m_digitColor = QColor(98, 98, 98);
        return;
    }

    getSelectedDigit(pos);
    Qt::CursorShape wantCursor = Qt::ArrowCursor;
    switch (m_digitPosition) {
        case OGLDisplayPanel::Freq1:
        case OGLDisplayPanel::Freq10:
        case OGLDisplayPanel::Freq100:
        case OGLDisplayPanel::Freq1000:
        case OGLDisplayPanel::Freq10000:
        case OGLDisplayPanel::Freq100000:
        case OGLDisplayPanel::Freq1000000:
        case OGLDisplayPanel::Freq10000000:
        case OGLDisplayPanel::Freq100000000:
        case OGLDisplayPanel::Freq1000000000:
            wantCursor = Qt::PointingHandCursor;
            m_panel->m_digitColor = QColor(136, 166, 178);
            break;

        case OGLDisplayPanel::None:
            wantCursor = (m_digitVfo != OGLDisplayPanel::DigitVfoNone) ? Qt::PointingHandCursor : Qt::ArrowCursor;
            m_panel->m_digitColor = QColor(106, 136, 148);
            break;
        default:
            break;
    }

    if (m_panel->cursor().shape() != wantCursor)
        m_panel->setCursor(wantCursor);

    if (oldDigit != m_digitPosition || oldVfo != m_digitVfo)
        m_panel->scheduleRepaint();
}

void DisplayPanelInputController::handleWheel(QWheelEvent *event)
{
    if (!m_panel)
        return;

    const qint64 deltaF = calculateDigitDelta(m_digitPosition);
    if (deltaF == 0)
        return;

    const int numDegrees = event->angleDelta().y() / 8;
    const int numSteps = numDegrees / 15;

    if (m_panel->m_currentReceiver < 0 || m_panel->m_currentReceiver >= m_panel->m_frequencyList.size()) {
        qWarning() << "DisplayPanelInputController::handleWheel invalid receiver index" << m_panel->m_currentReceiver;
        return;
    }

    const OGLDisplayPanel::DigitVfo which = (m_digitVfo == OGLDisplayPanel::DigitVfoB) ? OGLDisplayPanel::DigitVfoB : OGLDisplayPanel::DigitVfoA;
    const qint64 currentFreq = vfoMemoryHz(which);
    const qint64 maxFreq = static_cast<qint64>(m_panel->set->getMaxFrequency());
    const qint64 newFreq = calculateNewFrequency(currentFreq, m_digitPosition, numSteps, maxFreq);

    tuneDigitVfoTo(which, newFreq);
    event->accept();
}

QRect DisplayPanelInputController::vfoLabelRect(int yBaseline) const
{
    if (!m_panel || !m_panel->m_oglTextBig)
        return QRect();
    const QFontMetrics fm = m_panel->m_oglTextBig->fontMetrics();
    return QRect(m_panel->m_rxRect.left() + 4, yBaseline - fm.ascent() - 2,
                 qMax(m_panel->m_vfoLabelWidth, 16), fm.height() + 4);
}

qint64 DisplayPanelInputController::vfoMemoryHz(OGLDisplayPanel::DigitVfo which) const
{
    if (!m_panel)
        return 7000000;
    if (SliceModel *slice = m_panel->currentSlice()) {
        return (which == OGLDisplayPanel::DigitVfoB) ? slice->vfoBFrequency() : slice->vfoAFrequency();
    }
    if (m_panel->m_currentReceiver >= 0 && m_panel->m_currentReceiver < m_panel->m_frequencyList.size())
        return m_panel->m_frequencyList.at(m_panel->m_currentReceiver).frequency;
    return 7000000;
}

void DisplayPanelInputController::activateDigitVfo(OGLDisplayPanel::DigitVfo which)
{
    SliceModel *slice = m_panel ? m_panel->currentSlice() : nullptr;
    if (!slice || which == OGLDisplayPanel::DigitVfoNone)
        return;
    const SliceModel::ActiveVfo target = (which == OGLDisplayPanel::DigitVfoB) ? SliceModel::VfoB : SliceModel::VfoA;
    if (slice->activeVfo() == target)
        return;
    slice->setActiveVfo(target);
    if (m_panel && m_panel->set)
        m_panel->set->setVfoFrequencyVisible(m_panel->m_currentReceiver, slice->frequency());
}

void DisplayPanelInputController::tuneDigitVfoTo(OGLDisplayPanel::DigitVfo which, qint64 frequencyHz)
{
    activateDigitVfo(which);
    SliceModel *slice = m_panel ? m_panel->currentSlice() : nullptr;
    if (!slice || !m_panel)
        return;

    if (which == OGLDisplayPanel::DigitVfoB)
        slice->setVfoBFrequency(frequencyHz);
    else
        slice->setVfoAFrequency(frequencyHz);

    if (m_panel->set->getPanLockedStatus(m_panel->m_currentReceiver)) {
        const qint64 ctrf = slice->centerFrequency();
        const int s = (m_panel->m_radioModel ? m_panel->m_radioModel->sampleRate() : m_panel->set->getSampleRate()) / 2;
        if (frequencyHz > ctrf + s)
            frequencyHz = ctrf + s;
        else if (frequencyHz < ctrf - s)
            frequencyHz = ctrf - s;
        m_panel->set->setVFOFrequency(0, m_panel->m_currentReceiver, frequencyHz);
    } else {
        // Unlocked pan: digit wheel moves LO with the dial (legacy behaviour).
        m_panel->set->setCtrFrequency(0, m_panel->m_currentReceiver, frequencyHz);
        m_panel->set->setVFOFrequency(0, m_panel->m_currentReceiver, frequencyHz);
    }
}
