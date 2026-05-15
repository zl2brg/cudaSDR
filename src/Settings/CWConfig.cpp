#include "CWConfig.h"

CWConfig::CWConfig(QObject *parent)
    : QObject(parent)
    , m_internal_cw(0)
    , m_key_reversed(0)
    , m_keyer_spacing(0)
    , m_keyer_speed(12)
    , m_keyer_mode(0)
    , m_sidetone_volume(64)
    , m_sidetone_freq(1000)
    , m_ptt_delay(32)
    , m_hang_time(32)
    , m_keyer_weight(20)
{}

void CWConfig::setInternalCw(int val) {
    if (m_internal_cw != val) {
        m_internal_cw = val;
        emit internalCwChanged(m_internal_cw);
    }
}

void CWConfig::setKeyReversed(int val) {
    if (m_key_reversed != val) {
        m_key_reversed = val;
        emit keyReversedChanged(m_key_reversed);
    }
}

void CWConfig::setKeyerSpacing(int val) {
    if (m_keyer_spacing != val) {
        m_keyer_spacing = val;
        emit keyerSpacingChanged(m_keyer_spacing);
    }
}

void CWConfig::setKeyerSpeed(int val) {
    if (m_keyer_speed != val) {
        m_keyer_speed = val;
        emit keyerSpeedChanged(m_keyer_speed);
    }
}

void CWConfig::setKeyerMode(int val) {
    if (m_keyer_mode != val) {
        m_keyer_mode = val;
        emit keyerModeChanged(m_keyer_mode);
    }
}

void CWConfig::setSidetoneVolume(int val) {
    if (m_sidetone_volume != val) {
        m_sidetone_volume = val;
        emit sidetoneVolumeChanged(m_sidetone_volume);
    }
}

void CWConfig::setSidetoneFreq(int val) {
    if (m_sidetone_freq != val) {
        m_sidetone_freq = val;
        emit sidetoneFreqChanged(m_sidetone_freq);
    }
}

void CWConfig::setPttDelay(int val) {
    if (m_ptt_delay != val) {
        m_ptt_delay = val;
        emit pttDelayChanged(m_ptt_delay);
    }
}

void CWConfig::setHangTime(int val) {
    if (m_hang_time != val) {
        m_hang_time = val;
        emit hangTimeChanged(m_hang_time);
    }
}

void CWConfig::setKeyerWeight(int val) {
    if (m_keyer_weight != val) {
        m_keyer_weight = val;
        emit keyerWeightChanged(m_keyer_weight);
    }
}

void CWConfig::load(const QJsonObject &json) {
    if (json.contains("internalCw")) m_internal_cw = json["internalCw"].toInt();
    if (json.contains("keyReversed")) m_key_reversed = json["keyReversed"].toInt();
    if (json.contains("keyerSpacing")) m_keyer_spacing = json["keyerSpacing"].toInt();
    if (json.contains("keyerSpeed")) m_keyer_speed = json["keyerSpeed"].toInt();
    if (json.contains("keyerMode")) m_keyer_mode = json["keyerMode"].toInt();
    if (json.contains("sidetoneVolume")) m_sidetone_volume = json["sidetoneVolume"].toInt();
    if (json.contains("sidetoneFreq")) m_sidetone_freq = json["sidetoneFreq"].toInt();
    if (json.contains("pttDelay")) m_ptt_delay = json["pttDelay"].toInt();
    if (json.contains("hangTime")) m_hang_time = json["hangTime"].toInt();
    if (json.contains("keyerWeight")) m_keyer_weight = json["keyerWeight"].toInt();
}

void CWConfig::save(QJsonObject &json) const {
    json["internalCw"] = m_internal_cw;
    json["keyReversed"] = m_key_reversed;
    json["keyerSpacing"] = m_keyer_spacing;
    json["keyerSpeed"] = m_keyer_speed;
    json["keyerMode"] = m_keyer_mode;
    json["sidetoneVolume"] = m_sidetone_volume;
    json["sidetoneFreq"] = m_sidetone_freq;
    json["pttDelay"] = m_ptt_delay;
    json["hangTime"] = m_hang_time;
    json["keyerWeight"] = m_keyer_weight;
}
