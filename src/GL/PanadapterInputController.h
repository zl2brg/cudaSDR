/**
 * @file  PanadapterInputController.h
 * @brief Controller for mouse, wheel, keyboard, and gesture input on the OpenGL receiver panel.
 * @author Simon Eatough <simon.eatough@gmail.com>
 * @date 2026-09-08
 */

#ifndef PANADAPTER_INPUT_CONTROLLER_H
#define PANADAPTER_INPUT_CONTROLLER_H

#include <QObject>
#include <QPoint>
#include <QRect>

class QGLReceiverPanel;
class QMouseEvent;
class QWheelEvent;
class QEnterEvent;
class QEvent;

class PanadapterInputController : public QObject {
    Q_OBJECT

public:
    enum Region {
        freqScalePanadapterRegion,
        panadapterRegion,
        dBmScalePanadapterRegion,
        waterfallRegion,
        filterRegion,
        filterRegionLow,
        filterRegionHigh,
        agcButtonRegion,
        agcThresholdLine,
        agcHangLine,
        agcFixedGainLine,
        elsewhere,
        out
    };
    Q_ENUM(Region)

    explicit PanadapterInputController(QGLReceiverPanel *panel);
    ~PanadapterInputController() override = default;

    void handleEnter(QEnterEvent *event);
    void handleLeave(QEvent *event);
    void handleMousePress(QMouseEvent *event);
    void handleMouseRelease(QMouseEvent *event);
    void handleMouseDoubleClick(QMouseEvent *event);
    void handleMouseMove(QMouseEvent *event);
    void handleWheel(QWheelEvent *event);

    void getRegion(const QPoint &p);

    // Pure mathematical calculation helpers (for testability and clarity)
    static Region determineRegion(
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
        bool isShiftPressed);

    static qint64 calculateWheelFrequency(
        qint64 currentFreq,
        int angleDeltaY,
        double freqStep,
        qint64 minFreq,
        qint64 maxFreq);

    static qint64 calculatePanDragCenterFreq(
        qint64 currentCenterFreq,
        int deltaX,
        qreal spanHz,
        int rectWidth,
        qint64 minFreq,
        qint64 maxFreq);

    static void calculateFilterDragEdges(
        int deltaX,
        qreal spanHz,
        int rectWidth,
        qreal initialLo,
        qreal initialHi,
        bool dragLow,
        bool dragHigh,
        qreal &outLo,
        qreal &outHi);

    static void calculateDbmScaleDrag(
        int deltaY,
        int rectHeight,
        qreal initialMin,
        qreal initialMax,
        qreal &outMin,
        qreal &outMax);

private:
    QGLReceiverPanel *m_panel;
};

#endif // PANADAPTER_INPUT_CONTROLLER_H
