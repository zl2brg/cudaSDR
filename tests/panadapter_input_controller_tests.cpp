/**
 * @file  panadapter_input_controller_tests.cpp
 * @brief Unit tests for PanadapterInputController logic, region hit testing, and tuning math.
 * @author Simon Eatough <simon.eatough@gmail.com>
 * @date 2026-09-08
 */

#include <QtTest/QtTest>
#include "GL/PanadapterInputController.h"
#include "cusdr_settings.h"

class PanadapterInputControllerTests : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // Region hit tests
    void testRegionDetectionAgcButton();
    void testRegionDetectionFreqScale();
    void testRegionDetectionDbmScale();
    void testRegionDetectionFilterLowEdge();
    void testRegionDetectionFilterHighEdge();
    void testRegionDetectionFilterWholePassband();
    void testRegionDetectionAgcLines();
    void testRegionDetectionPanAndWaterfall();
    void testRegionDetectionElsewhere();

    // Mathematical calculation helpers
    void testCalculateWheelFrequencyStepUp();
    void testCalculateWheelFrequencyStepDown();
    void testCalculateWheelFrequencyClamping();
    void testCalculatePanDragCenterFreq();
    void testCalculateFilterDragEdges();
    void testCalculateDbmScaleDrag();
};

#include "GL/cusdr_oglReceiverPanel.h"

qreal QGLReceiverPanel::displayedFrequencySpanHz() const {
    return 192000.0;
}

qint64 QGLReceiverPanel::findPeakFrequencyNear(qint64 targetFreq, int, bool *found) const {
    if (found) *found = false;
    return targetFreq;
}

void QGLReceiverPanel::recomputeDisplayBinsFromCache()
{
}

void QGLReceiverPanel::showRadioPopup(bool)
{
}

void QGLReceiverPanel::setVFOFrequency(int, int, qint64)
{
}

void PanadapterInputControllerTests::initTestCase()
{
}

void PanadapterInputControllerTests::cleanupTestCase()
{
    Settings::delete_instance();
}

void PanadapterInputControllerTests::testRegionDetectionAgcButton()
{
    const QRect panRect(0, 0, 1000, 400);
    const QRect waterfallRect(0, 400, 1000, 400);
    const QRect freqScaleRect(0, 370, 1000, 30);
    const QRect dBmScaleRect(0, 0, 40, 370);
    const QRect filterRect(450, 0, 100, 370);
    const QRect secScaleWaterfallRect(0, 400, 40, 400);
    const QRect agcButtonRect(50, 10, 40, 20);
    const QRect panSMeterRect(800, 10, 150, 40);
    const QRect panFreqRect(800, 60, 150, 30);

    const int snapMouse = 4;
    const qreal agcThresh = 200.0;
    const qreal agcHang = 250.0;
    const qreal agcFixed = 300.0;

    // Inside AGC button
    auto r = PanadapterInputController::determineRegion(
        QPoint(60, 20), panRect, waterfallRect, freqScaleRect, dBmScaleRect,
        filterRect, secScaleWaterfallRect, agcButtonRect, panSMeterRect, panFreqRect,
        snapMouse, agcThresh, agcHang, agcFixed, false);

    QCOMPARE(r, PanadapterInputController::agcButtonRegion);
}

void PanadapterInputControllerTests::testRegionDetectionFreqScale()
{
    const QRect panRect(0, 0, 1000, 400);
    const QRect waterfallRect(0, 400, 1000, 400);
    const QRect freqScaleRect(0, 370, 1000, 30);
    const QRect dBmScaleRect(0, 0, 40, 370);
    const QRect filterRect(450, 0, 100, 370);
    const QRect secScaleWaterfallRect(0, 400, 40, 400);
    const QRect agcButtonRect(50, 10, 40, 20);
    const QRect panSMeterRect(800, 10, 150, 40);
    const QRect panFreqRect(800, 60, 150, 30);

    auto r = PanadapterInputController::determineRegion(
        QPoint(500, 385), panRect, waterfallRect, freqScaleRect, dBmScaleRect,
        filterRect, secScaleWaterfallRect, agcButtonRect, panSMeterRect, panFreqRect,
        4, 200, 250, 300, false);

    QCOMPARE(r, PanadapterInputController::freqScalePanadapterRegion);
}

void PanadapterInputControllerTests::testRegionDetectionDbmScale()
{
    const QRect panRect(0, 0, 1000, 400);
    const QRect waterfallRect(0, 400, 1000, 400);
    const QRect freqScaleRect(0, 370, 1000, 30);
    const QRect dBmScaleRect(0, 0, 40, 370);
    const QRect filterRect(450, 0, 100, 370);
    const QRect secScaleWaterfallRect(0, 400, 40, 400);
    const QRect agcButtonRect(50, 10, 40, 20);
    const QRect panSMeterRect(800, 10, 150, 40);
    const QRect panFreqRect(800, 60, 150, 30);

    auto r = PanadapterInputController::determineRegion(
        QPoint(20, 150), panRect, waterfallRect, freqScaleRect, dBmScaleRect,
        filterRect, secScaleWaterfallRect, agcButtonRect, panSMeterRect, panFreqRect,
        4, 200, 250, 300, false);

    QCOMPARE(r, PanadapterInputController::dBmScalePanadapterRegion);
}

void PanadapterInputControllerTests::testRegionDetectionFilterLowEdge()
{
    const QRect panRect(0, 0, 1000, 400);
    const QRect waterfallRect(0, 400, 1000, 400);
    const QRect freqScaleRect(0, 370, 1000, 30);
    const QRect dBmScaleRect(0, 0, 40, 370);
    const QRect filterRect(450, 0, 100, 370); // left edge at 450
    const QRect secScaleWaterfallRect(0, 400, 40, 400);
    const QRect agcButtonRect(50, 10, 40, 20);
    const QRect panSMeterRect(800, 10, 150, 40);
    const QRect panFreqRect(800, 60, 150, 30);

    // Hit at x = 451, which is within snapMouse (4) of 450
    auto r = PanadapterInputController::determineRegion(
        QPoint(451, 150), panRect, waterfallRect, freqScaleRect, dBmScaleRect,
        filterRect, secScaleWaterfallRect, agcButtonRect, panSMeterRect, panFreqRect,
        4, 200, 250, 300, false);

    QCOMPARE(r, PanadapterInputController::filterRegionLow);
}

void PanadapterInputControllerTests::testRegionDetectionFilterHighEdge()
{
    const QRect panRect(0, 0, 1000, 400);
    const QRect waterfallRect(0, 400, 1000, 400);
    const QRect freqScaleRect(0, 370, 1000, 30);
    const QRect dBmScaleRect(0, 0, 40, 370);
    const QRect filterRect(450, 0, 100, 370); // right edge at 550 (450 + 100)
    const QRect secScaleWaterfallRect(0, 400, 40, 400);
    const QRect agcButtonRect(50, 10, 40, 20);
    const QRect panSMeterRect(800, 10, 150, 40);
    const QRect panFreqRect(800, 60, 150, 30);

    // Hit at x = 549, which is within snapMouse (4) of 550
    auto r = PanadapterInputController::determineRegion(
        QPoint(549, 150), panRect, waterfallRect, freqScaleRect, dBmScaleRect,
        filterRect, secScaleWaterfallRect, agcButtonRect, panSMeterRect, panFreqRect,
        4, 200, 250, 300, false);

    QCOMPARE(r, PanadapterInputController::filterRegionHigh);
}

void PanadapterInputControllerTests::testRegionDetectionFilterWholePassband()
{
    const QRect panRect(0, 0, 1000, 400);
    const QRect waterfallRect(0, 400, 1000, 400);
    const QRect freqScaleRect(0, 370, 1000, 30);
    const QRect dBmScaleRect(0, 0, 40, 370);
    const QRect filterRect(450, 0, 100, 370); // center around 500
    const QRect secScaleWaterfallRect(0, 400, 40, 400);
    const QRect agcButtonRect(50, 10, 40, 20);
    const QRect panSMeterRect(800, 10, 150, 40);
    const QRect panFreqRect(800, 60, 150, 30);

    // Inside filter at x = 500 with Shift pressed
    auto r = PanadapterInputController::determineRegion(
        QPoint(500, 150), panRect, waterfallRect, freqScaleRect, dBmScaleRect,
        filterRect, secScaleWaterfallRect, agcButtonRect, panSMeterRect, panFreqRect,
        4, 200, 250, 300, true /* Shift */);

    QCOMPARE(r, PanadapterInputController::filterRegion);
}

void PanadapterInputControllerTests::testRegionDetectionAgcLines()
{
    const QRect panRect(0, 0, 1000, 400);
    const QRect waterfallRect(0, 400, 1000, 400);
    const QRect freqScaleRect(0, 370, 1000, 30);
    const QRect dBmScaleRect(0, 0, 40, 370);
    const QRect filterRect(450, 0, 100, 370);
    const QRect secScaleWaterfallRect(0, 400, 40, 400);
    const QRect agcButtonRect(50, 10, 40, 20);
    const QRect panSMeterRect(800, 10, 150, 40);
    const QRect panFreqRect(800, 60, 150, 30);

    const int snapMouse = 4;
    const qreal agcThresh = 100.0;
    const qreal agcHang = 180.0;
    const qreal agcFixed = 260.0;

    // Test hit on AGC threshold line (y = 101, distance 1 < 4)
    auto rThresh = PanadapterInputController::determineRegion(
        QPoint(300, 101), panRect, waterfallRect, freqScaleRect, dBmScaleRect,
        filterRect, secScaleWaterfallRect, agcButtonRect, panSMeterRect, panFreqRect,
        snapMouse, agcThresh, agcHang, agcFixed, false);
    QCOMPARE(rThresh, PanadapterInputController::agcThresholdLine);

    // Test hit on AGC hang line (y = 182, distance 2 < 4)
    auto rHang = PanadapterInputController::determineRegion(
        QPoint(300, 182), panRect, waterfallRect, freqScaleRect, dBmScaleRect,
        filterRect, secScaleWaterfallRect, agcButtonRect, panSMeterRect, panFreqRect,
        snapMouse, agcThresh, agcHang, agcFixed, false);
    QCOMPARE(rHang, PanadapterInputController::agcHangLine);

    // Test hit on AGC fixed gain line (y = 259, distance 1 < 4)
    auto rFixed = PanadapterInputController::determineRegion(
        QPoint(300, 259), panRect, waterfallRect, freqScaleRect, dBmScaleRect,
        filterRect, secScaleWaterfallRect, agcButtonRect, panSMeterRect, panFreqRect,
        snapMouse, agcThresh, agcHang, agcFixed, false);
    QCOMPARE(rFixed, PanadapterInputController::agcFixedGainLine);
}

void PanadapterInputControllerTests::testRegionDetectionPanAndWaterfall()
{
    const QRect panRect(0, 0, 1000, 400);
    const QRect waterfallRect(0, 400, 1000, 400);
    const QRect freqScaleRect(0, 370, 1000, 30);
    const QRect dBmScaleRect(0, 0, 40, 370);
    const QRect filterRect(450, 0, 100, 370);
    const QRect secScaleWaterfallRect(0, 400, 40, 400);
    const QRect agcButtonRect(50, 10, 40, 20);
    const QRect panSMeterRect(800, 10, 150, 40);
    const QRect panFreqRect(800, 60, 150, 30);

    // Panadapter body (e.g. x = 200, y = 200)
    auto rPan = PanadapterInputController::determineRegion(
        QPoint(200, 200), panRect, waterfallRect, freqScaleRect, dBmScaleRect,
        filterRect, secScaleWaterfallRect, agcButtonRect, panSMeterRect, panFreqRect,
        4, 50, 60, 70, false);
    QCOMPARE(rPan, PanadapterInputController::panadapterRegion);

    // Waterfall body (e.g. x = 200, y = 550)
    auto rWf = PanadapterInputController::determineRegion(
        QPoint(200, 550), panRect, waterfallRect, freqScaleRect, dBmScaleRect,
        filterRect, secScaleWaterfallRect, agcButtonRect, panSMeterRect, panFreqRect,
        4, 50, 60, 70, false);
    QCOMPARE(rWf, PanadapterInputController::waterfallRegion);
}

void PanadapterInputControllerTests::testRegionDetectionElsewhere()
{
    const QRect panRect(0, 0, 1000, 400);
    const QRect waterfallRect(0, 400, 1000, 400);
    const QRect freqScaleRect(0, 370, 1000, 30);
    const QRect dBmScaleRect(0, 0, 40, 370);
    const QRect filterRect(450, 0, 100, 370);
    const QRect secScaleWaterfallRect(0, 400, 40, 400);
    const QRect agcButtonRect(50, 10, 40, 20);
    const QRect panSMeterRect(800, 10, 150, 40);
    const QRect panFreqRect(800, 60, 150, 30);

    // Inside panSMeterRect -> returns elsewhere so as not to start pan drag
    auto rMeter = PanadapterInputController::determineRegion(
        QPoint(850, 25), panRect, waterfallRect, freqScaleRect, dBmScaleRect,
        filterRect, secScaleWaterfallRect, agcButtonRect, panSMeterRect, panFreqRect,
        4, 50, 60, 70, false);
    QCOMPARE(rMeter, PanadapterInputController::elsewhere);

    // Outside all bounds
    auto rOut = PanadapterInputController::determineRegion(
        QPoint(1500, 1500), panRect, waterfallRect, freqScaleRect, dBmScaleRect,
        filterRect, secScaleWaterfallRect, agcButtonRect, panSMeterRect, panFreqRect,
        4, 50, 60, 70, false);
    QCOMPARE(rOut, PanadapterInputController::elsewhere);
}

void PanadapterInputControllerTests::testCalculateWheelFrequencyStepUp()
{
    const qint64 currentFreq = 14200000;
    const int angleDeltaY = 120; // wheel up
    const double freqStep = 100.0;
    const qint64 minFreq = 100000;
    const qint64 maxFreq = 60000000;

    qint64 newFreq = PanadapterInputController::calculateWheelFrequency(
        currentFreq, angleDeltaY, freqStep, minFreq, maxFreq);

    QCOMPARE(newFreq, 14200100);
}

void PanadapterInputControllerTests::testCalculateWheelFrequencyStepDown()
{
    const qint64 currentFreq = 14200050; // not aligned to 100 Hz
    const int angleDeltaY = -120; // wheel down
    const double freqStep = 100.0;
    const qint64 minFreq = 100000;
    const qint64 maxFreq = 60000000;

    // 14200050 - 100 = 14199950 -> rounded to nearest 100 = 14200000
    qint64 newFreq = PanadapterInputController::calculateWheelFrequency(
        currentFreq, angleDeltaY, freqStep, minFreq, maxFreq);

    QCOMPARE(newFreq, 14200000);
}

void PanadapterInputControllerTests::testCalculateWheelFrequencyClamping()
{
    const qint64 currentFreq = 59999950;
    const int angleDeltaY = 120;
    const double freqStep = 100.0;
    const qint64 minFreq = 100000;
    const qint64 maxFreq = 60000000;

    qint64 newFreq = PanadapterInputController::calculateWheelFrequency(
        currentFreq, angleDeltaY, freqStep, minFreq, maxFreq);

    QCOMPARE(newFreq, 60000000);
}

void PanadapterInputControllerTests::testCalculatePanDragCenterFreq()
{
    const qint64 currentCenter = 14200000;
    const int deltaX = 10; // moved 10 pixels
    const qreal spanHz = 192000.0;
    const int rectWidth = 1000; // 192 Hz per pixel
    const qint64 minFreq = 0;
    const qint64 maxFreq = 60000000;

    // 10 px * 192 Hz/px = +1920 Hz
    qint64 newCenter = PanadapterInputController::calculatePanDragCenterFreq(
        currentCenter, deltaX, spanHz, rectWidth, minFreq, maxFreq);

    QCOMPARE(newCenter, 14201920);
}

void PanadapterInputControllerTests::testCalculateFilterDragEdges()
{
    const int deltaX = 5;
    const qreal spanHz = 100000.0;
    const int rectWidth = 1000; // 100 Hz per pixel
    const qreal initialLo = 300.0;
    const qreal initialHi = 2700.0;

    qreal outLo = 0.0;
    qreal outHi = 0.0;

    // 1. Drag low edge only: outLo changes by - (5 * 100) = -500 Hz
    PanadapterInputController::calculateFilterDragEdges(
        deltaX, spanHz, rectWidth, initialLo, initialHi, true, false, outLo, outHi);
    QCOMPARE(outLo, -200.0);
    QCOMPARE(outHi, 2700.0);

    // 2. Drag high edge only
    PanadapterInputController::calculateFilterDragEdges(
        deltaX, spanHz, rectWidth, initialLo, initialHi, false, true, outLo, outHi);
    QCOMPARE(outLo, 300.0);
    QCOMPARE(outHi, 2200.0);

    // 3. Drag both (Shift key passband shift)
    PanadapterInputController::calculateFilterDragEdges(
        deltaX, spanHz, rectWidth, initialLo, initialHi, true, true, outLo, outHi);
    QCOMPARE(outLo, -200.0);
    QCOMPARE(outHi, 2200.0);
}

void PanadapterInputControllerTests::testCalculateDbmScaleDrag()
{
    const int deltaY = 10;
    const int rectHeight = 400;
    const qreal initialMin = -140.0;
    const qreal initialMax = -10.0;

    // range = 130 dB. unit = (130 / 400) * 1.5 = 0.4875 dB/pixel.
    // deltaY = 10 -> unit * 10 = 4.875 dB
    qreal outMin = 0.0;
    qreal outMax = 0.0;

    PanadapterInputController::calculateDbmScaleDrag(
        deltaY, rectHeight, initialMin, initialMax, outMin, outMax);

    QCOMPARE(qRound(outMin * 1000.0), qRound((-140.0 - 4.875) * 1000.0));
    QCOMPARE(qRound(outMax * 1000.0), qRound((-10.0 - 4.875) * 1000.0));
}

QTEST_MAIN(PanadapterInputControllerTests)
#include "panadapter_input_controller_tests.moc"
