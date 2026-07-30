#include "AudioConfig.h"
#include <QSettings>
#include "Util/settings_utils.h"

AudioConfig::AudioConfig(QObject *parent)
    : QObject(parent)
    , m_micSource(1) // penelope
    , m_micInputDev(0)
    , m_digitalAudioInputDev(0)
    , m_micInputSourceName("")
    , m_digitalInputSourceName("")
    , m_micGain(10.0)
    , m_driveLevel(100)
    , m_fmPreemphasis(1)
    , m_amCarrierLevel(0.5)
    , m_audioCompression(0)
    , m_fmDeviation(5000.0)
    , m_mainVolume(0.1f)
{}

void AudioConfig::setMicSource(int source) {
    if (m_micSource != source) {
        m_micSource = source;
        emit micSourceChanged(m_micSource);
    }
}

void AudioConfig::setMicInputDev(int dev) {
    if (m_micInputDev != dev) {
        m_micInputDev = dev;
        emit micInputDevChanged(m_micInputDev);
    }
}

void AudioConfig::setDigitalAudioInputDev(int dev) {
    if (m_digitalAudioInputDev != dev) {
        m_digitalAudioInputDev = dev;
        emit digitalAudioInputDevChanged(m_digitalAudioInputDev);
    }
}

void AudioConfig::setMicInputSourceName(const QString &name) {
    if (m_micInputSourceName != name) {
        m_micInputSourceName = name;
        emit micInputSourceNameChanged(m_micInputSourceName);
    }
}

void AudioConfig::setDigitalInputSourceName(const QString &name) {
    if (m_digitalInputSourceName != name) {
        m_digitalInputSourceName = name;
        emit digitalInputSourceNameChanged(m_digitalInputSourceName);
    }
}

void AudioConfig::setMicGain(double gain) {
    if (m_micGain != gain) {
        m_micGain = gain;
        emit micGainChanged(m_micGain);
    }
}

void AudioConfig::setDriveLevel(int level) {
    if (m_driveLevel != level) {
        m_driveLevel = level;
        emit driveLevelChanged(m_driveLevel);
    }
}

void AudioConfig::setFmPreemphasis(int val) {
    if (m_fmPreemphasis != val) {
        m_fmPreemphasis = val;
        emit fmPreemphasisChanged(m_fmPreemphasis);
    }
}

void AudioConfig::setAmCarrierLevel(double level) {
    if (m_amCarrierLevel != level) {
        m_amCarrierLevel = level;
        emit amCarrierLevelChanged(m_amCarrierLevel);
    }
}

void AudioConfig::setAudioCompression(int val) {
    if (m_audioCompression != val) {
        m_audioCompression = val;
        emit audioCompressionChanged(m_audioCompression);
    }
}

void AudioConfig::setFmDeviation(double dev) {
    if (m_fmDeviation != dev) {
        m_fmDeviation = dev;
        emit fmDeviationChanged(m_fmDeviation);
    }
}

void AudioConfig::setMainVolume(float vol) {
    if (m_mainVolume != vol) {
        m_mainVolume = vol;
        emit mainVolumeChanged(m_mainVolume);
    }
}

void AudioConfig::load(const QJsonObject &json) {
    if (json.contains("micSource")) m_micSource = json["micSource"].toInt();
    if (json.contains("micInputDev")) m_micInputDev = json["micInputDev"].toInt();
    if (json.contains("digitalAudioInputDev")) m_digitalAudioInputDev = json["digitalAudioInputDev"].toInt();
    if (json.contains("micInputSourceName")) m_micInputSourceName = json["micInputSourceName"].toString();
    if (json.contains("digitalInputSourceName")) m_digitalInputSourceName = json["digitalInputSourceName"].toString();
    if (json.contains("micGain")) m_micGain = json["micGain"].toDouble();
    if (json.contains("driveLevel")) m_driveLevel = json["driveLevel"].toInt();
    if (json.contains("fmPreemphasis")) m_fmPreemphasis = json["fmPreemphasis"].toInt();
    if (json.contains("amCarrierLevel")) m_amCarrierLevel = json["amCarrierLevel"].toDouble();
    if (json.contains("audioCompression")) m_audioCompression = json["audioCompression"].toInt();
    if (json.contains("fmDeviation")) m_fmDeviation = json["fmDeviation"].toDouble();
    if (json.contains("mainVolume")) m_mainVolume = static_cast<float>(json["mainVolume"].toDouble());
}

void AudioConfig::save(QJsonObject &json) const {
    json["micSource"] = m_micSource;
    json["micInputDev"] = m_micInputDev;
    json["digitalAudioInputDev"] = m_digitalAudioInputDev;
    json["micInputSourceName"] = m_micInputSourceName;
    json["digitalInputSourceName"] = m_digitalInputSourceName;
    json["micGain"] = m_micGain;
    json["driveLevel"] = m_driveLevel;
    json["fmPreemphasis"] = m_fmPreemphasis;
    json["amCarrierLevel"] = m_amCarrierLevel;
    json["audioCompression"] = m_audioCompression;
    json["fmDeviation"] = m_fmDeviation;
    json["mainVolume"] = static_cast<double>(m_mainVolume);
}

void AudioConfig::loadIni(QSettings *settings) {
    QString str = settings->value("server/mic_source", "penelope").toString();
    int val = (str == "janus") ? 0 : 1;
    setMicSource(val);

    setMicInputDev(settings->value("mic_InputDevice", 0).toInt());
    setDigitalAudioInputDev(settings->value("digital_audio_InputDevice", 0).toInt());

    setMicInputSourceName(settings->value("mic_input_source",
                                          (micInputDev() > 0) ? QString("default") : QString()).toString());
    setDigitalInputSourceName(settings->value("digital_input_source",
                                              (digitalAudioInputDev() > 0) ? QString("default") : QString("none")).toString());

    if (micInputSourceName().isEmpty()) {
        setMicInputSourceName("default");
        setMicInputDev(1);
    }
    if (digitalInputSourceName().isEmpty()) {
        setDigitalInputSourceName((digitalAudioInputDev() > 0) ? QString("default") : QString("none"));
    }

    setMicGain(settings->value("micGain", 0).toDouble());
    setDriveLevel(SettingsUtils::clampDriveLevel(settings->value("driveLevel", 0).toInt()));

    setFmPreemphasis(settings->value("fm_preemphesize", 1).toInt());
    setAmCarrierLevel(settings->value("am_carrierlevel", 0.5).toDouble());
    setAudioCompression(settings->value("audiocompression", 0).toInt());
    setFmDeviation(settings->value("fmdeveation", 5000.0).toDouble());

    int volVal = settings->value("server/mainVolume", 10).toInt();
    if (volVal < 0) volVal = 0;
    if (volVal > 100) volVal = 100;
    setMainVolume(volVal / 100.0f);
}

void AudioConfig::saveIni(QSettings *settings) const {
    if (m_micSource == 0)
        settings->setValue("server/mic_source", "janus");
    else
        settings->setValue("server/mic_source", "penelope");

    settings->setValue("mic_InputDevice", m_micInputDev);
    settings->setValue("mic_input_source", m_micInputSourceName);
    settings->setValue("digital_audio_InputDevice", m_digitalAudioInputDev);
    settings->setValue("digital_input_source", m_digitalInputSourceName);
    settings->setValue("micGain", m_micGain);
    settings->setValue("driveLevel", m_driveLevel);
    settings->setValue("fm_preemphesize", m_fmPreemphasis);
    settings->setValue("am_carrierlevel", m_amCarrierLevel);
    settings->setValue("audiocompression", m_audioCompression);
    settings->setValue("fmdeveation", m_fmDeviation);
    settings->setValue("server/mainVolume", static_cast<int>(m_mainVolume * 100));
}
