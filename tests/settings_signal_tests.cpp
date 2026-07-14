#include <QtTest/QtTest>
#include <QSignalSpy>

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

QTEST_MAIN(SettingsSignalTests)
#include "settings_signal_tests.moc"
