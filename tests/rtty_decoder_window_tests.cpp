/**
 * @file  rtty_decoder_window_tests.cpp
 * @brief Unit tests for RttyDecoderWindow desktop window and UI controls.
 * @author Simon Eatough <simon.eatough@gmail.com>
 * @date 2026-09-22
 */

#include <QtTest/QtTest>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QCloseEvent>

#include "UI/RttyDecoderWindow.h"
#include "Models/RadioModel.h"
#include "Models/SliceModel.h"
#include "cusdr_settings.h"

class RttyDecoderWindowTests : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testWindowTitleAndRxNumber();
    void testScopeVisibilitySync();
    void testDecodedTextDisplay();
    void testClearButton();
    void testDockButton();
    void testAutoDetectButton();
    void testStatusBadgeFormatting();
    void testCloseEventDisablesRtty();
};

void RttyDecoderWindowTests::initTestCase() {
    qputenv("QT_QPA_PLATFORM", "offscreen");
}

void RttyDecoderWindowTests::cleanupTestCase() {
    Settings::delete_instance();
}

void RttyDecoderWindowTests::testWindowTitleAndRxNumber() {
    RadioModel radio;
    SliceModel slice0(0, &radio);
    RttyDecoderWindow win0(&slice0, Settings::instance(), 0);
    QCOMPARE(win0.windowTitle(), QStringLiteral("cudaSDR - RTTY Decoder (RX 1)"));

    SliceModel slice1(1, &radio);
    RttyDecoderWindow win1(&slice1, Settings::instance(), 1);
    QCOMPARE(win1.windowTitle(), QStringLiteral("cudaSDR - RTTY Decoder (RX 2)"));
}

void RttyDecoderWindowTests::testScopeVisibilitySync() {
    RadioModel radio;
    SliceModel slice(0, &radio);
    RttyDecoderWindow win(&slice, Settings::instance(), 0);

    RttyScopeWidget *scopeWidget = win.findChild<RttyScopeWidget*>();
    QVERIFY(scopeWidget != nullptr);
    QWidget *scopeContainer = scopeWidget->parentWidget();
    QVERIFY(scopeContainer != nullptr);

    // Initial state: slice has scope visible by default
    slice.setRttyScopeVisible(true);
    QVERIFY(!scopeContainer->isHidden());

    // Toggle hidden
    slice.setRttyScopeVisible(false);
    QVERIFY(scopeContainer->isHidden());

    // Toggle visible again
    slice.setRttyScopeVisible(true);
    QVERIFY(!scopeContainer->isHidden());
}

void RttyDecoderWindowTests::testDecodedTextDisplay() {
    RadioModel radio;
    SliceModel slice(0, &radio);
    RttyDecoderWindow win(&slice, Settings::instance(), 0);

    QPlainTextEdit *terminal = win.findChild<QPlainTextEdit*>();
    QVERIFY(terminal != nullptr);

    slice.setRttyDecodedText(QStringLiteral("RYRYRY CQ CQ DE ZL2BRG K"));
    QCOMPARE(terminal->toPlainText(), QStringLiteral("RYRYRY CQ CQ DE ZL2BRG K"));
}

void RttyDecoderWindowTests::testClearButton() {
    RadioModel radio;
    SliceModel slice(0, &radio);
    RttyDecoderWindow win(&slice, Settings::instance(), 0);

    QPlainTextEdit *terminal = win.findChild<QPlainTextEdit*>();
    QVERIFY(terminal != nullptr);

    slice.setRttyDecodedText(QStringLiteral("TEST BUFFER CONTENT"));
    QCOMPARE(terminal->toPlainText(), QStringLiteral("TEST BUFFER CONTENT"));

    const auto buttons = win.findChildren<QPushButton*>();
    QPushButton *clearBtn = nullptr;
    for (QPushButton *btn : buttons) {
        if (btn->text() == QStringLiteral("Clear")) {
            clearBtn = btn;
            break;
        }
    }
    QVERIFY(clearBtn != nullptr);
    clearBtn->click();

    QVERIFY(slice.rttyDecodedText().isEmpty());
    QVERIFY(terminal->toPlainText().isEmpty());
}

void RttyDecoderWindowTests::testDockButton() {
    RadioModel radio;
    SliceModel slice(0, &radio);
    RttyDecoderWindow win(&slice, Settings::instance(), 0);

    slice.setRttyFloating(true);
    QCOMPARE(slice.rttyFloating(), true);

    const auto buttons = win.findChildren<QPushButton*>();
    QPushButton *dockBtn = nullptr;
    for (QPushButton *btn : buttons) {
        if (btn->text().contains(QStringLiteral("Dock"))) {
            dockBtn = btn;
            break;
        }
    }
    QVERIFY(dockBtn != nullptr);
    dockBtn->click();

    QCOMPARE(slice.rttyFloating(), false);
}

void RttyDecoderWindowTests::testAutoDetectButton() {
    RadioModel radio;
    SliceModel slice(0, &radio);
    RttyDecoderWindow win(&slice, Settings::instance(), 0);

    slice.setRttyAutoDetect(false);
    QCOMPARE(slice.rttyAutoDetect(), false);

    const auto buttons = win.findChildren<QPushButton*>();
    QPushButton *autoBtn = nullptr;
    for (QPushButton *btn : buttons) {
        if (btn->text() == QStringLiteral("Auto")) {
            autoBtn = btn;
            break;
        }
    }
    QVERIFY(autoBtn != nullptr);
    autoBtn->click();

    QCOMPARE(slice.rttyAutoDetect(), true);
}

void RttyDecoderWindowTests::testStatusBadgeFormatting() {
    RadioModel radio;
    SliceModel slice(0, &radio);
    RttyDecoderWindow win(&slice, Settings::instance(), 0);

    slice.setRttyToneLocked(true);
    slice.setRttyBaudRate(45.45f);
    slice.setRttyShiftHz(170.0f);
    slice.setRttySnrDb(18.5f);
    slice.setRttyReverse(false);

    // Find the badge button (it's the first button without standard text "Auto"/"Scope"/"Clear"/etc.)
    const auto buttons = win.findChildren<QPushButton*>();
    QPushButton *badgeBtn = nullptr;
    for (QPushButton *btn : buttons) {
        if (btn->toolTip().contains(QStringLiteral("RTTY settings"))) {
            badgeBtn = btn;
            break;
        }
    }
    QVERIFY(badgeBtn != nullptr);

    const QString badgeText = badgeBtn->text();
    QVERIFY(badgeText.contains(QStringLiteral("45/170")));
    QVERIFY(badgeText.contains(QStringLiteral("19dB")));
    QVERIFY(badgeText.contains(QStringLiteral("[NOR]")));
}

void RttyDecoderWindowTests::testCloseEventDisablesRtty() {
    RadioModel radio;
    SliceModel slice(0, &radio);
    RttyDecoderWindow win(&slice, Settings::instance(), 0);

    slice.setRttyDecodeEnabled(true);
    QCOMPARE(slice.rttyDecodeEnabled(), true);

    win.close();
    QCOMPARE(slice.rttyDecodeEnabled(), false);
}

QTEST_MAIN(RttyDecoderWindowTests)
#include "rtty_decoder_window_tests.moc"
