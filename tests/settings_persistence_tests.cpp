#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QSettings>

#include "cusdr_settings.h"
#include "Models/RadioModel.h"
#include "Models/SliceModel.h"

class SettingsPersistenceTests : public QObject {
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void loadClampsWindowAndNetworkValues();
    void savePersistsCallsignAndDriveLevel();
    void loadEmptyIniFallbacksToDefaults();
    void loadCorruptedAndClampedValues();
    void loadAndSaveAllConfigModules();
    void savePersistsDBmPanScaleWithSliceModel();

private:
    QTemporaryDir m_tempDir;
    QString m_iniPath;
    Settings *m_settings = nullptr;

    void writeSeedIni(const QString &callsign = QStringLiteral("SEED1ABC"));
};

void SettingsPersistenceTests::init()
{
    QVERIFY(m_tempDir.isValid());
    m_iniPath = m_tempDir.filePath(QStringLiteral("settings.ini"));

    Settings::delete_instance();
    m_settings = Settings::instance();
    m_settings->reopenSettingsStorage(m_iniPath);
}

void SettingsPersistenceTests::cleanup()
{
    Settings::delete_instance();
    m_settings = nullptr;
}

void SettingsPersistenceTests::writeSeedIni(const QString &callsign)
{
    QSettings seed(m_iniPath, QSettings::IniFormat);
    seed.setValue(QStringLiteral("user/callSign"), callsign);
    seed.setValue(QStringLiteral("window/minimumWidgetWidth"), 100);
    seed.setValue(QStringLiteral("window/minimumGroupBoxWidth"), 200);
    seed.setValue(QStringLiteral("network/server_port"), 99999);
    seed.setValue(QStringLiteral("network/server_ipAddress"), QStringLiteral("\"192.168.1.10\""));
    seed.setValue(QStringLiteral("network/socketBufferSize"), 48);
    seed.setValue(QStringLiteral("driveLevel"), 42);
    seed.sync();
}

void SettingsPersistenceTests::loadClampsWindowAndNetworkValues()
{
    writeSeedIni();
    QVERIFY(m_settings->loadSettings() >= 0);

    QCOMPARE(m_settings->getCallsign(), QStringLiteral("SEED1ABC"));
    QCOMPARE(m_settings->getMinimumWidgetWidth(), 300);
    QCOMPARE(m_settings->getMinimumGroupBoxWidth(), 250);
    QCOMPARE(m_settings->getServerPort(), static_cast<quint16>(52685));
    QCOMPARE(m_settings->getSocketBufferSize(), 32);
    QCOMPARE(m_settings->networkConfig()->serverAddress(), QStringLiteral("192.168.1.10"));
}

void SettingsPersistenceTests::savePersistsCallsignAndDriveLevel()
{
    writeSeedIni();
    QVERIFY(m_settings->loadSettings() >= 0);

    m_settings->setCallsign(QStringLiteral("ZL2BRG"));
    m_settings->setDriveLevel(77);
    QVERIFY(m_settings->saveSettings() >= 0);

    QSettings saved(m_iniPath, QSettings::IniFormat);
    QCOMPARE(saved.value(QStringLiteral("user/callSign")).toString(), QStringLiteral("ZL2BRG"));
    QCOMPARE(saved.value(QStringLiteral("driveLevel")).toInt(), 77);
}

void SettingsPersistenceTests::loadEmptyIniFallbacksToDefaults()
{
    {
        QSettings empty(m_iniPath, QSettings::IniFormat);
        empty.clear();
        empty.sync();
    }

    QVERIFY(m_settings->loadSettings() >= 0);

    QCOMPARE(m_settings->getCallsign(), QStringLiteral("Your Call sign"));
    QCOMPARE(m_settings->getMinimumWidgetWidth(), 300);
    QCOMPARE(m_settings->getServerPort(), static_cast<quint16>(52685));
    QCOMPARE(m_settings->getSocketBufferSize(), 32);
    QCOMPARE(m_settings->networkConfig()->serverAddress(), QStringLiteral("127.0.0.1"));
}

void SettingsPersistenceTests::loadCorruptedAndClampedValues()
{
    {
        QSettings seed(m_iniPath, QSettings::IniFormat);
        seed.setValue(QStringLiteral("user/callSign"), QStringLiteral("CORRUPTED"));
        seed.setValue(QStringLiteral("window/minimumWidgetWidth"), -500);
        seed.setValue(QStringLiteral("network/server_port"), 80000);
        seed.setValue(QStringLiteral("network/socketBufferSize"), 9999);
        seed.setValue(QStringLiteral("driveLevel"), 250);
        seed.setValue(QStringLiteral("window/multiRxView"), QStringLiteral("not_a_number"));
        seed.sync();
    }

    QVERIFY(m_settings->loadSettings() >= 0);

    QCOMPARE(m_settings->getCallsign(), QStringLiteral("CORRUPTED"));
    QCOMPARE(m_settings->getMinimumWidgetWidth(), 300);
    QCOMPARE(m_settings->getServerPort(), static_cast<quint16>(52685));
    QCOMPARE(m_settings->getSocketBufferSize(), 32);
    QCOMPARE(m_settings->getDriveLevel(), 0);
    QCOMPARE(m_settings->getMultiRxView(), 0);
}

void SettingsPersistenceTests::loadAndSaveAllConfigModules()
{
    // Write an INI file with specific test values for all modules
    {
        QSettings seed(m_iniPath, QSettings::IniFormat);
        seed.setValue(QStringLiteral("graphics/dBmDistScaleMin"), -50);
        seed.setValue(QStringLiteral("graphics/dBmDistScaleMax"), 120);
        seed.setValue(QStringLiteral("graphics/sMeterHoldTime"), 3500);
        seed.setValue(QStringLiteral("colors/panBackground"), QColor(1, 2, 3));

        seed.setValue(QStringLiteral("server/mic_source"), QStringLiteral("janus"));
        seed.setValue(QStringLiteral("micGain"), 15.5);
        seed.setValue(QStringLiteral("fmdeveation"), 6000.0);
        seed.setValue(QStringLiteral("server/mainVolume"), 85);

        seed.setValue(QStringLiteral("cw/internal"), QStringLiteral("on"));
        seed.setValue(QStringLiteral("cw/key_spacing"), QStringLiteral("on"));
        seed.setValue(QStringLiteral("cw/keyer_speed"), 18);
        seed.setValue(QStringLiteral("cw/sidetone_volume"), 80);
        seed.setValue(QStringLiteral("cw/sidetone_freq"), 800);

        seed.setValue(QStringLiteral("server/10mhzsource"), QStringLiteral("atlas"));
        seed.setValue(QStringLiteral("server/122_88mhzsource"), QStringLiteral("penelope"));

        seed.setValue(QStringLiteral("rx0/dspCore"), QStringLiteral("qtdsp"));
        seed.setValue(QStringLiteral("rx0/centerFrequency"), 14100000.0);
        seed.setValue(QStringLiteral("rx0/vfoFrequency"), 14150000.0);

        seed.sync();
    }

    QVERIFY(m_settings->loadSettings() >= 0);

    // Verify Display Config
    QCOMPARE(m_settings->displayConfig()->dBmDistScaleMin(), -50.0);
    QCOMPARE(m_settings->displayConfig()->dBmDistScaleMax(), 120.0);
    QCOMPARE(m_settings->displayConfig()->sMeterHoldTime(), 3500);
    QCOMPARE(m_settings->displayConfig()->panadapterColors().panBackgroundColor, QColor(1, 2, 3));

    // Verify Audio Config
    QCOMPARE(m_settings->audioConfig()->micSource(), 0); // janus
    QCOMPARE(m_settings->audioConfig()->micGain(), 15.5);
    QCOMPARE(m_settings->audioConfig()->fmDeviation(), 6000.0);
    QCOMPARE(m_settings->audioConfig()->mainVolume(), 0.85f);

    // Verify CW Config
    QCOMPARE(m_settings->cwConfig()->internalCw(), 1);
    QCOMPARE(m_settings->cwConfig()->keyerSpacing(), 1);
    QCOMPARE(m_settings->cwConfig()->keyerSpeed(), 18);
    QCOMPARE(m_settings->cwConfig()->sidetoneVolume(), 80);
    QCOMPARE(m_settings->cwConfig()->sidetoneFreq(), 800);

    // Verify Hardware Config
    QCOMPARE(m_settings->hardwareConfig()->source10Mhz(), 0); // atlas
    QCOMPARE(m_settings->hardwareConfig()->source122_88Mhz(), 0); // penelope

    // Verify Receiver Config
    QCOMPARE(m_settings->receiverConfigs().at(0)->dspCore(), QSDR::QtDSP);
    QCOMPARE(m_settings->receiverConfigs().at(0)->ctrFrequency(), 14100000);
    QCOMPARE(m_settings->receiverConfigs().at(0)->vfoFrequency(), 14150000);

    // Update values, save, and reload
    m_settings->displayConfig()->setdBmDistScaleMin(-60.0);
    m_settings->audioConfig()->setMicSource(1); // penelope
    m_settings->cwConfig()->setInternalCw(0);
    m_settings->hardwareConfig()->setSource10Mhz(2); // mercury
    m_settings->setVfoFrequency(0, 14200000);

    QVERIFY(m_settings->saveSettings() >= 0);

    // Reopen Settings to force load
    Settings::delete_instance();
    m_settings = Settings::instance();
    m_settings->reopenSettingsStorage(m_iniPath);

    QVERIFY(m_settings->loadSettings() >= 0);

    QCOMPARE(m_settings->displayConfig()->dBmDistScaleMin(), -60.0);
    QCOMPARE(m_settings->audioConfig()->micSource(), 1); // penelope
    QCOMPARE(m_settings->cwConfig()->internalCw(), 0);
    QCOMPARE(m_settings->hardwareConfig()->source10Mhz(), 2); // mercury
    QCOMPARE(m_settings->receiverConfigs().at(0)->vfoFrequency(), 14200000);
}

void SettingsPersistenceTests::savePersistsDBmPanScaleWithSliceModel()
{
    writeSeedIni();
    QVERIFY(m_settings->loadSettings() >= 0);

    // Defaults from empty/missing INI keys
    QCOMPARE(m_settings->getdBmPanScaleMin(0, (HamBand)m40), -120.0);
    QCOMPARE(m_settings->getdBmPanScaleMax(0, (HamBand)m40), -10.0);

    RadioModel *radio = new RadioModel(m_settings);
    SliceModel *slice = new SliceModel(0, radio);
    radio->addSlice(slice);
    m_settings->setRadioModel(radio);
    m_settings->syncSlicesWithSettings();

    m_settings->setHamBand(0, false, (HamBand)m40);
    m_settings->setdBmPanScaleMin(0, -95.5);
    m_settings->setdBmPanScaleMax(0, 5.0);

    QCOMPARE(slice->dBmPanScaleMin(), -95.5);
    QCOMPARE(slice->dBmPanScaleMax(), 5.0);
    QCOMPARE(m_settings->getdBmPanScaleMin(0, (HamBand)m40), -95.5);
    QCOMPARE(m_settings->getdBmPanScaleMax(0, (HamBand)m40), 5.0);

    QVERIFY(m_settings->saveSettings() >= 0);

    QSettings saved(m_iniPath, QSettings::IniFormat);
    QCOMPARE(saved.value(QStringLiteral("receiver0/dBmPanScaleMin40m")).toDouble(), -95.5);
    QCOMPARE(saved.value(QStringLiteral("receiver0/dBmPanScaleMax40m")).toDouble(), 5.0);

    m_settings->setRadioModel(nullptr);
    delete radio;

    Settings::delete_instance();
    m_settings = Settings::instance();
    m_settings->reopenSettingsStorage(m_iniPath);
    QVERIFY(m_settings->loadSettings() >= 0);

    QCOMPARE(m_settings->getdBmPanScaleMin(0, (HamBand)m40), -95.5);
    QCOMPARE(m_settings->getdBmPanScaleMax(0, (HamBand)m40), 5.0);
}

QTEST_MAIN(SettingsPersistenceTests)
#include "settings_persistence_tests.moc"
