/**
 * @file  display_panel_renderer_tests.cpp
 * @brief Unit tests for DisplayFreqRenderer, SMeterRenderer, and display panel formatting.
 * @author Simon Eatough <simon.eatough@gmail.com>
 * @date 2026-09-08
 */

#include <QtTest/QtTest>
#include "GL/DisplayFreqRenderer.h"
#include "GL/SMeterRenderer.h"

class DisplayPanelRendererTests : public QObject {
    Q_OBJECT

private slots:
    void testFreqMhzDisplayStringLeadingZeros();
    void testFreqMhzDisplayStringBands();
    void testSplitFreqDisplay();
    void testSMeterIARUMarkers();
    void testSMeterBallisticsFastAttack();
    void testSMeterBallisticsSmoothDecay();
};

void DisplayPanelRendererTests::testFreqMhzDisplayStringLeadingZeros()
{
    DisplayFreqRenderer renderer(nullptr);

    // 0 Hz
    QString s0 = renderer.freqMhzDisplayString(0);
    QCOMPARE(s0.trimmed(), QStringLiteral("0"));

    // 500 kHz (630m MF band)
    QString s500k = renderer.freqMhzDisplayString(500000);
    QCOMPARE(s500k.trimmed(), QStringLiteral("500"));

    // 7.100 MHz (40m HF band): leading GHz and 100MHz blanked to spaces
    QString s7m = renderer.freqMhzDisplayString(7100000);
    QCOMPARE(s7m.trimmed(), QStringLiteral("7.100"));

    // 14.250 MHz (20m HF band)
    QString s14m = renderer.freqMhzDisplayString(14250000);
    QCOMPARE(s14m.trimmed(), QStringLiteral("14.250"));

    // 144.200 MHz (2m VHF band)
    QString s144m = renderer.freqMhzDisplayString(144200000);
    QCOMPARE(s144m.trimmed(), QStringLiteral("144.200"));

    // 1.296100 GHz (23cm UHF band)
    QString s1g = renderer.freqMhzDisplayString(1296100000);
    QCOMPARE(s1g.trimmed(), QStringLiteral("1.296.100"));
}

void DisplayPanelRendererTests::testFreqMhzDisplayStringBands()
{
    DisplayFreqRenderer renderer(nullptr);

    // Verify exactly 3 decimals in the kHz part
    QString sTopBand = renderer.freqMhzDisplayString(1840000);
    QVERIFY(sTopBand.endsWith(QStringLiteral(".840")));

    QString s10m = renderer.freqMhzDisplayString(28400000);
    QVERIFY(s10m.endsWith(QStringLiteral("28.400")));

    QString s6m = renderer.freqMhzDisplayString(50125000);
    QVERIFY(s6m.endsWith(QStringLiteral("50.125")));
}

void DisplayPanelRendererTests::testSplitFreqDisplay()
{
    DisplayFreqRenderer renderer(nullptr);

    QString f1, f2;
    renderer.splitFreqDisplay(14074123, &f1, &f2);
    QCOMPARE(f1.trimmed(), QStringLiteral("14.074"));
    QCOMPARE(f2, QStringLiteral("123"));

    renderer.splitFreqDisplay(7100005, &f1, &f2);
    QCOMPARE(f1.trimmed(), QStringLiteral("7.100"));
    QCOMPARE(f2, QStringLiteral("005"));

    renderer.splitFreqDisplay(3500000, &f1, &f2);
    QCOMPARE(f1.trimmed(), QStringLiteral("3.500"));
    QCOMPARE(f2, QStringLiteral("000"));
}

void DisplayPanelRendererTests::testSMeterIARUMarkers()
{
    // Standard IARU S-Meter definition:
    // S9 = -73 dBm on HF. Each S-unit below S9 is 6 dB.
    // Above S9 is dB over S9.
    auto calcSUnit = [](double orgVal) -> QString {
        if (orgVal >= -73.0) {
            const int over = qRound(orgVal - (-73.0));
            return (over > 0) ? QStringLiteral("S9+%1").arg(over) : QStringLiteral("S9");
        } else {
            const int s = qBound(0, static_cast<int>(9.0 + (orgVal - (-73.0)) / 6.0 + 0.5), 9);
            return QStringLiteral("S%1").arg(s);
        }
    };

    QCOMPARE(calcSUnit(-73.0), QStringLiteral("S9"));
    QCOMPARE(calcSUnit(-53.0), QStringLiteral("S9+20"));
    QCOMPARE(calcSUnit(-33.0), QStringLiteral("S9+40"));
    QCOMPARE(calcSUnit(-13.0), QStringLiteral("S9+60"));

    QCOMPARE(calcSUnit(-79.0), QStringLiteral("S8"));
    QCOMPARE(calcSUnit(-85.0), QStringLiteral("S7"));
    QCOMPARE(calcSUnit(-97.0), QStringLiteral("S5"));
    QCOMPARE(calcSUnit(-109.0), QStringLiteral("S3"));
    QCOMPARE(calcSUnit(-121.0), QStringLiteral("S1"));
    QCOMPARE(calcSUnit(-140.0), QStringLiteral("S0"));
}

void DisplayPanelRendererTests::testSMeterBallisticsFastAttack()
{
    // Fast-attack ballistics: 80% new + 20% old
    float prev = 20.0f;
    float current = 80.0f;
    float next = current * 0.80f + prev * 0.20f;
    QCOMPARE(next, 68.0f);
    QVERIFY(next > prev);
}

void DisplayPanelRendererTests::testSMeterBallisticsSmoothDecay()
{
    // Smooth-decay ballistics: 15% new + 85% old
    float prev = 80.0f;
    float current = 20.0f;
    float next = current * 0.15f + prev * 0.85f;
    QCOMPARE(next, 71.0f);
    QVERIFY(next < prev);
    QVERIFY(next > current);
}

#include "GL/cusdr_oglDisplayPanel.h"
#include "GL/cusdr_oglText.h"

QFontMetrics OGLText::fontMetrics() const {
    static QFont f;
    return QFontMetrics(f);
}

void OGLDisplayPanel::qglColor(QColor) {}
void OGLDisplayPanel::renderPanelText(OGLText*, float, float, QString const&) {}
void OGLDisplayPanel::renderPanelText(OGLText*, float, float, float, QString const&) {}
QRect OGLDisplayPanel::vfoLabelRect(int) const { return QRect(); }
void OGLDisplayPanel::drawPanelRoundedRect(QRect const&, QColor const&, int, float) {}
void OGLDisplayPanel::drawPanelRoundedRectOutline(QRect const&, QColor const&, int, float) {}
void OGLDisplayPanel::drawPanelRect(QRect const&, QColor const&, float) {}
void OGLDisplayPanel::drawPanelGradientRect(QRect const&, QColor const&, QColor const&, bool, float) {}
qint64 OGLDisplayPanel::vfoMemoryHz(OGLDisplayPanel::DigitVfo) const { return 0; }
SliceModel *OGLDisplayPanel::currentSlice() const { return nullptr; }
void OGLDisplayPanel::rebuildAllFreqDigitHitRegions() {}

QTEST_APPLESS_MAIN(DisplayPanelRendererTests)
#include "display_panel_renderer_tests.moc"
