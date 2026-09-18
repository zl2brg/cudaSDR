#ifndef TEXTSTREAMLOGGER_H
#define TEXTSTREAMLOGGER_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QDateTime>
#include <QDir>
#include <QStandardPaths>

/**
 * Thread-safe log file writer for decoded Morse (CW) and RTTY text streams.
 * Automatically organizes daily logs into ~/.cudaSDR/logs/ (e.g. rtty_rx0_20260918.log).
 */
class TextStreamLogger : public QObject {
    Q_OBJECT

public:
    explicit TextStreamLogger(const QString &modePrefix, int rxId = 0, QObject *parent = nullptr);
    ~TextStreamLogger() override;

    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool enable);

    int rxId() const { return m_rxId; }
    QString modePrefix() const { return m_modePrefix; }
    QString currentLogFilePath() const { return m_currentFilePath; }

    void appendText(const QString &text);
    void logCallsign(const QString &callsign, float snrDb, qint64 freqHz = 0);

private:
    void ensureFileOpen();
    void closeFile();

    QString m_modePrefix;
    int m_rxId = 0;
    bool m_enabled = false;
    QString m_currentDateString;
    QString m_currentFilePath;

    QFile m_file;
    QTextStream m_stream;
    mutable QMutex m_mutex;
};

#endif // TEXTSTREAMLOGGER_H
