#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QComboBox>
#include <QSpinBox>

#include "cusdr_settings.h"
#include "cusdr_transmitOptionsWidget.h"
#include "UI/tx_settings_dialog.h"
#include "Controllers/TransmitSettingsController.h"
#include "Models/TransmitModel.h"
#include "Util/AudioDeviceService.h"

class TransmitOptionsTests : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testOptionsWidgetDevicePopulation();
    void testOptionsWidgetSignalsAndSetters();
    void testOptionsWidgetFilterControls();
    void testControllerBindingWithOptions();
    void testTwoWaySyncBetweenOptionsAndSettingsDialog();
    void testLiveMicInputChangeAppliesToSettings();
};

void TransmitOptionsTests::initTestCase()
{
    // Settings instance initialized
}

void TransmitOptionsTests::cleanupTestCase()
{
    Settings::delete_instance();
}

void TransmitOptionsTests::testOptionsWidgetDevicePopulation()
{
    TransmitOptionsWidget widget;
    // By default, the combo should at least have "HPSDR Mic Input" at index 0
    const auto combos = widget.findChildren<QComboBox*>();
    QVERIFY(!combos.isEmpty());
    QComboBox* combo = combos.first();
    QVERIFY(combo != nullptr);
    QVERIFY(combo->count() >= 1);
    QCOMPARE(combo->itemText(0), QStringLiteral("HPSDR Mic Input"));

    if (combo->count() == 1) {
        QCOMPARE(widget.micInputDev(), 0);
        QCOMPARE(widget.micInputSourceName(), QStringLiteral("hpsdr-local"));
    } else {
        QVERIFY(widget.micInputDev() >= 0);
        QCOMPARE(widget.micInputSourceName(), combo->currentText());
    }
}

void TransmitOptionsTests::testOptionsWidgetSignalsAndSetters()
{
    TransmitOptionsWidget widget;
    QSignalSpy spyDev(&widget, &TransmitOptionsWidget::micInputDevChanged);
    QSignalSpy spyName(&widget, &TransmitOptionsWidget::micInputSourceNameChanged);

    // Test programmatically setting mic input dev
    widget.setMicInputDev(0);
    // Setting via setter uses QSignalBlocker, so no signals should be emitted
    QCOMPARE(spyDev.count(), 0);
    QCOMPARE(spyName.count(), 0);

    // Test setMicInputSourceName
    widget.setMicInputSourceName(QStringLiteral("hpsdr-local"));
    QCOMPARE(widget.micInputDev(), 0);
    QCOMPARE(widget.micInputSourceName(), QStringLiteral("hpsdr-local"));
    QCOMPARE(spyDev.count(), 0);
    QCOMPARE(spyName.count(), 0);

    // If there are multiple devices, test selecting another device
    const auto combos = widget.findChildren<QComboBox*>();
    QVERIFY(!combos.isEmpty());
    QComboBox* combo = combos.first();
    if (combo->count() > 1) {
        combo->setCurrentIndex(1);
        QCOMPARE(spyDev.count(), 1);
        QCOMPARE(spyName.count(), 1);
        QCOMPARE(spyDev.first().at(0).toInt(), 1);
        QCOMPARE(spyName.first().at(0).toString(), combo->itemText(1));

        // Switch back to 0
        combo->setCurrentIndex(0);
        QCOMPARE(spyDev.count(), 2);
        QCOMPARE(spyName.count(), 2);
        QCOMPARE(spyDev.at(1).at(0).toInt(), 0);
        QCOMPARE(spyName.at(1).at(0).toString(), QStringLiteral("hpsdr-local"));
    }
}

void TransmitOptionsTests::testOptionsWidgetFilterControls()
{
    TransmitOptionsWidget widget;
    QSignalSpy spyLow(&widget, &TransmitOptionsWidget::txFilterLowRequested);
    QSignalSpy spyHigh(&widget, &TransmitOptionsWidget::txFilterHighRequested);

    // Test setters (they should block signals)
    widget.setTxFilterLow(150);
    widget.setTxFilterHigh(2900);
    QCOMPARE(spyLow.count(), 0);
    QCOMPARE(spyHigh.count(), 0);

    // Find spinboxes
    const auto spinboxes = widget.findChildren<QSpinBox*>();
    QVERIFY(spinboxes.size() >= 2);
    // Locate high and low filter spinboxes by their value
    QSpinBox* lowSpin = nullptr;
    QSpinBox* highSpin = nullptr;
    for (auto* sb : spinboxes) {
        if (sb->value() == 150) lowSpin = sb;
        if (sb->value() == 2900) highSpin = sb;
    }
    QVERIFY(lowSpin != nullptr);
    QVERIFY(highSpin != nullptr);

    // Changing spinbox should emit signals
    lowSpin->setValue(250);
    QCOMPARE(spyLow.count(), 1);
    QCOMPARE(spyLow.first().at(0).toInt(), 250);

    highSpin->setValue(3200);
    QCOMPARE(spyHigh.count(), 1);
    QCOMPARE(spyHigh.first().at(0).toInt(), 3200);
}

void TransmitOptionsTests::testControllerBindingWithOptions()
{
    TransmitModel model;
    TransmitOptionsWidget widget;
    TransmitSettingsController controller;

    controller.bindOptions(&widget, &model);

    // 1. Model -> OptionsWidget sync
    model.setTxFilterLow(180);
    model.setTxFilterHigh(3000);
    model.setMicInputDev(0);
    model.setMicInputSourceName(QStringLiteral("hpsdr-local"));

    QCOMPARE(widget.micInputDev(), 0);
    QCOMPARE(widget.micInputSourceName(), QStringLiteral("hpsdr-local"));

    // 2. OptionsWidget -> Model sync
    emit widget.txFilterLowRequested(220);
    QCOMPARE(model.txFilterLow(), 220);

    emit widget.txFilterHighRequested(3300);
    QCOMPARE(model.txFilterHigh(), 3300);

    emit widget.micInputDevChanged(0);
    emit widget.micInputSourceNameChanged(QStringLiteral("hpsdr-local"));
    QCOMPARE(model.micInputDev(), 0);
    QCOMPARE(model.micInputSourceName(), QStringLiteral("hpsdr-local"));
}

void TransmitOptionsTests::testTwoWaySyncBetweenOptionsAndSettingsDialog()
{
    TransmitModel model;
    TransmitOptionsWidget optionsWidget;
    tx_settings_dialog settingsDialog;
    TransmitSettingsController controller;

    controller.bind(&settingsDialog, &model, Settings::instance());
    controller.bindOptions(&optionsWidget, &model);

    // Check initial synchronization
    model.setMicInputSourceName(QStringLiteral("hpsdr-local"));
    QCOMPARE(optionsWidget.micInputSourceName(), QStringLiteral("hpsdr-local"));

    // Change filters from options widget -> verify dialog and model update
    emit optionsWidget.txFilterLowRequested(175);
    QCOMPARE(model.txFilterLow(), 175);

    emit optionsWidget.txFilterHighRequested(2950);
    QCOMPARE(model.txFilterHigh(), 2950);

    // Change filters from dialog -> verify options widget and model update
    emit settingsDialog.txFilterLowRequested(190);
    QCOMPARE(model.txFilterLow(), 190);

    emit settingsDialog.txFilterHighRequested(3150);
    QCOMPARE(model.txFilterHigh(), 3150);

    // If extra devices exist, verify changing combo in options updates dialog
    const auto optionsCombos = optionsWidget.findChildren<QComboBox*>();
    QVERIFY(!optionsCombos.isEmpty());
    QComboBox* optionsCombo = optionsCombos.first();
    if (optionsCombo->count() > 1) {
        optionsCombo->setCurrentIndex(1);
        QCOMPARE(model.micInputDev(), 1);
        QCOMPARE(model.micInputSourceName(), optionsCombo->itemText(1));

        // Switch back via dialog
        emit settingsDialog.micInputDevChanged(0);
        emit settingsDialog.micInputSourceNameChanged(QStringLiteral("hpsdr-local"));
        QCOMPARE(model.micInputDev(), 0);
        QCOMPARE(optionsWidget.micInputDev(), 0);
        QCOMPARE(optionsWidget.micInputSourceName(), QStringLiteral("hpsdr-local"));
    }
}

void TransmitOptionsTests::testLiveMicInputChangeAppliesToSettings()
{
    TransmitModel model;
    TransmitOptionsWidget optionsWidget;
    tx_settings_dialog settingsDialog;
    TransmitSettingsController controller;
    Settings* settings = Settings::instance();

    const int previousDev = settings->getMicInputDev();
    const QString previousName = settings->getMicInputSourceName();

    controller.bind(&settingsDialog, &model, settings);
    controller.bindOptions(&optionsWidget, &model);

    QSignalSpy spy(settings, &Settings::micInputChanged);

    emit optionsWidget.micInputDevChanged(1);
    emit optionsWidget.micInputSourceNameChanged(QStringLiteral("default"));

    QCOMPARE(model.micInputDev(), 1);
    QCOMPARE(model.micInputSourceName(), QStringLiteral("default"));
    QCOMPARE(settings->getMicInputDev(), 1);
    QCOMPARE(settings->getMicInputSourceName(), QStringLiteral("default"));
    QVERIFY(spy.count() >= 1);
    QCOMPARE(spy.last().at(0).toInt(), 1);

    emit settingsDialog.micInputDevChanged(0);
    emit settingsDialog.micInputSourceNameChanged(QStringLiteral("hpsdr-local"));

    QCOMPARE(model.micInputDev(), 0);
    QCOMPARE(settings->getMicInputDev(), 0);
    QCOMPARE(settings->getMicInputSourceName(), QStringLiteral("hpsdr-local"));
    QCOMPARE(spy.last().at(0).toInt(), 0);

    settings->setMicInputDev(previousDev);
    settings->setMicInputSourceName(previousName);
}

QTEST_MAIN(TransmitOptionsTests)
#include "transmit_options_tests.moc"
