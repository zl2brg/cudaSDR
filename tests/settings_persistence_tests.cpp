#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QSettings>
#include <QFile>
#include <QSet>
#include <QMap>
#include <functional>

#include "cusdr_settings.h"
#include "Models/RadioModel.h"
#include "Models/SliceModel.h"

namespace {

QMap<QString, QStringList> parseIniKeyOccurrences(const QString &path)
{
    QFile file(path);
    QMap<QString, QStringList> occurrences;
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return occurrences;

    QString section;
    while (!file.atEnd()) {
        QString line = QString::fromUtf8(file.readLine()).trimmed();
        if (line.isEmpty() || line.startsWith(QLatin1Char(';')) || line.startsWith(QLatin1Char('#')))
            continue;
        if (line.startsWith(QLatin1Char('[')) && line.endsWith(QLatin1Char(']'))) {
            section = line.mid(1, line.size() - 2);
            if (section.compare(QLatin1String("General"), Qt::CaseInsensitive) == 0)
                section.clear();
            continue;
        }
        const int eq = line.indexOf(QLatin1Char('='));
        if (eq <= 0)
            continue;
        const QString key = line.left(eq).trimmed();
        const QString value = line.mid(eq + 1).trimmed();
        const QString fullKey = section.isEmpty() ? key : (section + QLatin1Char('/') + key);
        occurrences[fullKey].append(value);
    }
    return occurrences;
}

QMap<QString, QVariant> dumpConfigIni(const std::function<void(QSettings *)> &save)
{
    QTemporaryDir dir;
    QSettings out(dir.filePath(QStringLiteral("module.ini")), QSettings::IniFormat);
    save(&out);
    out.sync();
    QMap<QString, QVariant> values;
    const auto keys = out.allKeys();
    for (const QString &key : keys)
        values.insert(key, out.value(key));
    return values;
}

void assertNoConflictingValues(const QMap<QString, QVariant> &a, const QString &aName,
                               const QMap<QString, QVariant> &b, const QString &bName)
{
    for (auto it = a.constBegin(); it != a.constEnd(); ++it) {
        if (!b.contains(it.key()))
            continue;
        if (b.value(it.key()) != it.value()) {
            QFAIL(qPrintable(QStringLiteral("%1 and %2 both write '%3' with different values (%4 vs %5)")
                                 .arg(aName, bName, it.key(),
                                      it.value().toString(), b.value(it.key()).toString())));
        }
    }
}

} // namespace

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
    void saveDoesNotWriteConflictingDuplicateIniKeys();
    void loadPersistentSettingsMigratesIniToJson();
    void loadPersistentSettingsPrefersJsonOverIni();
    void loadPersistentSettingsFallsBackFromCorruptJson();
    void saveSettingsWritesJsonSibling();

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
    {
        QSettings wipe(m_iniPath, QSettings::IniFormat);
        wipe.clear();
        wipe.sync();
    }

    Settings::delete_instance();
    m_settings = Settings::instance();
    m_settings->reopenSettingsStorage(m_iniPath);
    QFile::remove(m_settings->defaultJsonConfigPath());
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
    QCOMPARE(m_settings->getMinimumWidgetWidth(), m_settings->windowConfig()->minimumWidgetWidth());
    QCOMPARE(m_settings->getMinimumGroupBoxWidth(), 250);
    QCOMPARE(m_settings->getMinimumGroupBoxWidth(), m_settings->windowConfig()->minimumGroupBoxWidth());
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

        seed.setValue(QStringLiteral("receiver0/dspCore"), QStringLiteral("qtdsp"));
        seed.setValue(QStringLiteral("receiver0/centerFrequency"), 14100000.0);
        seed.setValue(QStringLiteral("receiver0/vfoFrequency"), 14150000.0);
        seed.setValue(QStringLiteral("receiver0/filterSlope"), 2);
        seed.setValue(QStringLiteral("rx0/dspCore"), QStringLiteral("qtdsp"));
        seed.setValue(QStringLiteral("rx0/centerFrequency"), 14100000.0);
        seed.setValue(QStringLiteral("rx0/vfoFrequency"), 14150000.0);

        seed.setValue(QStringLiteral("alex/manual"), QStringLiteral("on"));
        seed.setValue(QStringLiteral("alex/lpf160m"), QStringLiteral("on"));
        seed.setValue(QStringLiteral("alex/lpf6m"), QStringLiteral("on"));
        seed.setValue(QStringLiteral("alex/state160m"), 34);
        seed.setValue(QStringLiteral("freedv/rx0_mode"), 3);
        seed.setValue(QStringLiteral("network/tci_enabled"), false);
        seed.setValue(QStringLiteral("network/tci_rx_gain"), 1.5);
        seed.setValue(QStringLiteral("SoapySDR/rxAntenna"), QStringLiteral("LNAW"));
        seed.setValue(QStringLiteral("SoapySDR/lnaGain"), 18);

        seed.sync();
    }

    QVERIFY(m_settings->loadSettings() >= 0);

    // Verify Display Config
    QCOMPARE(m_settings->displayConfig()->dBmDistScaleMin(), -50.0);
    QCOMPARE(m_settings->displayConfig()->dBmDistScaleMax(), 120.0);
    QCOMPARE(m_settings->displayConfig()->sMeterHoldTime(), 3500);
    QCOMPARE(m_settings->displayConfig()->panadapterColors().panBackgroundColor, QColor(1, 2, 3));

    // Verify Audio (RX) and Transmit configs
    QCOMPARE(m_settings->audioConfig()->mainVolume(), 0.85f);
    QCOMPARE(m_settings->transmitConfig()->micSource(), 0); // janus
    QCOMPARE(m_settings->transmitConfig()->micGain(), 15.5);
    QCOMPARE(m_settings->transmitConfig()->fmDeviation(), 6000.0);

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
    QCOMPARE(m_settings->getVfoFrequency(0), static_cast<qint64>(14150000));
    QCOMPARE(m_settings->getCtrFrequency(0), static_cast<qint64>(14100000));
    QCOMPARE(m_settings->getVfoFrequency(0), m_settings->getReceiverDataList().at(0).vfoFrequency);
    QCOMPARE(m_settings->receiverConfigs().at(0)->dspCore(), QSDR::QtDSP);
    QCOMPARE(m_settings->receiverConfigs().at(0)->ctrFrequency(), 14100000);
    QCOMPARE(m_settings->receiverConfigs().at(0)->vfoFrequency(), 14150000);
    QCOMPARE(m_settings->getReceiverDataList().at(0).filterSlope, 2);

    QCOMPARE(m_settings->getAlexConfig(), static_cast<quint16>(0x4101));
    QCOMPARE(m_settings->getAlexConfig(), m_settings->alexConfig()->alexConfig());
    QCOMPARE(m_settings->getAlexStates().value(m160), 34);
    QCOMPARE(m_settings->getAlexStates(), m_settings->alexConfig()->alexStates());
    QCOMPARE(m_settings->getFreeDVMode(0), 3);
    QCOMPARE(m_settings->getFreeDVMode(0), m_settings->freeDVConfig()->rxMode(0));
    QCOMPARE(m_settings->getServerPort(), m_settings->networkConfig()->serverPort());
    QCOMPARE(m_settings->getTciServerEnabled(), false);
    QCOMPARE(m_settings->getTciServerEnabled(), m_settings->tciConfig()->serverEnabled());
    QCOMPARE(m_settings->getTciRxGain(), 1.5f);
    QCOMPARE(m_settings->getTciRxGain(), m_settings->tciConfig()->rxGain());
    QCOMPARE(m_settings->getSoapyRxAntenna(), QStringLiteral("LNAW"));
    QCOMPARE(m_settings->getSoapyRxAntenna(), m_settings->soapyConfig()->rxAntenna());
    QCOMPARE(m_settings->getSoapyLnaGain(), 18);
    QCOMPARE(m_settings->getSoapyLnaGain(), m_settings->soapyConfig()->lnaGain());

    // Update values, save, and reload
    m_settings->displayConfig()->setdBmDistScaleMin(-60.0);
    m_settings->transmitConfig()->setMicSource(1); // penelope
    m_settings->cwConfig()->setInternalCw(0);
    m_settings->hardwareConfig()->setSource10Mhz(2); // mercury
    m_settings->setVfoFrequency(0, 14200000);
    m_settings->setAlexConfiguration(0x410F);
    m_settings->setServerAddr(QStringLiteral("10.0.0.8"));
    m_settings->setFreeDVMode(0, 4);

    QVERIFY(m_settings->saveSettings() >= 0);

    // Reopen Settings to force load
    Settings::delete_instance();
    m_settings = Settings::instance();
    m_settings->reopenSettingsStorage(m_iniPath);

    QVERIFY(m_settings->loadSettings() >= 0);

    QCOMPARE(m_settings->displayConfig()->dBmDistScaleMin(), -60.0);
    QCOMPARE(m_settings->transmitConfig()->micSource(), 1); // penelope
    QCOMPARE(m_settings->cwConfig()->internalCw(), 0);
    QCOMPARE(m_settings->hardwareConfig()->source10Mhz(), 2); // mercury
    QCOMPARE(m_settings->getVfoFrequency(0), static_cast<qint64>(14200000));
    QCOMPARE(m_settings->receiverConfigs().at(0)->vfoFrequency(), 14200000);
    QCOMPARE(m_settings->receiverConfigs().at(0)->vfoFrequency(), m_settings->getVfoFrequency(0));
    QCOMPARE(m_settings->getAlexConfig(), static_cast<quint16>(0x410F));
    QCOMPARE(m_settings->getAlexConfig(), m_settings->alexConfig()->alexConfig());
    QCOMPARE(m_settings->getServerAddr(), QStringLiteral("10.0.0.8"));
    QCOMPARE(m_settings->getServerAddr(), m_settings->networkConfig()->serverAddress());
    QCOMPARE(m_settings->getFreeDVMode(0), 4);
    QCOMPARE(m_settings->getFreeDVMode(0), m_settings->freeDVConfig()->rxMode(0));
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

void SettingsPersistenceTests::saveDoesNotWriteConflictingDuplicateIniKeys()
{
    QVERIFY(m_settings->loadSettings() >= 0);

    m_settings->setAlexConfiguration(0x410F);
    m_settings->setDriveLevel(77);
    m_settings->setServerPort(45000);
    m_settings->setFreeDVMode(1, 4);

    QVERIFY(m_settings->saveSettings() >= 0);

    const auto occurrences = parseIniKeyOccurrences(m_iniPath);
    QVERIFY2(!occurrences.isEmpty(), qPrintable(m_iniPath));
    for (auto it = occurrences.constBegin(); it != occurrences.constEnd(); ++it) {
        QSet<QString> unique;
        for (const QString &value : it.value())
            unique.insert(value);
        if (unique.size() > 1) {
            QFAIL(qPrintable(QStringLiteral("INI key '%1' written %2 times with different values: %3")
                                 .arg(it.key())
                                 .arg(it.value().size())
                                 .arg(it.value().join(QLatin1String(", ")))));
        }
    }

    QSettings saved(m_iniPath, QSettings::IniFormat);
    QCOMPARE(saved.value(QStringLiteral("driveLevel")).toInt(), m_settings->transmitConfig()->driveLevel());
    QCOMPARE(saved.value(QStringLiteral("driveLevel")).toInt(), m_settings->getDriveLevel());
    QCOMPARE(saved.value(QStringLiteral("network/server_port")).toInt(),
             static_cast<int>(m_settings->networkConfig()->serverPort()));
    QCOMPARE(saved.value(QStringLiteral("network/server_port")).toInt(),
             static_cast<int>(m_settings->getServerPort()));
    QCOMPARE(saved.value(QStringLiteral("freedv/rx1_mode")).toInt(), m_settings->freeDVConfig()->rxMode(1));
    QCOMPARE(saved.value(QStringLiteral("freedv/rx1_mode")).toInt(), m_settings->getFreeDVMode(1));
    QCOMPARE(m_settings->getAlexConfig(), m_settings->alexConfig()->alexConfig());
    QCOMPARE(saved.value(QStringLiteral("alex/manual")).toString().toLower(), QStringLiteral("on"));
    QCOMPARE(saved.value(QStringLiteral("alex/lpf160m")).toString().toLower(), QStringLiteral("on"));
    QCOMPARE(saved.value(QStringLiteral("alex/lpf6m")).toString().toLower(), QStringLiteral("on"));
    QCOMPARE(saved.value(QStringLiteral("window/minimumWidgetWidth")).toInt(),
             m_settings->windowConfig()->minimumWidgetWidth());
    QCOMPARE(saved.value(QStringLiteral("window/minimumWidgetWidth")).toInt(),
             m_settings->getMinimumWidgetWidth());
    QCOMPARE(saved.value(QStringLiteral("network/tci_enabled")).toBool(),
             m_settings->tciConfig()->serverEnabled());
    QCOMPARE(saved.value(QStringLiteral("network/tci_enabled")).toBool(),
             m_settings->getTciServerEnabled());
    QCOMPARE(saved.value(QStringLiteral("SoapySDR/lnaGain")).toInt(),
             m_settings->soapyConfig()->lnaGain());
    QCOMPARE(saved.value(QStringLiteral("SoapySDR/lnaGain")).toInt(),
             m_settings->getSoapyLnaGain());
    QCOMPARE(saved.value(QStringLiteral("receiver0/vfoFrequency")).toLongLong(),
             m_settings->getVfoFrequency(0));
    QCOMPARE(saved.value(QStringLiteral("receiver0/filterSlope")).toInt(),
             m_settings->getReceiverDataList().at(0).filterSlope);

    const auto network = dumpConfigIni([&](QSettings *s) { m_settings->networkConfig()->saveIni(s); });
    const auto audio = dumpConfigIni([&](QSettings *s) { m_settings->audioConfig()->saveIni(s); });
    const auto alex = dumpConfigIni([&](QSettings *s) { m_settings->alexConfig()->saveIni(s); });
    const auto transmit = dumpConfigIni([&](QSettings *s) { m_settings->transmitConfig()->saveIni(s); });
    const auto freedv = dumpConfigIni([&](QSettings *s) { m_settings->freeDVConfig()->saveIni(s); });
    const auto hardware = dumpConfigIni([&](QSettings *s) { m_settings->hardwareConfig()->saveIni(s); });
    const auto display = dumpConfigIni([&](QSettings *s) { m_settings->displayConfig()->saveIni(s); });
    const auto cw = dumpConfigIni([&](QSettings *s) { m_settings->cwConfig()->saveIni(s); });
    const auto window = dumpConfigIni([&](QSettings *s) { m_settings->windowConfig()->saveIni(s); });
    const auto tci = dumpConfigIni([&](QSettings *s) { m_settings->tciConfig()->saveIni(s); });
    const auto soapy = dumpConfigIni([&](QSettings *s) { m_settings->soapyConfig()->saveIni(s); });
    QMap<QString, QVariant> receivers;
    for (ReceiverConfig *rx : m_settings->receiverConfigs()) {
        const auto dumped = dumpConfigIni([&](QSettings *s) { rx->saveIni(s); });
        for (auto it = dumped.constBegin(); it != dumped.constEnd(); ++it)
            receivers.insert(it.key(), it.value());
    }

    const QList<QPair<QString, QMap<QString, QVariant>>> modules = {
        {QStringLiteral("NetworkConfig"), network},
        {QStringLiteral("AudioConfig"), audio},
        {QStringLiteral("AlexConfig"), alex},
        {QStringLiteral("TransmitConfig"), transmit},
        {QStringLiteral("FreeDVConfig"), freedv},
        {QStringLiteral("HardwareConfig"), hardware},
        {QStringLiteral("DisplayConfig"), display},
        {QStringLiteral("CWConfig"), cw},
        {QStringLiteral("WindowConfig"), window},
        {QStringLiteral("TciConfig"), tci},
        {QStringLiteral("SoapyConfig"), soapy},
        {QStringLiteral("ReceiverConfig"), receivers},
    };
    for (int i = 0; i < modules.size(); ++i) {
        for (int j = i + 1; j < modules.size(); ++j)
            assertNoConflictingValues(modules[i].second, modules[i].first,
                                      modules[j].second, modules[j].first);
    }
}

void SettingsPersistenceTests::loadPersistentSettingsMigratesIniToJson()
{
    writeSeedIni(QStringLiteral("MIGRATE1"));
    const QString jsonPath = m_settings->defaultJsonConfigPath();
    QVERIFY(!QFile::exists(jsonPath));

    QVERIFY(m_settings->loadPersistentSettings() >= 0);
    QCOMPARE(m_settings->getCallsign(), QStringLiteral("MIGRATE1"));
    QVERIFY(QFile::exists(jsonPath));

    Settings::delete_instance();
    m_settings = Settings::instance();
    m_settings->reopenSettingsStorage(m_iniPath);
    QVERIFY(m_settings->loadJson(jsonPath));
    QCOMPARE(m_settings->getCallsign(), QStringLiteral("MIGRATE1"));
}

void SettingsPersistenceTests::loadPersistentSettingsPrefersJsonOverIni()
{
    writeSeedIni(QStringLiteral("FROMINI"));
    QVERIFY(m_settings->loadSettings() >= 0);
    m_settings->setCallsign(QStringLiteral("FROMJSON"));
    QVERIFY(m_settings->saveJson());

    writeSeedIni(QStringLiteral("FROMINI"));
    Settings::delete_instance();
    m_settings = Settings::instance();
    m_settings->reopenSettingsStorage(m_iniPath);

    QVERIFY(m_settings->loadPersistentSettings() >= 0);
    QCOMPARE(m_settings->getCallsign(), QStringLiteral("FROMJSON"));
}

void SettingsPersistenceTests::loadPersistentSettingsFallsBackFromCorruptJson()
{
    writeSeedIni(QStringLiteral("INIFALLBACK"));
    const QString jsonPath = m_settings->defaultJsonConfigPath();
    {
        QFile bad(jsonPath);
        QVERIFY(bad.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text));
        bad.write("{ not valid json");
    }

    QVERIFY(m_settings->loadPersistentSettings() >= 0);
    QCOMPARE(m_settings->getCallsign(), QStringLiteral("INIFALLBACK"));
    QVERIFY(m_settings->loadJson(jsonPath));
    QCOMPARE(m_settings->getCallsign(), QStringLiteral("INIFALLBACK"));
}

void SettingsPersistenceTests::saveSettingsWritesJsonSibling()
{
    QVERIFY(m_settings->loadSettings() >= 0);
    m_settings->setCallsign(QStringLiteral("DUALWRITE"));
    QVERIFY(m_settings->saveSettings() >= 0);

    const QString jsonPath = m_settings->defaultJsonConfigPath();
    QVERIFY(QFile::exists(jsonPath));
    QVERIFY(QFile::exists(m_iniPath));

    Settings::delete_instance();
    m_settings = Settings::instance();
    m_settings->reopenSettingsStorage(m_iniPath);
    QVERIFY(m_settings->loadJson(jsonPath));
    QCOMPARE(m_settings->getCallsign(), QStringLiteral("DUALWRITE"));
}

QTEST_MAIN(SettingsPersistenceTests)
#include "settings_persistence_tests.moc"
