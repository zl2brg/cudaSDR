#include "TextStreamLogger.h"

TextStreamLogger::TextStreamLogger(const QString &modePrefix, int rxId, QObject *parent)
    : QObject(parent)
    , m_modePrefix(modePrefix)
    , m_rxId(rxId)
{
}

TextStreamLogger::~TextStreamLogger()
{
    QMutexLocker locker(&m_mutex);
    closeFile();
}

void TextStreamLogger::setEnabled(bool enable)
{
    QMutexLocker locker(&m_mutex);
    if (m_enabled == enable) {
        return;
    }
    m_enabled = enable;
    if (!m_enabled) {
        closeFile();
    } else {
        ensureFileOpen();
    }
}

void TextStreamLogger::ensureFileOpen()
{
    if (!m_enabled) {
        return;
    }

    QString today = QDateTime::currentDateTimeUtc().toString("yyyyMMdd");
    if (m_file.isOpen() && today == m_currentDateString) {
        return;
    }

    closeFile();

    QString logDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.cudaSDR/logs";
    QDir dir(logDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    m_currentDateString = today;
    m_currentFilePath = QString("%1/%2_rx%3_%4.log")
                            .arg(logDir)
                            .arg(m_modePrefix.toLower())
                            .arg(m_rxId)
                            .arg(m_currentDateString);

    m_file.setFileName(m_currentFilePath);
    bool isNewFile = !QFile::exists(m_currentFilePath) || m_file.size() == 0;

    if (m_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        m_stream.setDevice(&m_file);
        if (isNewFile) {
            m_stream << QString("# cudaSDR %1 Log - RX%2 - Started %3 UTC\n")
                            .arg(m_modePrefix.toUpper())
                            .arg(m_rxId)
                            .arg(QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd HH:mm:ss"));
            m_stream.flush();
        }
    }
}

void TextStreamLogger::closeFile()
{
    if (m_file.isOpen()) {
        m_stream.flush();
        m_file.close();
        m_stream.setDevice(nullptr);
    }
}

void TextStreamLogger::appendText(const QString &text)
{
    if (!m_enabled || text.isEmpty()) {
        return;
    }

    QMutexLocker locker(&m_mutex);
    ensureFileOpen();
    if (!m_file.isOpen()) {
        return;
    }

    m_stream << text;
    m_stream.flush();
}

void TextStreamLogger::logCallsign(const QString &callsign, float snrDb, qint64 freqHz)
{
    if (!m_enabled || callsign.isEmpty()) {
        return;
    }

    QMutexLocker locker(&m_mutex);
    ensureFileOpen();
    if (!m_file.isOpen()) {
        return;
    }

    QString timeStr = QDateTime::currentDateTimeUtc().toString("HH:mm:ss");
    QString entry = QString("\n[%1 UTC] CALL: %2 | SNR: %3 dB")
                        .arg(timeStr)
                        .arg(callsign)
                        .arg(static_cast<double>(snrDb), 0, 'f', 1);
    if (freqHz > 0) {
        entry += QString(" | FREQ: %1 Hz").arg(freqHz);
    }
    entry += "\n";

    m_stream << entry;
    m_stream.flush();
}
