#include "AudioDeviceService.h"
#include <QDebug>

AudioDeviceService* AudioDeviceService::instance() {
    static AudioDeviceService* s_instance = new AudioDeviceService();
    return s_instance;
}

AudioDeviceService::AudioDeviceService(QObject* parent) : QObject(parent) {
    connect(&m_mediaDevices, &QMediaDevices::audioInputsChanged, this, &AudioDeviceService::audioInputsChanged);
    connect(&m_mediaDevices, &QMediaDevices::audioOutputsChanged, this, &AudioDeviceService::audioOutputsChanged);
}

QList<QAudioDevice> AudioDeviceService::audioInputs() const {
    return QMediaDevices::audioInputs();
}

QList<QAudioDevice> AudioDeviceService::audioOutputs() const {
    return QMediaDevices::audioOutputs();
}

QStringList AudioDeviceService::audioInputDescriptions(bool includeHpsdrLocal) const {
    QStringList descriptions;
    if (includeHpsdrLocal) {
        descriptions << "hpsdr-local";
    }
    for (const auto& device : audioInputs()) {
        descriptions << device.description();
    }
    return descriptions;
}

QStringList AudioDeviceService::audioOutputDescriptions() const {
    QStringList descriptions;
    for (const auto& device : audioOutputs()) {
        descriptions << device.description();
    }
    return descriptions;
}

QAudioDevice AudioDeviceService::findInputByDescription(const QString& description) const {
    for (const auto& device : audioInputs()) {
        if (device.description() == description) {
            return device;
        }
    }
    return QMediaDevices::defaultAudioInput();
}

QAudioDevice AudioDeviceService::findOutputByDescription(const QString& description) const {
    for (const auto& device : audioOutputs()) {
        if (device.description() == description) {
            return device;
        }
    }
    return QMediaDevices::defaultAudioOutput();
}

QAudioDevice AudioDeviceService::defaultInput() const {
    return QMediaDevices::defaultAudioInput();
}

QAudioDevice AudioDeviceService::defaultOutput() const {
    return QMediaDevices::defaultAudioOutput();
}
