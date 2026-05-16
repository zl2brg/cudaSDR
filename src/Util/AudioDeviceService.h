#ifndef AUDIODEVICESERVICE_H
#define AUDIODEVICESERVICE_H

#include <QObject>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QStringList>
#include <QList>

class AudioDeviceService : public QObject {
    Q_OBJECT

public:
    static AudioDeviceService* instance();

    QList<QAudioDevice> audioInputs() const;
    QList<QAudioDevice> audioOutputs() const;

    QStringList audioInputDescriptions(bool includeHpsdrLocal = false) const;
    QStringList audioOutputDescriptions() const;

    QAudioDevice findInputByDescription(const QString& description) const;
    QAudioDevice findOutputByDescription(const QString& description) const;

    QAudioDevice defaultInput() const;
    QAudioDevice defaultOutput() const;

signals:
    void audioInputsChanged();
    void audioOutputsChanged();

private:
    explicit AudioDeviceService(QObject* parent = nullptr);
    QMediaDevices m_mediaDevices;
};

#endif // AUDIODEVICESERVICE_H
