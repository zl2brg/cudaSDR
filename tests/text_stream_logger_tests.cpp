#include <QtTest>
#include "Util/TextStreamLogger.h"

class TextStreamLoggerTests : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testInitialState();
    void testDisabledDoesNotCreateFile();
    void testEnableCreatesFileAndHeader();
    void testAppendText();
    void testLogCallsign();
    void testDisableClosesFile();

private:
    QString m_testFilePath;
};

void TextStreamLoggerTests::initTestCase()
{
}

void TextStreamLoggerTests::cleanupTestCase()
{
    if (!m_testFilePath.isEmpty() && QFile::exists(m_testFilePath)) {
        QFile::remove(m_testFilePath);
    }
}

void TextStreamLoggerTests::testInitialState()
{
    TextStreamLogger logger("RTTY", 1);
    QCOMPARE(logger.modePrefix(), QString("RTTY"));
    QCOMPARE(logger.rxId(), 1);
    QVERIFY(!logger.isEnabled());
}

void TextStreamLoggerTests::testDisabledDoesNotCreateFile()
{
    TextStreamLogger logger("test_inactive", 9);
    logger.appendText("SHOULD NOT WRITE");
    logger.logCallsign("ZL2BRG", 12.5f, 7040000);

    QString path = logger.currentLogFilePath();
    QVERIFY(path.isEmpty());
}

void TextStreamLoggerTests::testEnableCreatesFileAndHeader()
{
    TextStreamLogger logger("test_cw", 0);
    logger.setEnabled(true);
    QVERIFY(logger.isEnabled());

    m_testFilePath = logger.currentLogFilePath();
    QVERIFY(!m_testFilePath.isEmpty());
    QVERIFY(QFile::exists(m_testFilePath));

    QFile file(m_testFilePath);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY(content.contains("# cudaSDR TEST_CW Log - RX0"));
}

void TextStreamLoggerTests::testAppendText()
{
    TextStreamLogger logger("test_cw", 0);
    logger.setEnabled(true);

    logger.appendText("CQ CQ DE ZL2BRG K\n");

    QString path = logger.currentLogFilePath();
    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY(content.contains("CQ CQ DE ZL2BRG K\n"));
}

void TextStreamLoggerTests::testLogCallsign()
{
    TextStreamLogger logger("test_cw", 0);
    logger.setEnabled(true);

    logger.logCallsign("W1AW", 18.2f, 14025000);

    QString path = logger.currentLogFilePath();
    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY(content.contains("CALL: W1AW"));
    QVERIFY(content.contains("SNR: 18.2 dB"));
    QVERIFY(content.contains("FREQ: 14025000 Hz"));
}

void TextStreamLoggerTests::testDisableClosesFile()
{
    TextStreamLogger logger("test_cw", 0);
    logger.setEnabled(true);
    logger.setEnabled(false);
    QVERIFY(!logger.isEnabled());

    logger.appendText("AFTER_DISABLE");

    QString path = logger.currentLogFilePath();
    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY(!content.contains("AFTER_DISABLE"));
}

QTEST_MAIN(TextStreamLoggerTests)
#include "text_stream_logger_tests.moc"
