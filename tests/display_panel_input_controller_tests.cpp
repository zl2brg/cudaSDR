/**
 * @file  display_panel_input_controller_tests.cpp
 * @brief Unit tests for DisplayPanelInputController digit calculations, step cycling, and hit-testing.
 * @author Simon Eatough <simon.eatough@gmail.com>
 * @date 2026-09-09
 */

#include <QtTest/QtTest>
#include "GL/DisplayPanelInputController.h"
#include "GL/cusdr_oglDisplayPanel.h"
#include "Models/SliceModel.h"
#include "Models/RadioModel.h"
#include "cusdr_settings.h"

class DisplayPanelInputControllerTests : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testCalculateDigitDelta();
    void testCalculateNewFrequencySteps();
    void testCalculateNewFrequencyClamping();
    void testCycleFreqStep();
    void testHitTestDigit();
    void testHitTestDigitBlanking();
    void testUpdateFreqDigitHitRegions();
    void testVfoSwitchingAndModelIntegration();
};

#include "GL/cusdr_oglText.h"

QFontMetrics OGLText::fontMetrics() const {
    static QFont f;
    return QFontMetrics(f);
}

void OGLDisplayPanel::scheduleRepaint() {}
SliceModel *OGLDisplayPanel::currentSlice() const { return nullptr; }
QString OGLDisplayPanel::freqMhzDisplayString(qint64) const { return QString(); }

void DisplayPanelInputControllerTests::initTestCase()
{
}

void DisplayPanelInputControllerTests::cleanupTestCase()
{
    Settings::delete_instance();
}

void DisplayPanelInputControllerTests::testCalculateDigitDelta()
{
    QCOMPARE(DisplayPanelInputController::calculateDigitDelta(OGLDisplayPanel::Freq1), 1LL);
    QCOMPARE(DisplayPanelInputController::calculateDigitDelta(OGLDisplayPanel::Freq10), 10LL);
    QCOMPARE(DisplayPanelInputController::calculateDigitDelta(OGLDisplayPanel::Freq100), 100LL);
    QCOMPARE(DisplayPanelInputController::calculateDigitDelta(OGLDisplayPanel::Freq1000), 1000LL);
    QCOMPARE(DisplayPanelInputController::calculateDigitDelta(OGLDisplayPanel::Freq10000), 10000LL);
    QCOMPARE(DisplayPanelInputController::calculateDigitDelta(OGLDisplayPanel::Freq100000), 100000LL);
    QCOMPARE(DisplayPanelInputController::calculateDigitDelta(OGLDisplayPanel::Freq1000000), 1000000LL);
    QCOMPARE(DisplayPanelInputController::calculateDigitDelta(OGLDisplayPanel::Freq10000000), 10000000LL);
    QCOMPARE(DisplayPanelInputController::calculateDigitDelta(OGLDisplayPanel::Freq100000000), 100000000LL);
    QCOMPARE(DisplayPanelInputController::calculateDigitDelta(OGLDisplayPanel::Freq1000000000), 1000000000LL);

    // Non-digit elements must return 0
    QCOMPARE(DisplayPanelInputController::calculateDigitDelta(OGLDisplayPanel::dp0), 0LL);
    QCOMPARE(DisplayPanelInputController::calculateDigitDelta(OGLDisplayPanel::dp1), 0LL);
    QCOMPARE(DisplayPanelInputController::calculateDigitDelta(OGLDisplayPanel::dp2), 0LL);
    QCOMPARE(DisplayPanelInputController::calculateDigitDelta(OGLDisplayPanel::None), 0LL);
}

void DisplayPanelInputControllerTests::testCalculateNewFrequencySteps()
{
    const qint64 baseFreq = 14074000;
    const qint64 maxFreq = 61440000;

    // Step up by 2 on 1 kHz digit -> 14.076.000
    qint64 fUp = DisplayPanelInputController::calculateNewFrequency(baseFreq, OGLDisplayPanel::Freq1000, 2, maxFreq);
    QCOMPARE(fUp, 14076000LL);

    // Step down by 3 on 100 kHz digit -> 13.774.000
    qint64 fDown = DisplayPanelInputController::calculateNewFrequency(baseFreq, OGLDisplayPanel::Freq100000, -3, maxFreq);
    QCOMPARE(fDown, 13774000LL);

    // Step up by 1 on 10 MHz digit -> 24.074.000
    qint64 f10M = DisplayPanelInputController::calculateNewFrequency(baseFreq, OGLDisplayPanel::Freq10000000, 1, maxFreq);
    QCOMPARE(f10M, 24074000LL);

    // Invalid digit returns current freq
    qint64 fNone = DisplayPanelInputController::calculateNewFrequency(baseFreq, OGLDisplayPanel::None, 5, maxFreq);
    QCOMPARE(fNone, baseFreq);
}

void DisplayPanelInputControllerTests::testCalculateNewFrequencyClamping()
{
    const qint64 maxFreq = 60000000;

    // Underflow clamped to 0
    qint64 fUnder = DisplayPanelInputController::calculateNewFrequency(500, OGLDisplayPanel::Freq1000, -2, maxFreq);
    QCOMPARE(fUnder, 0LL);

    // Overflow clamped to maxFreq
    qint64 fOver = DisplayPanelInputController::calculateNewFrequency(59000000, OGLDisplayPanel::Freq1000000, 3, maxFreq);
    QCOMPARE(fOver, maxFreq);
}

void DisplayPanelInputControllerTests::testCycleFreqStep()
{
    // 1 Hz cycles between 1 and 5
    QCOMPARE(DisplayPanelInputController::cycleFreqStep(OGLDisplayPanel::Freq1, 1.0), 5.0);
    QCOMPARE(DisplayPanelInputController::cycleFreqStep(OGLDisplayPanel::Freq1, 5.0), 1.0);

    // 10 Hz cycles between 10 and 50
    QCOMPARE(DisplayPanelInputController::cycleFreqStep(OGLDisplayPanel::Freq10, 10.0), 50.0);
    QCOMPARE(DisplayPanelInputController::cycleFreqStep(OGLDisplayPanel::Freq10, 50.0), 10.0);

    // 100 Hz cycles between 100 and 500
    QCOMPARE(DisplayPanelInputController::cycleFreqStep(OGLDisplayPanel::Freq100, 100.0), 500.0);
    QCOMPARE(DisplayPanelInputController::cycleFreqStep(OGLDisplayPanel::Freq100, 500.0), 100.0);

    // 1 kHz cycles 1000 -> 5000 -> 9000 -> 1000
    QCOMPARE(DisplayPanelInputController::cycleFreqStep(OGLDisplayPanel::Freq1000, 1000.0), 5000.0);
    QCOMPARE(DisplayPanelInputController::cycleFreqStep(OGLDisplayPanel::Freq1000, 5000.0), 9000.0);
    QCOMPARE(DisplayPanelInputController::cycleFreqStep(OGLDisplayPanel::Freq1000, 9000.0), 1000.0);

    // 10 kHz cycles 10000 -> 50000 -> 10000
    QCOMPARE(DisplayPanelInputController::cycleFreqStep(OGLDisplayPanel::Freq10000, 10000.0), 50000.0);
    QCOMPARE(DisplayPanelInputController::cycleFreqStep(OGLDisplayPanel::Freq10000, 50000.0), 10000.0);

    // High steps remain fixed
    QCOMPARE(DisplayPanelInputController::cycleFreqStep(OGLDisplayPanel::Freq100000000, 10.0), 100000000.0);
    QCOMPARE(DisplayPanelInputController::cycleFreqStep(OGLDisplayPanel::Freq1000000000, 10.0), 1000000000.0);

    // Unrecognized digit leaves step unchanged
    QCOMPARE(DisplayPanelInputController::cycleFreqStep(OGLDisplayPanel::None, 25.0), 25.0);
}

void DisplayPanelInputControllerTests::testHitTestDigit()
{
    OGLDisplayPanel::FreqDigitHitRegions regs;
    regs.freg1 = QRegion(QRect(100, 10, 10, 20));
    regs.freg10 = QRegion(QRect(90, 10, 10, 20));
    regs.freg100 = QRegion(QRect(80, 10, 10, 20));
    regs.freg1000 = QRegion(QRect(65, 10, 10, 20));

    QString f1str = QStringLiteral("  14.074");

    int digit = OGLDisplayPanel::None;
    bool hit = DisplayPanelInputController::hitTestDigit(regs, f1str, QPoint(105, 15), &digit);
    QVERIFY(hit);
    QCOMPARE(digit, static_cast<int>(OGLDisplayPanel::Freq1));

    hit = DisplayPanelInputController::hitTestDigit(regs, f1str, QPoint(95, 15), &digit);
    QVERIFY(hit);
    QCOMPARE(digit, static_cast<int>(OGLDisplayPanel::Freq10));

    hit = DisplayPanelInputController::hitTestDigit(regs, f1str, QPoint(85, 15), &digit);
    QVERIFY(hit);
    QCOMPARE(digit, static_cast<int>(OGLDisplayPanel::Freq100));

    // Outside all regions
    hit = DisplayPanelInputController::hitTestDigit(regs, f1str, QPoint(0, 0), &digit);
    QVERIFY(!hit);
    QCOMPARE(digit, static_cast<int>(OGLDisplayPanel::None));
}

void DisplayPanelInputControllerTests::testHitTestDigitBlanking()
{
    OGLDisplayPanel::FreqDigitHitRegions regs;
    regs.freg1000000000 = QRegion(QRect(10, 10, 10, 20)); // idx 0
    regs.point2 = QRegion(QRect(20, 10, 5, 20));         // idx 1
    regs.freg100000000 = QRegion(QRect(25, 10, 10, 20));  // idx 2
    regs.freg10000000 = QRegion(QRect(35, 10, 10, 20));   // idx 3
    regs.freg1000000 = QRegion(QRect(45, 10, 10, 20));    // idx 4

    // Leading slots are blanked with spaces ("  14.074")
    QString f1str = QStringLiteral("   7.100");

    // Hit test on GHz slot (index 0) is space -> suppressed to None
    int digit = OGLDisplayPanel::None;
    bool hit = DisplayPanelInputController::hitTestDigit(regs, f1str, QPoint(15, 15), &digit);
    QVERIFY(!hit);
    QCOMPARE(digit, static_cast<int>(OGLDisplayPanel::None));

    // Hit test on 100 MHz slot (index 2) is space -> suppressed to None
    hit = DisplayPanelInputController::hitTestDigit(regs, f1str, QPoint(28, 15), &digit);
    QVERIFY(!hit);
    QCOMPARE(digit, static_cast<int>(OGLDisplayPanel::None));

    // Hit test on 1 MHz slot (index 4) has '7' -> valid hit
    hit = DisplayPanelInputController::hitTestDigit(regs, f1str, QPoint(48, 15), &digit);
    QVERIFY(hit);
    QCOMPARE(digit, static_cast<int>(OGLDisplayPanel::Freq1000000));
}

void DisplayPanelInputControllerTests::testUpdateFreqDigitHitRegions()
{
    OGLDisplayPanel::FreqDigitHitRegions out;
    const QRect labelRect(10, 5, 20, 30);
    const QString f1str = QStringLiteral("  14.074");

    DisplayPanelInputController::updateFreqDigitHitRegions(
        out, 50, 40, f1str, true, labelRect,
        12, 10, 6,
        25, 18, 30, 22);

    // Label region must match labelRect
    QVERIFY(out.label.contains(QPoint(15, 15)));
    QVERIFY(!out.label.contains(QPoint(5, 5)));

    // Blank slots (GHz and point2) should have empty regions
    QVERIFY(out.freg1000000000.isEmpty());
    QVERIFY(out.point2.isEmpty());

    // Non-blank slots must have non-empty regions
    QVERIFY(!out.freg100000000.isEmpty());
    QVERIFY(!out.freg10000000.isEmpty());
    QVERIFY(!out.freg1.isEmpty());
}

void DisplayPanelInputControllerTests::testVfoSwitchingAndModelIntegration()
{
    RadioModel radioModel;
    auto slice = new SliceModel(0, &radioModel);
    radioModel.addSlice(slice);
    QVERIFY(slice != nullptr);

    slice->setVfoAFrequency(7100000);
    slice->setVfoBFrequency(14250000);
    slice->setActiveVfo(SliceModel::VfoA);

    QCOMPARE(slice->frequency(), 7100000LL);
    QCOMPARE(slice->activeVfo(), SliceModel::VfoA);

    // Switch to VFO B
    slice->setActiveVfo(SliceModel::VfoB);
    QCOMPARE(slice->frequency(), 14250000LL);
    QCOMPARE(slice->activeVfo(), SliceModel::VfoB);

    // Update VFO B frequency
    slice->setVfoBFrequency(14074000);
    QCOMPARE(slice->frequency(), 14074000LL);
    QCOMPARE(slice->vfoBFrequency(), 14074000LL);
    QCOMPARE(slice->vfoAFrequency(), 7100000LL);
}

QTEST_MAIN(DisplayPanelInputControllerTests)
#include "display_panel_input_controller_tests.moc"
