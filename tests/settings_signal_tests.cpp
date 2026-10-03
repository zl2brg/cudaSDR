#include <QtTest/QtTest>
#include <QSignalSpy>

#include "Models/RadioModel.h"
#include "Models/SliceModel.h"
#include "cusdr_hamDatabase.h"
#include "cusdr_settings.h"

class SettingsSignalTests : public QObject {
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void callsignChangedOnUpdate();
    void callsignUnchangedSkipsSignal();
    void radioStateChangedOnTransition();
    void radioStateUnchangedSkipsSignal();
    void driveLevelChangedAlwaysEmitted();
    void vfoFrequencyChangedOnUpdate();
    void vfoMode0SetsNcoKeepsCenter();
    void ctrMode1ClearsStaleNcoWhenVfoUnchanged();
    void visibleRetuneKeepsCenterInsideSpan();
    void visibleRetuneRecentersOutsideSpan();
    void applyHamBandIgnoresOffBandLastVfo();
    void applyHamBandMovesRx2WhenSettingsVfoAlreadyOnBand();
    void applyHamBandMovesRx2SliceWhenLastVfoStale();

private:
    Settings *m_settings = nullptr;
};

void SettingsSignalTests::init()
{
    Settings::delete_instance();
    m_settings = Settings::instance();
}

void SettingsSignalTests::cleanup()
{
    Settings::delete_instance();
    m_settings = nullptr;
}

void SettingsSignalTests::callsignChangedOnUpdate()
{
    QSignalSpy spy(m_settings, &Settings::callsignChanged);
    m_settings->setCallsign(QStringLiteral("TEST"));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(m_settings->getCallsign(), QStringLiteral("TEST"));
}

void SettingsSignalTests::callsignUnchangedSkipsSignal()
{
    m_settings->setCallsign(QStringLiteral("SAME"));
    QSignalSpy spy(m_settings, &Settings::callsignChanged);
    m_settings->setCallsign(QStringLiteral("SAME"));
    QCOMPARE(spy.count(), 0);
}

void SettingsSignalTests::radioStateChangedOnTransition()
{
    m_settings->setRadioState(RadioState::RX);
    QSignalSpy spy(m_settings, &Settings::radioStateChanged);
    m_settings->setRadioState(RadioState::MOX);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).value<RadioState>(), RadioState::MOX);
}

void SettingsSignalTests::radioStateUnchangedSkipsSignal()
{
    m_settings->setRadioState(RadioState::RX);
    QSignalSpy spy(m_settings, &Settings::radioStateChanged);
    m_settings->setRadioState(RadioState::RX);
    QCOMPARE(spy.count(), 0);
}

void SettingsSignalTests::driveLevelChangedAlwaysEmitted()
{
    QSignalSpy spy(m_settings, &Settings::driveLevelChanged);
    m_settings->setDriveLevel(25);
    m_settings->setDriveLevel(25);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.at(1).at(0).toInt(), 25);
}

void SettingsSignalTests::vfoFrequencyChangedOnUpdate()
{
    const qint64 frequency = 14'100'000;
    QSignalSpy spy(m_settings, &Settings::vfoFrequencyChanged);
    m_settings->setVFOFrequency(0, 0, frequency);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toInt(), 0);
    QCOMPARE(spy.at(0).at(1).toInt(), 0);
    QCOMPARE(spy.at(0).at(2).value<qint64>(), frequency);
}

void SettingsSignalTests::vfoMode0SetsNcoKeepsCenter()
{
    // Click-to-tune contract: mode 0 moves VFO/NCO only; center/LO stays put.
    const qint64 center = 14'100'000;
    const qint64 vfo = 14'105'000;
    m_settings->setCtrFrequency(0, 0, center);
    m_settings->setVFOFrequency(0, 0, center);

    QSignalSpy ncoSpy(m_settings, &Settings::ncoFrequencyChanged);
    QSignalSpy ctrSpy(m_settings, &Settings::ctrFrequencyChanged);
    m_settings->setVFOFrequency(0, 0, vfo);

    QCOMPARE(m_settings->getCtrFrequency(0), center);
    QCOMPARE(m_settings->getVfoFrequency(0), vfo);
    QCOMPARE(m_settings->getReceiverDataList().at(0).ncoFrequency, vfo - center);
    QCOMPARE(ncoSpy.count(), 1);
    QCOMPARE(ncoSpy.at(0).at(0).toInt(), 0);
    QCOMPARE(ncoSpy.at(0).at(1).value<qint64>(), vfo - center);
    QCOMPARE(ctrSpy.count(), 0);
}

void SettingsSignalTests::ctrMode1ClearsStaleNcoWhenVfoUnchanged()
{
    // After a mode-0 band hop, VFO is already at the dial freq but CTR/NCO are
    // wrong. Mode 1 must retune CTR and clear NCO even when VFO is unchanged.
    const qint64 stuckCenter = 14'074'000;
    const qint64 dial = 7'074'000;
    m_settings->setCtrFrequency(0, 0, stuckCenter);
    m_settings->setVFOFrequency(0, 0, dial);
    QCOMPARE(m_settings->getReceiverDataList().at(0).ncoFrequency, dial - stuckCenter);

    m_settings->setCtrFrequency(1, 0, dial);
    QCOMPARE(m_settings->getCtrFrequency(0), dial);
    QCOMPARE(m_settings->getVfoFrequency(0), dial);
    QCOMPARE(m_settings->getReceiverDataList().at(0).ncoFrequency, 0);
}

void SettingsSignalTests::visibleRetuneKeepsCenterInsideSpan()
{
    // VFO A/B switch within the displayed span: only the NCO moves, so the
    // panadapter does not jump under the operator.
    const qint64 center = 14'100'000;
    const qint64 target = center + m_settings->getSampleRate() / 4;
    m_settings->setCtrFrequency(0, 0, center);
    m_settings->setVFOFrequency(0, 0, center);

    m_settings->setVfoFrequencyVisible(0, target);

    QCOMPARE(m_settings->getCtrFrequency(0), center);
    QCOMPARE(m_settings->getVfoFrequency(0), target);
    QCOMPARE(m_settings->getReceiverDataList().at(0).ncoFrequency, target - center);
}

void SettingsSignalTests::visibleRetuneRecentersOutsideSpan()
{
    // A VFO-B memory on another band is off-panel: the LO has to follow, else
    // the RX filter and cursor are clamped off the edge of the panadapter.
    const qint64 center = 14'100'000;
    const qint64 target = 7'074'000;
    m_settings->setCtrFrequency(0, 0, center);
    m_settings->setVFOFrequency(0, 0, center);

    m_settings->setVfoFrequencyVisible(0, target);

    QCOMPARE(m_settings->getCtrFrequency(0), target);
    QCOMPARE(m_settings->getVfoFrequency(0), target);
    QCOMPARE(m_settings->getReceiverDataList().at(0).ncoFrequency, 0);
}

void SettingsSignalTests::applyHamBandIgnoresOffBandLastVfo()
{
    // ReceiverConfig pads unused RX last-VFO slots with 7.050 MHz. A 20 m
    // button must not restore that 40 m frequency (and then no-op).
    const QList<THamBandFrequencies> bands = m_settings->getBandFrequencyList();
    QCOMPARE(getBandFromFrequency(bands, m_settings->frequencyForHamBand(1, static_cast<HamBand>(m20))),
             static_cast<HamBand>(m20));

    m_settings->setCtrFrequency(0, 1, 7'050'000);
    m_settings->setVFOFrequency(0, 1, 7'050'000);
    m_settings->applyHamBand(1, static_cast<HamBand>(m20));

    QCOMPARE(getBandFromFrequency(bands, m_settings->getVfoFrequency(1)), static_cast<HamBand>(m20));
    QCOMPARE(getBandFromFrequency(bands, m_settings->getCtrFrequency(1)), static_cast<HamBand>(m20));
}

void SettingsSignalTests::applyHamBandMovesRx2WhenSettingsVfoAlreadyOnBand()
{
    // Settings VFO already 20 m, CTR still on 40 m — the old setVFOFrequency(2)
    // early-return left the panadapter on 40 m.
    const qint64 stuckCenter = 7'050'000;
    const qint64 dial = 14'100'000;
    m_settings->setCtrFrequency(0, 1, stuckCenter);
    m_settings->setVFOFrequency(0, 1, dial);
    QCOMPARE(m_settings->getCtrFrequency(1), stuckCenter);

    m_settings->applyHamBand(1, static_cast<HamBand>(m20));

    QCOMPARE(m_settings->getCtrFrequency(1), dial);
    QCOMPARE(m_settings->getVfoFrequency(1), dial);
    QCOMPARE(m_settings->getReceiverDataList().at(1).ncoFrequency, 0);
}

void SettingsSignalTests::applyHamBandMovesRx2SliceWhenLastVfoStale()
{
    RadioModel radio;
    radio.addSlice(new SliceModel(0, &radio));
    radio.addSlice(new SliceModel(1, &radio));
    m_settings->setRadioModel(&radio);

    SliceModel *rx2 = radio.slices().at(1);
    rx2->setCenterFrequency(7'000'000);
    rx2->setFrequency(7'000'000);
    m_settings->setCtrFrequency(0, 1, 7'000'000);
    m_settings->setVFOFrequency(0, 1, 7'000'000);

    m_settings->applyHamBand(1, static_cast<HamBand>(m20));

    const QList<THamBandFrequencies> bands = m_settings->getBandFrequencyList();
    QCOMPARE(getBandFromFrequency(bands, rx2->frequency()), static_cast<HamBand>(m20));
    QCOMPARE(getBandFromFrequency(bands, rx2->centerFrequency()), static_cast<HamBand>(m20));

    m_settings->setRadioModel(nullptr);
}

QTEST_MAIN(SettingsSignalTests)
#include "settings_signal_tests.moc"
