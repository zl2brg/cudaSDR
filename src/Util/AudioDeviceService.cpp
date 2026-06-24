#include "AudioDeviceService.h"
#include <QDebug>

namespace {
bool audioDiagEnabled() {
    static const bool enabled = qEnvironmentVariableIntValue("CUSDR_AUDIO_DIAG") != 0;
    return enabled;
}

void logAudioDevicesSnapshot(const char *reason) {
    if (!audioDiagEnabled())
        return;

    const QList<QAudioDevice> inputs = QMediaDevices::audioInputs();
    const QList<QAudioDevice> outputs = QMediaDevices::audioOutputs();
    const QAudioDevice defIn = QMediaDevices::defaultAudioInput();
    const QAudioDevice defOut = QMediaDevices::defaultAudioOutput();

    qInfo().nospace()
        << "[AudioDiag] reason=" << reason
        << " QT_MEDIA_BACKEND=\"" << qEnvironmentVariable("QT_MEDIA_BACKEND")
        << "\" XDG_RUNTIME_DIR=\"" << qEnvironmentVariable("XDG_RUNTIME_DIR")
        << "\" DBUS_SESSION_BUS_ADDRESS=\"" << qEnvironmentVariable("DBUS_SESSION_BUS_ADDRESS")
        << "\" PULSE_SERVER=\"" << qEnvironmentVariable("PULSE_SERVER")
        << "\" PIPEWIRE_REMOTE=\"" << qEnvironmentVariable("PIPEWIRE_REMOTE") << "\"";

    qInfo().nospace() << "[AudioDiag] inputs=" << inputs.size()
                      << " defaultInput=\"" << defIn.description() << "\"";
    for (int i = 0; i < inputs.size(); ++i) {
        const QAudioDevice &d = inputs.at(i);
        qInfo().nospace() << "[AudioDiag] input[" << i << "] id=\"" << d.id()
                          << "\" desc=\"" << d.description() << "\"";
    }

    qInfo().nospace() << "[AudioDiag] outputs=" << outputs.size()
                      << " defaultOutput=\"" << defOut.description() << "\"";
    for (int i = 0; i < outputs.size(); ++i) {
        const QAudioDevice &d = outputs.at(i);
        qInfo().nospace() << "[AudioDiag] output[" << i << "] id=\"" << d.id()
                          << "\" desc=\"" << d.description() << "\"";
    }
}
} // namespace

AudioDeviceService* AudioDeviceService::instance() {
    static AudioDeviceService* s_instance = new AudioDeviceService();
    return s_instance;
}

AudioDeviceService::AudioDeviceService(QObject* parent) : QObject(parent) {
    connect(&m_mediaDevices, &QMediaDevices::audioInputsChanged, this, [this]() {
        logAudioDevicesSnapshot("audioInputsChanged");
        emit audioInputsChanged();
    });
    connect(&m_mediaDevices, &QMediaDevices::audioOutputsChanged, this, [this]() {
        logAudioDevicesSnapshot("audioOutputsChanged");
        emit audioOutputsChanged();
    });
    logAudioDevicesSnapshot("serviceInit");
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
