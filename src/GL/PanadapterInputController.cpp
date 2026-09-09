/**
 * @file  PanadapterInputController.cpp
 * @brief Controller for mouse, wheel, keyboard, and gesture input on the OpenGL receiver panel.
 * @author Simon Eatough <simon.eatough@gmail.com>
 * @date 2026-09-08
 */

#include "PanadapterInputController.h"
#include "cusdr_oglReceiverPanel.h"
#include "Models/SliceModel.h"
#include "cusdr_settings.h"
#include "UI/FrequencyEntryDialog.h"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QEnterEvent>
#include <QGuiApplication>
#include <QCursor>
#include <QtMath>

PanadapterInputController::PanadapterInputController(QGLReceiverPanel *panel)
    : QObject(panel)
    , m_panel(panel)
{
}

PanadapterInputController::Region PanadapterInputController::determineRegion(
    const QPoint &p,
    const QRect &panRect,
    const QRect &waterfallRect,
    const QRect &freqScalePanRect,
    const QRect &dBmScalePanRect,
    const QRect &filterRect,
    const QRect &secScaleWaterfallRect,
    const QRect &agcButtonRect,
    const QRect &panSMeterRect,
    const QRect &panFreqRect,
    int snapMouse,
    qreal agcThresholdPixel,
    qreal agcHangLevelPixel,
    qreal agcFixedGainLevelPixel,
    bool isShiftPressed)
{
    if (agcButtonRect.contains(p)) {
        return agcButtonRegion;
    }
    if (freqScalePanRect.contains(p)) {
        return freqScalePanadapterRegion;
    }
    if (dBmScalePanRect.contains(p)) {
        return dBmScalePanadapterRegion;
    }
    if (filterRect.width() > 0
        && qAbs(p.x() - filterRect.left()) < snapMouse
        && ((panRect.contains(p) && !dBmScalePanRect.contains(p))
            || (waterfallRect.contains(p) && !secScaleWaterfallRect.contains(p))))
    {
        return filterRegionLow;
    }
    if (filterRect.width() > 0
        && qAbs(p.x() - filterRect.right()) < snapMouse
        && ((panRect.contains(p) && !dBmScalePanRect.contains(p))
            || (waterfallRect.contains(p) && !secScaleWaterfallRect.contains(p))))
    {
        return filterRegionHigh;
    }
    if ((filterRect.contains(p)
         || (filterRect.width() > 0
             && p.x() >= filterRect.left() && p.x() <= filterRect.right()
             && waterfallRect.contains(p) && !secScaleWaterfallRect.contains(p)))
        && isShiftPressed)
    {
        return filterRegion;
    }
    if (qAbs(p.y() - agcThresholdPixel) < snapMouse) {
        return agcThresholdLine;
    }
    if (qAbs(p.y() - agcHangLevelPixel) < snapMouse) {
        return agcHangLine;
    }
    if (qAbs(p.y() - agcFixedGainLevelPixel) < snapMouse) {
        return agcFixedGainLine;
    }
    if (panRect.contains(p)) {
        if ((panSMeterRect.isValid() && panSMeterRect.contains(p)) ||
            (panFreqRect.isValid() && panFreqRect.contains(p)))
            return elsewhere;
        return panadapterRegion;
    }
    if (waterfallRect.contains(p)) {
        return waterfallRegion;
    }
    return elsewhere;
}

qint64 PanadapterInputController::calculateWheelFrequency(
    qint64 currentFreq,
    int angleDeltaY,
    double freqStep,
    qint64 minFreq,
    qint64 maxFreq)
{
    if (angleDeltaY == 0 || freqStep == 0.0)
        return currentFreq;

    const double delta = (angleDeltaY < 0) ? -freqStep : freqStep;
    const double stepAbs = qAbs(freqStep);
    qint64 newFreq = static_cast<qint64>(qRound((currentFreq + delta) / stepAbs) * stepAbs);
    return qBound(minFreq, newFreq, maxFreq);
}

qint64 PanadapterInputController::calculatePanDragCenterFreq(
    qint64 currentCenterFreq,
    int deltaX,
    qreal spanHz,
    int rectWidth,
    qint64 minFreq,
    qint64 maxFreq)
{
    if (rectWidth <= 0)
        return currentCenterFreq;

    const qreal unit = spanHz / static_cast<qreal>(rectWidth);
    const qreal deltaFreq = unit * static_cast<qreal>(deltaX);
    qint64 newFreq = currentCenterFreq + static_cast<long>(qRound(deltaFreq));
    if (newFreq < 0)
        newFreq = 0;
    return qBound(minFreq, newFreq, maxFreq);
}

void PanadapterInputController::calculateFilterDragEdges(
    int deltaX,
    qreal spanHz,
    int rectWidth,
    qreal initialLo,
    qreal initialHi,
    bool dragLow,
    bool dragHigh,
    qreal &outLo,
    qreal &outHi)
{
    if (rectWidth <= 0) {
        outLo = initialLo;
        outHi = initialHi;
        return;
    }

    const qreal unit = spanHz / static_cast<qreal>(rectWidth);
    const qreal dFreq = static_cast<qreal>(deltaX) * unit;

    if (dragLow && dragHigh) {
        outLo = qRound(initialLo - dFreq);
        outHi = qRound(initialHi - dFreq);
    } else if (dragLow) {
        outLo = qRound(initialLo - dFreq);
        outHi = initialHi;
    } else if (dragHigh) {
        outLo = initialLo;
        outHi = qRound(initialHi - dFreq);
    } else {
        outLo = initialLo;
        outHi = initialHi;
    }
}

void PanadapterInputController::calculateDbmScaleDrag(
    int deltaY,
    int rectHeight,
    qreal initialMin,
    qreal initialMax,
    qreal &outMin,
    qreal &outMax)
{
    if (rectHeight <= 0) {
        outMin = initialMin;
        outMax = initialMax;
        return;
    }

    const qreal unit = (qAbs(initialMax - initialMin) / static_cast<qreal>(rectHeight)) * 1.5;
    const qreal newMin = initialMin - unit * static_cast<qreal>(deltaY);
    const qreal newMax = initialMax - unit * static_cast<qreal>(deltaY);

    if (newMin > MINDBM && newMax < MAXDBM) {
        outMin = newMin;
        outMax = newMax;
    } else {
        outMin = initialMin;
        outMax = initialMax;
    }
}

void PanadapterInputController::getRegion(const QPoint &p) {
    if (!m_panel) return;

    const bool isShift = (QGuiApplication::keyboardModifiers() & Qt::ShiftModifier);
    const Region r = determineRegion(
        p,
        m_panel->m_panRect,
        m_panel->m_waterfallRect,
        m_panel->m_freqScalePanRect,
        m_panel->m_dBmScalePanRect,
        m_panel->m_filterRect,
        m_panel->m_secScaleWaterfallRect,
        m_panel->m_agcButtonRect,
        m_panel->m_panSMeterRect,
        m_panel->m_panFreqRect,
        m_panel->m_snapMouse,
        m_panel->m_agcThresholdPixel,
        m_panel->m_agcHangLevelPixel,
        m_panel->m_agcFixedGainLevelPixel,
        isShift
    );

    m_panel->m_mouseRegion = static_cast<QGLReceiverPanel::Region>(r);

    if (r == filterRegionLow) {
        m_panel->m_mouseDownFilterFrequencyLo = m_panel->m_filterLowerFrequency;
    } else if (r == filterRegionHigh) {
        m_panel->m_mouseDownFilterFrequencyHi = m_panel->m_filterUpperFrequency;
    } else if (r == agcThresholdLine) {
        m_panel->m_mouseDownAGCThreshold = m_panel->m_agcThresholdOld;
    } else if (r == agcHangLine) {
        m_panel->m_mouseDownAGCHangLevel = m_panel->m_agcHangLevelOld;
    } else if (r == agcFixedGainLine) {
        m_panel->m_mouseDownFixedGainLevel = -m_panel->m_agcFixedGain;
    }
}

void PanadapterInputController::handleEnter(QEnterEvent *event) {
    Q_UNUSED(event)
    if (!m_panel) return;

    m_panel->setFocus(Qt::MouseFocusReason);
    m_panel->m_mousePos = QPoint(-1, -1);
    m_panel->m_mouseRegion = QGLReceiverPanel::elsewhere;
    m_panel->setCursor(Qt::BlankCursor);
}

void PanadapterInputController::handleLeave(QEvent *event) {
    Q_UNUSED(event)
    if (!m_panel) return;

    m_panel->m_mousePos = QPoint(-100, -100);
    m_panel->m_mouseRegion = QGLReceiverPanel::elsewhere;
}

void PanadapterInputController::handleMousePress(QMouseEvent* event) {
    if (!m_panel) return;

    // Right-click on decoded CW text box erases the text
    if (event->button() == Qt::RightButton) {
        if (m_panel->m_cwTextRect.isValid() && m_panel->m_cwTextRect.contains(event->pos())) {
            if (m_panel->m_sliceModel) {
                m_panel->m_sliceModel->setCwDecodedText(QString());
            }
            m_panel->update();
            event->accept();
            return;
        }
    }

    // Left-click on decoded CW text box starts movable dragging
    if (event->button() == Qt::LeftButton && m_panel->m_cwTextRect.isValid() && m_panel->m_cwTextRect.contains(event->pos())) {
        m_panel->m_dragCwText = true;
        m_panel->m_cwDragStartMouse = event->pos();
        if (!m_panel->m_hasCustomCwBoxPos) {
            m_panel->m_cwBoxPos = m_panel->m_cwTextRect.topLeft();
            m_panel->m_hasCustomCwBoxPos = true;
        }
        m_panel->setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    m_panel->m_mousePos = event->pos();
    m_panel->m_mouseDownPos = m_panel->m_mousePos;

    getRegion(m_panel->m_mousePos);

    // Click on VFO badge in mini frequency display toggles VFO A/B
    if (event->button() == Qt::LeftButton && m_panel->m_panFreqVfoRect.isValid() && m_panel->m_panFreqVfoRect.contains(m_panel->m_mousePos)) {
        if (m_panel->m_sliceModel) {
            m_panel->m_sliceModel->setActiveVfo(m_panel->m_sliceModel->activeVfo() == SliceModel::VfoA ? SliceModel::VfoB : SliceModel::VfoA);
            m_panel->update();
            event->accept();
            return;
        }
    }

    // Click-to-tune (Click-VFO or Shift+Click outside filter) on panadapter or waterfall
    if (event->button() == Qt::LeftButton && m_panel->m_mouseRegion != QGLReceiverPanel::filterRegion && (m_panel->m_clickVFO || (event->modifiers() & Qt::ShiftModifier))) {
        if (m_panel->m_panRect.contains(m_panel->m_mousePos) || m_panel->m_waterfallRect.contains(m_panel->m_mousePos)) {
            m_panel->m_dragMouse = false;
            m_panel->m_highlightFilter = false;

            if (m_panel->m_receiver != m_panel->set->getCurrentReceiver()) {
                m_panel->set->setCurrentReceiver(m_panel->m_receiver);
            }

            const int dx = m_panel->m_panRect.width()/2 - m_panel->m_mousePos.x();
            const qreal unit = m_panel->displayedFrequencySpanHz() / m_panel->m_panRect.width();
            qint64 clickedFreq = (qint64)(qRound(m_panel->m_centerFrequency - (unit * dx)));

            const DSPMode mode = m_panel->m_sliceModel ? m_panel->m_sliceModel->dspMode() : m_panel->m_dspMode;
            const bool isCw = (mode == DSPMode::CWL || mode == DSPMode::CWU);
            const int cwPitch = m_panel->set->getCwSidetoneFreq();

            // Auto-snap to nearest peak within ~25 pixels if clicking near a CW carrier or with Shift key
            bool peakFound = false;
            const qint64 peakRf = m_panel->findPeakFrequencyNear(clickedFreq, qMax(600, qRound(unit * 25.0)), &peakFound);

            qint64 newVfo = clickedFreq;
            if (peakFound && isCw) {
                // Zero-beat snap for CW
                newVfo = (mode == DSPMode::CWL) ? (peakRf + cwPitch) : (peakRf - cwPitch);
            } else if (peakFound && (event->modifiers() & Qt::ShiftModifier)) {
                newVfo = peakRf;
            }

            newVfo = qBound(m_panel->m_centerFrequency - m_panel->m_sampleRate/2, newVfo, m_panel->m_centerFrequency + m_panel->m_sampleRate/2);

            m_panel->m_vfoFrequency = newVfo;
            m_panel->m_deltaFrequency = m_panel->m_centerFrequency - m_panel->m_vfoFrequency;
            m_panel->m_deltaF = (qreal)(1.0 * m_panel->m_deltaFrequency / m_panel->m_sampleRate);

            m_panel->set->setVFOFrequency(0, m_panel->m_receiver, m_panel->m_vfoFrequency);
            m_panel->update();
            event->accept();
            return;
        }
    }

    if (m_panel->m_mouseRegion == QGLReceiverPanel::agcButtonRegion) {
        if (event->buttons() == Qt::LeftButton) {
            if (m_panel->m_showAGCLines) {
                m_panel->m_showAGCLines = false;
                m_panel->set->setAGCShowLines(m_panel->m_receiver, false);
            } else {
                m_panel->m_showAGCLines = true;
                m_panel->set->setAGCShowLines(m_panel->m_receiver, true);
            }
        }
    }
    else if (m_panel->m_mouseRegion == QGLReceiverPanel::panadapterRegion || m_panel->m_mouseRegion == QGLReceiverPanel::waterfallRegion) {
        if (event->buttons() == Qt::LeftButton && m_panel->m_receiver != m_panel->set->getCurrentReceiver()) {
            m_panel->set->setCurrentReceiver(m_panel->m_receiver);
        }
        else if (event->buttons() == Qt::LeftButton) {
            m_panel->m_crossHairCursor = false;
            if (m_panel->cursor().shape() != Qt::OpenHandCursor)
                m_panel->setCursor(Qt::OpenHandCursor);
            m_panel->m_dragMouse = true;
        }
        else if (event->buttons() == Qt::RightButton) {
            m_panel->showRadioPopup(true);
        }
    }
    else if (m_panel->m_mouseRegion == QGLReceiverPanel::filterRegion) {
        if (event->buttons() == Qt::LeftButton) {
            m_panel->m_highlightFilter = true;
            m_panel->m_mouseDownFilterFrequencyLo = m_panel->m_filterLowerFrequency;
            m_panel->m_mouseDownFilterFrequencyHi = m_panel->m_filterUpperFrequency;
        }
    }
    else if (m_panel->m_mouseRegion == QGLReceiverPanel::filterRegionLow || m_panel->m_mouseRegion == QGLReceiverPanel::filterRegionHigh) {
        if (event->buttons() == Qt::LeftButton) {
            m_panel->m_mouseDownFilterFrequencyLo = m_panel->m_filterLowerFrequency;
            m_panel->m_mouseDownFilterFrequencyHi = m_panel->m_filterUpperFrequency;
        }
    }
    else if (m_panel->m_mouseRegion == QGLReceiverPanel::freqScalePanadapterRegion) {
        m_panel->m_rulerMouseDownPos = m_panel->m_freqScalePanRect.topLeft();
        if (event->buttons() == Qt::LeftButton || event->buttons() == Qt::RightButton)
            m_panel->m_dragFreqScale = true;
        m_panel->m_dragFreqScaleZoom = (event->button() == Qt::RightButton);
        if (event->buttons() == Qt::RightButton)
            m_panel->setCursor(Qt::SplitHCursor);
        return;
    }
    else if (m_panel->m_mouseRegion == QGLReceiverPanel::dBmScalePanadapterRegion) {
        m_panel->m_rulerMouseDownPos = m_panel->m_dBmScalePanRect.topLeft();
        if (event->buttons() == Qt::LeftButton)
            m_panel->m_dragDBmScale = true;
        else if (event->buttons() == Qt::RightButton)
            m_panel->setCursor(Qt::SplitVCursor);
        return;
    }
}

void PanadapterInputController::handleMouseRelease(QMouseEvent *event) {
    if (!m_panel) return;

    if (m_panel->m_dragCwText) {
        m_panel->m_dragCwText = false;
        if (m_panel->cursor().shape() != Qt::ArrowCursor)
            m_panel->setCursor(Qt::ArrowCursor);
        m_panel->update();
        event->accept();
        return;
    }

    m_panel->m_mousePos = event->pos();
    m_panel->m_mouseDownPos = m_panel->m_mousePos;

    getRegion(m_panel->m_mousePos);

    if (m_panel->m_mouseRegion == QGLReceiverPanel::freqScalePanadapterRegion) {
        m_panel->m_dragFreqScale = false;
        m_panel->m_dragFreqScaleZoom = false;
        m_panel->m_freqScalePanadapterUpdate = true;
        if (m_panel->m_crossHair) {
            if (m_panel->cursor().shape() != Qt::BlankCursor)
                m_panel->setCursor(Qt::BlankCursor);
        } else if (m_panel->cursor().shape() != Qt::ArrowCursor) {
            m_panel->setCursor(Qt::ArrowCursor);
        }
        m_panel->update();
        return;
    }

    const bool wasDragging = m_panel->m_dragMouse || m_panel->m_highlightFilter
                             || m_panel->m_showFilterLeftBoundary || m_panel->m_showFilterRightBoundary;
    m_panel->m_dragMouse = false;
    m_panel->m_dragDBmScale = false;
    m_panel->m_dragFreqScale = false;
    m_panel->m_dragFreqScaleZoom = false;
    m_panel->m_showFilterLeftBoundary = false;
    m_panel->m_showFilterRightBoundary = false;
    m_panel->m_highlightFilter = false;
    m_panel->m_freqScalePanadapterUpdate = true;
    m_panel->m_dBmScalePanadapterUpdate = true;
    m_panel->m_crossHairCursor = true;
    if (m_panel->m_crossHair) {
        if (m_panel->cursor().shape() != Qt::BlankCursor)
            m_panel->setCursor(Qt::BlankCursor);
    } else if (m_panel->cursor().shape() != Qt::ArrowCursor) {
        m_panel->setCursor(Qt::ArrowCursor);
    }

    if (wasDragging)
        m_panel->update();
}

void PanadapterInputController::handleMouseDoubleClick(QMouseEvent *event) {
    if (!m_panel) return;

    m_panel->m_mousePos = event->pos();
    m_panel->m_mouseDownPos = m_panel->m_mousePos;

    getRegion(m_panel->m_mousePos);

    if (event->button() == Qt::LeftButton && m_panel->m_panFreqRect.isValid() && m_panel->m_panFreqRect.contains(m_panel->m_mousePos)) {
        FrequencyEntryDialog dlg(m_panel->m_vfoFrequency, m_panel);
        if (dlg.exec() == QDialog::Accepted) {
            const qint64 newFreq = dlg.frequency();
            if (newFreq < static_cast<qint64>(m_panel->set->getMaxFrequency()) && newFreq >= 0) {
                m_panel->setVFOFrequency(0, m_panel->m_receiver, newFreq);
            }
        }
        event->accept();
        return;
    }

    if (m_panel->m_mouseRegion == QGLReceiverPanel::panadapterRegion || m_panel->m_mouseRegion == QGLReceiverPanel::waterfallRegion) {
        if (event->button() == Qt::LeftButton) {
            const int dx = m_panel->m_panRect.width()/2 - m_panel->m_mousePos.x();
            const qreal unit = m_panel->displayedFrequencySpanHz() / m_panel->m_panRect.width();
            qint64 clickedFreq = (qint64)(qRound(m_panel->m_centerFrequency - (unit * dx)));

            const DSPMode mode = m_panel->m_sliceModel ? m_panel->m_sliceModel->dspMode() : m_panel->m_dspMode;
            const bool isCw = (mode == DSPMode::CWL || mode == DSPMode::CWU);
            const int cwPitch = m_panel->set->getCwSidetoneFreq();

            // Auto-snap to nearest spectral peak within ±35 pixels
            bool peakFound = false;
            const qint64 peakRf = m_panel->findPeakFrequencyNear(clickedFreq, qMax(800, qRound(unit * 35.0)), &peakFound);

            qint64 newVfo = clickedFreq;
            if (peakFound && isCw) {
                // Zero-beat snap for CW
                newVfo = (mode == DSPMode::CWL) ? (peakRf + cwPitch) : (peakRf - cwPitch);
            } else if (peakFound) {
                newVfo = peakRf;
            }

            newVfo = qBound(m_panel->m_centerFrequency - m_panel->m_sampleRate/2, newVfo, m_panel->m_centerFrequency + m_panel->m_sampleRate/2);

            m_panel->m_vfoFrequency = newVfo;
            m_panel->m_deltaFrequency = m_panel->m_centerFrequency - m_panel->m_vfoFrequency;
            m_panel->m_deltaF = (qreal)(1.0 * m_panel->m_deltaFrequency / m_panel->m_sampleRate);

            m_panel->set->setVFOFrequency(0, m_panel->m_receiver, m_panel->m_vfoFrequency);
            m_panel->update();
            event->accept();
            return;
        }
    }
}

void PanadapterInputController::handleMouseMove(QMouseEvent* event) {
    if (!m_panel) return;

    m_panel->m_mousePos = event->pos();

    if (m_panel->m_dragCwText && (event->buttons() & Qt::LeftButton)) {
        const QPoint delta = event->pos() - m_panel->m_cwDragStartMouse;
        m_panel->m_cwDragStartMouse = event->pos();
        m_panel->m_cwBoxPos += delta;
        const int w = m_panel->m_cwTextRect.width() > 0 ? m_panel->m_cwTextRect.width() : 200;
        const int h = m_panel->m_cwTextRect.height() > 0 ? m_panel->m_cwTextRect.height() : 24;
        m_panel->m_cwBoxPos.setX(qBound(m_panel->m_panRect.left() + 4, m_panel->m_cwBoxPos.x(), m_panel->m_panRect.right() - w - 4));
        m_panel->m_cwBoxPos.setY(qBound(m_panel->m_panRect.top() + 4, m_panel->m_cwBoxPos.y(), m_panel->m_panRect.bottom() - h - 4));
        m_panel->update();
        event->accept();
        return;
    }

    if (event->buttons() == Qt::NoButton) {
        getRegion(m_panel->m_mousePos);
        if (m_panel->m_cwTextRect.isValid() && m_panel->m_cwTextRect.contains(m_panel->m_mousePos)) {
            if (m_panel->cursor().shape() != Qt::OpenHandCursor)
                m_panel->setCursor(Qt::OpenHandCursor);
        }
    }

    switch (m_panel->m_mouseRegion) {
        case QGLReceiverPanel::agcThresholdLine: {
            if (!m_panel->m_showAGCLines || (m_panel->m_agcMode == (AGCMode)agcOFF))
                break;

            m_panel->m_crossHairCursor = false;
            if (m_panel->cursor().shape() != Qt::SizeVerCursor)
                m_panel->setCursor(Qt::SizeVerCursor);

            if (event->buttons() == Qt::LeftButton) {
                QPoint dPos = m_panel->m_mouseDownPos - m_panel->m_mousePos;
                qreal unit = qAbs(m_panel->m_dBmPanMax - m_panel->m_dBmPanMin) / m_panel->m_panRect.height();
                qreal dAGCThreshold = dPos.y() * unit;

                m_panel->m_agcThresholdNew = m_panel->m_mouseDownAGCThreshold + dAGCThreshold;
                if (m_panel->m_agcThresholdNew > m_panel->m_dBmPanMax - 2)
                    m_panel->m_agcThresholdNew = m_panel->m_dBmPanMax - 2;
                if (m_panel->m_agcThresholdNew < m_panel->m_dBmPanMin + 2)
                    m_panel->m_agcThresholdNew = m_panel->m_dBmPanMin + 2;

                m_panel->set->setAGCThreshold_dB(m_panel->m_receiver, m_panel->m_agcThresholdNew);
            }
            break;
        }

        case QGLReceiverPanel::agcHangLine: {
            if (!m_panel->m_showAGCLines || (m_panel->m_agcMode == (AGCMode)agcOFF) || !m_panel->m_agcHangEnabled)
                break;

            m_panel->m_crossHairCursor = false;
            if (m_panel->cursor().shape() != Qt::SizeVerCursor)
                m_panel->setCursor(Qt::SizeVerCursor);

            if (event->buttons() == Qt::LeftButton) {
                QPoint dPos = m_panel->m_mouseDownPos - m_panel->m_mousePos;
                qreal unit = qAbs(m_panel->m_dBmPanMax - m_panel->m_dBmPanMin) / m_panel->m_panRect.height();
                qreal dAGCThreshold = dPos.y() * unit;

                m_panel->m_agcHangLevelNew = m_panel->m_mouseDownAGCHangLevel + dAGCThreshold;
                if (m_panel->m_agcHangLevelNew > m_panel->m_dBmPanMax - 2)
                    m_panel->m_agcHangLevelNew = m_panel->m_dBmPanMax - 2;
                if (m_panel->m_agcHangLevelNew < m_panel->m_dBmPanMin + 2)
                    m_panel->m_agcHangLevelNew = m_panel->m_dBmPanMin + 2;

                m_panel->set->setAGCHangThreshold(m_panel->m_receiver, m_panel->m_agcHangLevelNew);
            }
            break;
        }

        case QGLReceiverPanel::agcFixedGainLine: {
            if (!m_panel->m_showAGCLines || (m_panel->m_agcMode != (AGCMode)agcOFF))
                break;

            m_panel->m_crossHairCursor = false;
            if (m_panel->cursor().shape() != Qt::SizeVerCursor)
                m_panel->setCursor(Qt::SizeVerCursor);

            if (event->buttons() == Qt::LeftButton) {
                QPoint dPos = m_panel->m_mouseDownPos - m_panel->m_mousePos;
                qreal unit = qAbs(m_panel->m_dBmPanMax - m_panel->m_dBmPanMin) / m_panel->m_panRect.height();
                qreal dAGCFixedGain = dPos.y() * unit;

                qreal agcFixedGain = m_panel->m_mouseDownFixedGainLevel + dAGCFixedGain;
                if (agcFixedGain > m_panel->m_dBmPanMax - 2)
                    agcFixedGain = m_panel->m_dBmPanMax - 2;
                if (agcFixedGain < m_panel->m_dBmPanMin + 2)
                    agcFixedGain = m_panel->m_dBmPanMin + 2;

                m_panel->set->setAGCFixedGain_dB(m_panel->m_receiver, -agcFixedGain);
            }
            break;
        }

        case QGLReceiverPanel::panadapterRegion:
        case QGLReceiverPanel::waterfallRegion: {
            if (!m_panel->m_dragMouse) {
                m_panel->m_crossHairCursor = true;
                if (m_panel->m_crossHair) {
                    if (m_panel->cursor().shape() != Qt::BlankCursor)
                        m_panel->setCursor(Qt::BlankCursor);
                } else if (m_panel->cursor().shape() != Qt::ArrowCursor) {
                    m_panel->setCursor(Qt::ArrowCursor);
                }
            }

            if (m_panel->m_dragMouse && (event->buttons() == Qt::LeftButton)) {
                QPoint dPos = m_panel->m_mouseDownPos - m_panel->m_mousePos;
                qreal unit = m_panel->displayedFrequencySpanHz() / m_panel->m_freqScalePanRect.width();
                qreal deltaFreq = unit * dPos.x();

                long newFrequency = m_panel->m_centerFrequency + deltaFreq;
                if (newFrequency > m_panel->set->getMaxFrequency())
                    newFrequency = m_panel->set->getMaxFrequency();
                else if (newFrequency < m_panel->set->getMinFrequency())
                    newFrequency = m_panel->set->getMinFrequency();
                else if (newFrequency + deltaFreq < 0)
                    newFrequency = 0;
                else {
                    if (m_panel->m_panLocked) {
                        if (m_panel->m_vfoFrequency > m_panel->m_centerFrequency + m_panel->m_sampleRate/2)
                            m_panel->m_vfoFrequency = m_panel->m_centerFrequency + m_panel->m_sampleRate/2;
                        else if (m_panel->m_vfoFrequency < m_panel->m_centerFrequency - m_panel->m_sampleRate/2)
                            m_panel->m_vfoFrequency = m_panel->m_centerFrequency - m_panel->m_sampleRate/2;

                        m_panel->m_vfoFrequency -= deltaFreq;
                    }
                    else {
                        m_panel->m_centerFrequency += deltaFreq;
                    }
                }

                if (m_panel->m_panLocked) {
                    m_panel->set->setVFOFrequency(0, m_panel->m_receiver, m_panel->m_vfoFrequency);
                    m_panel->m_deltaFrequency = m_panel->m_centerFrequency - m_panel->m_vfoFrequency;
                    m_panel->m_deltaF = (qreal)(1.0 * m_panel->m_deltaFrequency / m_panel->m_sampleRate);
                    m_panel->m_freqScalePanadapterUpdate = true;
                    m_panel->m_panGridUpdate = true;
                }
                else {
                    m_panel->m_vfoFrequency = m_panel->m_centerFrequency - m_panel->m_deltaFrequency;
                    m_panel->m_freqScalePanadapterUpdate = true;
                    m_panel->m_panGridUpdate = true;
                    m_panel->set->setVFOFrequency(0, m_panel->m_receiver, m_panel->m_vfoFrequency);
                    m_panel->set->setCtrFrequency(0, m_panel->m_receiver, m_panel->m_centerFrequency);
                }

                m_panel->m_mouseDownPos = m_panel->m_mousePos;
                m_panel->m_displayCenterlineHeight = m_panel->m_panRect.top() + (m_panel->m_panRect.height() - 3);
                m_panel->m_showFilterLeftBoundary = false;
                m_panel->m_showFilterRightBoundary = false;
                m_panel->m_highlightFilter = false;
                m_panel->update();
            }

            m_panel->m_displayCenterlineHeight = m_panel->m_panRect.top() + (m_panel->m_panRect.height() - 3);
            m_panel->m_showFilterLeftBoundary = false;
            m_panel->m_showFilterRightBoundary = false;
            m_panel->m_highlightFilter = false;
            break;
        }

        case QGLReceiverPanel::dBmScalePanadapterRegion: {
            if (event->buttons() == Qt::LeftButton) {
                m_panel->m_dragDBmScale = true;
                QPoint dPos = m_panel->m_mouseDownPos - m_panel->m_mousePos;
                qreal unit = (qreal)(qAbs(m_panel->m_dBmPanMax - m_panel->m_dBmPanMin) / m_panel->m_panRect.height()) * 1.5;

                qreal newMin = m_panel->m_dBmPanMin - unit * dPos.y();
                qreal newMax = m_panel->m_dBmPanMax - unit * dPos.y();

                if (newMin > MINDBM && newMax < MAXDBM) {
                    m_panel->m_dBmPanMin = newMin;
                    m_panel->m_dBmPanMax = newMax;
                    m_panel->set->setdBmPanScaleMin(m_panel->m_receiver, m_panel->m_dBmPanMin);
                    m_panel->set->setdBmPanScaleMax(m_panel->m_receiver, m_panel->m_dBmPanMax);
                }

                m_panel->m_mouseDownPos = m_panel->m_mousePos;
                m_panel->m_dBmScalePanadapterUpdate = true;
            }
            else if (event->buttons() == Qt::RightButton && event->modifiers() == Qt::ControlModifier) {
                m_panel->m_dragDBmScale = true;
                QPoint dPos = m_panel->m_mouseDownPos - m_panel->m_mousePos;
                if (dPos.y() > 0)
                    m_panel->m_dBmPanDelta = 0.5f;
                else if (dPos.y() < 0)
                    m_panel->m_dBmPanDelta = -0.5f;

                m_panel->m_dBmPanMin += m_panel->m_dBmPanDelta;
                m_panel->m_dBmPanMax -= m_panel->m_dBmPanDelta;

                if (qAbs(m_panel->m_dBmPanMax - m_panel->m_dBmPanMin) < 10) {
                    m_panel->m_dBmPanMin -= m_panel->m_dBmPanDelta;
                    m_panel->m_dBmPanMax += m_panel->m_dBmPanDelta;
                }
                if (m_panel->m_dBmPanMin < MINDBM) m_panel->m_dBmPanMin = MINDBM;
                if (m_panel->m_dBmPanMax > MAXDBM) m_panel->m_dBmPanMax = MAXDBM;

                m_panel->set->setdBmPanScaleMin(m_panel->m_receiver, m_panel->m_dBmPanMin);
                m_panel->set->setdBmPanScaleMax(m_panel->m_receiver, m_panel->m_dBmPanMax);
                m_panel->m_mouseDownPos = m_panel->m_mousePos;
                m_panel->m_dBmScalePanadapterUpdate = true;
            }
            else if (event->buttons() == Qt::RightButton) {
                m_panel->m_dragDBmScale = true;
                QPoint dPos = m_panel->m_mouseDownPos - m_panel->m_mousePos;
                if (dPos.y() > 0)
                    m_panel->m_dBmPanDelta = 0.5f;
                else if (dPos.y() < 0)
                    m_panel->m_dBmPanDelta = -0.5f;

                m_panel->m_dBmPanMax -= m_panel->m_dBmPanDelta;

                if (qAbs(m_panel->m_dBmPanMax - m_panel->m_dBmPanMin) < 10) {
                    m_panel->m_dBmPanMin -= m_panel->m_dBmPanDelta;
                    m_panel->m_dBmPanMax += m_panel->m_dBmPanDelta;
                }
                if (m_panel->m_dBmPanMin < MINDBM) m_panel->m_dBmPanMin = MINDBM;
                if (m_panel->m_dBmPanMax > MAXDBM) m_panel->m_dBmPanMax = MAXDBM;

                m_panel->set->setdBmPanScaleMin(m_panel->m_receiver, m_panel->m_dBmPanMin);
                m_panel->set->setdBmPanScaleMax(m_panel->m_receiver, m_panel->m_dBmPanMax);
                m_panel->m_mouseDownPos = m_panel->m_mousePos;
                m_panel->m_dBmScalePanadapterUpdate = true;
            }
            else {
                m_panel->setCursor(Qt::ArrowCursor);
            }
            break;
        }

        case QGLReceiverPanel::freqScalePanadapterRegion: {
            if (event->buttons() != Qt::NoButton)
                m_panel->m_dragFreqScale = true;
            if (event->buttons() == Qt::RightButton)
                m_panel->m_dragFreqScaleZoom = true;
            else if (event->buttons() == Qt::LeftButton)
                m_panel->m_dragFreqScaleZoom = false;

            if (event->buttons() == Qt::LeftButton && event->modifiers() == Qt::ShiftModifier) {
                QPoint dPos = m_panel->m_mouseDownPos - m_panel->m_mousePos;
                int bottom_y = m_panel->height() - m_panel->m_freqScalePanRect.height();
                int new_y = m_panel->m_rulerMouseDownPos.y() - dPos.y();

                if (new_y < m_panel->m_panRect.top() + m_panel->m_panSpectrumMinimumHeight)
                    new_y = m_panel->m_panRect.top() + m_panel->m_panSpectrumMinimumHeight;
                if (new_y > bottom_y)
                    new_y = bottom_y;

                m_panel->m_freqRulerPosition = (float)(new_y - m_panel->m_panRect.top()) / (bottom_y - m_panel->m_panRect.top());
                m_panel->set->setFreqRulerPosition(m_panel->m_receiver, m_panel->m_freqRulerPosition);
            }
            else if (event->buttons() == Qt::LeftButton) {
                QPoint dPos = m_panel->m_mouseDownPos - m_panel->m_mousePos;
                qreal unit = m_panel->displayedFrequencySpanHz() / m_panel->m_freqScalePanRect.width();
                qreal deltaFreq = unit * dPos.x();

                long newFrequency = m_panel->m_centerFrequency + deltaFreq;
                if (newFrequency > m_panel->set->getMaxFrequency())
                    newFrequency = m_panel->set->getMaxFrequency();
                else if (newFrequency < m_panel->set->getMinFrequency())
                    newFrequency = m_panel->set->getMinFrequency();
                else if (newFrequency + deltaFreq < 0)
                    newFrequency = 0;
                else {
                    m_panel->m_centerFrequency += deltaFreq;
                }

                if (!m_panel->m_panLocked) {
                    m_panel->m_vfoFrequency = m_panel->m_centerFrequency - m_panel->m_deltaFrequency;
                    m_panel->set->setVFOFrequency(0, m_panel->m_receiver, m_panel->m_vfoFrequency);
                    m_panel->set->setCtrFrequency(0, m_panel->m_receiver, m_panel->m_centerFrequency);
                }
                else {
                    m_panel->m_deltaFrequency = m_panel->m_centerFrequency - m_panel->m_vfoFrequency;
                    m_panel->m_deltaF = (qreal)(1.0 * m_panel->m_deltaFrequency / m_panel->m_sampleRate);

                    if (m_panel->m_sliceModel) {
                        m_panel->m_sliceModel->setCenterFrequency(m_panel->m_centerFrequency);
                        m_panel->set->setNCOFrequency(true, m_panel->m_receiver, -m_panel->m_deltaFrequency);
                    } else {
                        qreal vol = m_panel->set->getMainVolume(m_panel->m_receiver);
                        m_panel->set->setMainVolume(m_panel->m_receiver, 0.0f);
                        m_panel->set->setCtrFrequency(0, m_panel->m_receiver, m_panel->m_centerFrequency);
                        m_panel->set->setNCOFrequency(true, m_panel->m_receiver, -m_panel->m_deltaFrequency);
                        m_panel->set->setMainVolume(m_panel->m_receiver, vol);
                    }
                }

                m_panel->m_mouseDownPos = m_panel->m_mousePos;
                m_panel->m_displayCenterlineHeight = m_panel->m_panRect.top() + (m_panel->m_panRect.height() - 3);
                m_panel->m_showFilterLeftBoundary = false;
                m_panel->m_showFilterRightBoundary = false;
                m_panel->m_highlightFilter = false;
            }
            else if (event->buttons() == Qt::RightButton) {
                QPoint dPos = m_panel->m_mouseDownPos - m_panel->m_mousePos;
                if (dPos.x() > 0)
                    m_panel->m_freqScaleZoomFactor += 0.01;
                else if (dPos.x() < 0)
                    m_panel->m_freqScaleZoomFactor -= 0.01;

                if (m_panel->m_freqScaleZoomFactor > 1.0) m_panel->m_freqScaleZoomFactor = 1.0;
                if (m_panel->m_freqScaleZoomFactor < 0.05) m_panel->m_freqScaleZoomFactor = 0.05;

                m_panel->m_mouseDownPos = m_panel->m_mousePos;
                m_panel->m_freqScalePanadapterUpdate = true;
                m_panel->m_panGridUpdate = true;
                m_panel->recomputeDisplayBinsFromCache();
                m_panel->update();
            }
            else {
                m_panel->setCursor(Qt::ArrowCursor);
            }

            m_panel->m_showFilterLeftBoundary = false;
            m_panel->m_showFilterRightBoundary = false;
            m_panel->m_highlightFilter = false;
            break;
        }

        case QGLReceiverPanel::filterRegionLow: {
            m_panel->setCursor(Qt::SizeHorCursor);
            m_panel->m_showFilterLeftBoundary = true;
            if (event->buttons() == Qt::LeftButton) {
                QPoint dPos = m_panel->m_mouseDownPos - m_panel->m_mousePos;
                qreal dFreq = dPos.x() * m_panel->displayedFrequencySpanHz() / m_panel->m_panRect.width();
                m_panel->m_filterLowerFrequency = qRound(m_panel->m_mouseDownFilterFrequencyLo - dFreq);
                m_panel->set->setRXFilter(m_panel->m_receiver, m_panel->m_filterLowerFrequency, m_panel->m_filterUpperFrequency);
            }
            m_panel->m_highlightFilter = false;
            break;
        }

        case QGLReceiverPanel::filterRegionHigh: {
            m_panel->setCursor(Qt::SizeHorCursor);
            m_panel->m_showFilterRightBoundary = true;
            if (event->buttons() == Qt::LeftButton) {
                QPoint dPos = m_panel->m_mouseDownPos - m_panel->m_mousePos;
                qreal dFreq = dPos.x() * m_panel->displayedFrequencySpanHz() / m_panel->m_panRect.width();
                m_panel->m_filterUpperFrequency = qRound(m_panel->m_mouseDownFilterFrequencyHi - dFreq);
                m_panel->set->setRXFilter(m_panel->m_receiver, m_panel->m_filterLowerFrequency, m_panel->m_filterUpperFrequency);
            }
            m_panel->m_highlightFilter = false;
            break;
        }

        case QGLReceiverPanel::filterRegion: {
            m_panel->setCursor(Qt::SizeAllCursor);
            m_panel->m_displayCenterlineHeight = m_panel->m_panRect.top() + (m_panel->size().height() - 3);

            if (event->buttons() == Qt::LeftButton) {
                m_panel->m_highlightFilter = true;
                QPoint dPos = m_panel->m_mouseDownPos - m_panel->m_mousePos;
                qreal dFreq = dPos.x() * m_panel->displayedFrequencySpanHz() / m_panel->m_panRect.width();
                m_panel->m_filterUpperFrequency = qRound(m_panel->m_mouseDownFilterFrequencyHi - dFreq);
                m_panel->m_filterLowerFrequency = qRound(m_panel->m_mouseDownFilterFrequencyLo - dFreq);
                m_panel->set->setRXFilter(m_panel->m_receiver, m_panel->m_filterLowerFrequency, m_panel->m_filterUpperFrequency);
            }
            m_panel->m_showFilterLeftBoundary = false;
            m_panel->m_showFilterRightBoundary = false;
            break;
        }

        case QGLReceiverPanel::elsewhere:
        default:
            break;
    }
}

void PanadapterInputController::handleWheel(QWheelEvent* event) {
    if (!m_panel) return;

    getRegion(event->position().toPoint());
    if (m_panel->m_panFreqRect.isValid() && m_panel->m_panFreqRect.contains(event->position().toPoint()))
        m_panel->m_mouseRegion = QGLReceiverPanel::panadapterRegion;

    double freqStep = m_panel->set->getMouseWheelFreqStep(m_panel->m_currentReceiver);

    switch (m_panel->m_mouseRegion) {
        case QGLReceiverPanel::panadapterRegion:
        case QGLReceiverPanel::waterfallRegion:
        case QGLReceiverPanel::filterRegion:
        case QGLReceiverPanel::filterRegionLow:
        case QGLReceiverPanel::filterRegionHigh: {
            double delta = 0;
            if (event->angleDelta().y() < 0)
                delta = -freqStep;
            else if (event->angleDelta().y() > 0)
                delta = freqStep;

            if (!m_panel->m_panLocked) {
                if (m_panel->m_centerFrequency + delta > m_panel->set->getMaxFrequency())
                    m_panel->m_centerFrequency = m_panel->set->getMaxFrequency();
                else if (m_panel->m_centerFrequency + delta < m_panel->set->getMinFrequency())
                    m_panel->m_centerFrequency = m_panel->set->getMinFrequency();
                else
                    m_panel->m_centerFrequency = (long)(qRound((m_panel->m_centerFrequency + delta) / qAbs(freqStep)) * qAbs(freqStep));

                m_panel->m_vfoFrequency = m_panel->m_centerFrequency - m_panel->m_deltaFrequency;
            }
            else {
                if (m_panel->m_vfoFrequency + delta > m_panel->m_centerFrequency + m_panel->m_sampleRate/2)
                    m_panel->m_vfoFrequency = m_panel->m_centerFrequency + m_panel->m_sampleRate/2;
                else if (m_panel->m_vfoFrequency + delta < m_panel->m_centerFrequency - m_panel->m_sampleRate/2)
                    m_panel->m_vfoFrequency = m_panel->m_centerFrequency - m_panel->m_sampleRate/2;
                else
                    m_panel->m_vfoFrequency = (long)(qRound((m_panel->m_vfoFrequency + delta) / qAbs(freqStep)) * qAbs(freqStep));

                m_panel->m_deltaFrequency = m_panel->m_centerFrequency - m_panel->m_vfoFrequency;
                m_panel->m_deltaF = (qreal)(1.0 * m_panel->m_deltaFrequency / m_panel->m_sampleRate);
            }

            m_panel->set->setCtrFrequency(0, m_panel->m_receiver, m_panel->m_centerFrequency);
            m_panel->set->setVFOFrequency(0, m_panel->m_receiver, m_panel->m_vfoFrequency);
            break;
        }
        default:
            break;
    }
}
