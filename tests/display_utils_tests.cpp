#include <QtTest/QtTest>

#include "Util/display_utils.h"

using namespace DisplayUtils;

class DisplayUtilsTests : public QObject {
    Q_OBJECT

private slots:
    void appliesFixedDbOffset();
    void emptySpectrumIsNoOp();
};

void DisplayUtilsTests::appliesFixedDbOffset()
{
    QVector<float> spectrum { -80.0f, -40.0f, 0.0f };
    applyTxPanadapterDisplayOffset(spectrum);
    QCOMPARE(spectrum.at(0), -80.0f + kTxPanadapterDisplayDbOffset);
    QCOMPARE(spectrum.at(1), -40.0f + kTxPanadapterDisplayDbOffset);
    QCOMPARE(spectrum.at(2), 0.0f + kTxPanadapterDisplayDbOffset);
}

void DisplayUtilsTests::emptySpectrumIsNoOp()
{
    QVector<float> spectrum;
    applyTxPanadapterDisplayOffset(spectrum);
    QVERIFY(spectrum.isEmpty());
}

QTEST_APPLESS_MAIN(DisplayUtilsTests)
#include "display_utils_tests.moc"
