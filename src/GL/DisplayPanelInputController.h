/**
 * @file  DisplayPanelInputController.h
 * @brief Controller for mouse, wheel, keyboard, and digit hit-testing on OGLDisplayPanel.
 * @author Simon Eatough <simon.eatough@gmail.com>
 * @date 2026-09-09
 */

#ifndef DISPLAY_PANEL_INPUT_CONTROLLER_H
#define DISPLAY_PANEL_INPUT_CONTROLLER_H

#include <QObject>
#include <QPoint>
#include <QRect>
#include <QRegion>
#include <QString>
#include "cusdr_oglDisplayPanel.h"

class OGLDisplayPanel;
class QMouseEvent;
class QWheelEvent;
class QEvent;

class DisplayPanelInputController : public QObject {
    Q_OBJECT

public:
    explicit DisplayPanelInputController(OGLDisplayPanel *panel);
    ~DisplayPanelInputController() override = default;

    void handleEnter(QEvent *event);
    void handleLeave(QEvent *event);
    void handleMousePress(QMouseEvent *event);
    void handleMouseRelease(QMouseEvent *event);
    void handleMouseDoubleClick(QMouseEvent *event);
    void handleMouseMove(QMouseEvent *event);
    void handleWheel(QWheelEvent *event);

    void rebuildAllFreqDigitHitRegions();
    void getSelectedDigit(const QPoint &p);

    int digitPosition() const { return m_digitPosition; }
    int digitVfo() const { return m_digitVfo; }

    QRect vfoLabelRect(int yBaseline) const;
    qint64 vfoMemoryHz(OGLDisplayPanel::DigitVfo which) const;
    void activateDigitVfo(OGLDisplayPanel::DigitVfo which);
    void tuneDigitVfoTo(OGLDisplayPanel::DigitVfo which, qint64 frequencyHz);

    // Static math and calculation helpers for testing and clarity
    static qint64 calculateDigitDelta(int digitPosition);
    static qint64 calculateNewFrequency(qint64 currentFreq, int digitPosition, int numSteps, qint64 maxFreq);
    static qreal cycleFreqStep(int digitPosition, qreal currentStep);
    static bool hitTestDigit(const OGLDisplayPanel::FreqDigitHitRegions &regs, const QString &f1str,
                             const QPoint &p, int *digitOut);
    static void updateFreqDigitHitRegions(
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
        int freq2Height);

private:
    OGLDisplayPanel *m_panel = nullptr;

    OGLDisplayPanel::FreqDigitHitRegions m_hitA;
    OGLDisplayPanel::FreqDigitHitRegions m_hitB;

    int m_digitPosition = OGLDisplayPanel::None;
    int m_digitVfo = OGLDisplayPanel::DigitVfoNone;
};

#endif // DISPLAY_PANEL_INPUT_CONTROLLER_H
