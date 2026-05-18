#include "SoapySDRDataSource.h"

#ifdef HAVE_SOAPYSDR

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#include <QDebug>
#include <algorithm>

SoapySDRDataSource::SoapySDRDataSource(THPSDRParameter *ioData)
    : QObject(nullptr)
    , set(Settings::instance())
    , io(ioData)
    , m_device(nullptr)
    , m_rxStream(nullptr)
    , m_stopped(false)
    , m_sampleRate(set->getSampleRate())
    , m_numChannels(1)
{
}

SoapySDRDataSource::~SoapySDRDataSource() {
    stop();
}

void SoapySDRDataSource::init() {
    try {
        SoapySDR::Kwargs args;
        TSoapyDevice selected = set->getCurrentSoapyDevice();
        
        if (!selected.driver.isEmpty()) {
            args["driver"] = selected.driver.toStdString();
            if (!selected.serial.isEmpty()) {
                args["serial"] = selected.serial.toStdString();
            }
            qDebug() << "SoapySDRDataSource: Opening device" << selected.label;
            m_device = SoapySDR::Device::make(args);
        } else {
            auto results = SoapySDR::Device::enumerate();
            if (results.empty()) {
                QString msg = "SoapySDRDataSource: No devices found!";
                qCritical() << msg;
                emit messageEvent(msg);
                return;
            }
            m_device = SoapySDR::Device::make(results[0]);
        }

        if (!m_device) {
            QString msg = "SoapySDRDataSource: Failed to create device!";
            qCritical() << msg;
            emit messageEvent(msg);
            return;
        }

        qDebug() << "SoapySDRDataSource: Using device" << QString::fromStdString(m_device->getHardwareKey());

        m_device->setSampleRate(SOAPY_SDR_RX, 0, m_sampleRate);
        
        m_rxStream = m_device->setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32);
        m_device->activateStream(m_rxStream);

        qDebug() << "SoapySDRDataSource: Stream activated at" << m_sampleRate << "Hz";

        connect(set, &Settings::sampleRateChanged, this, &SoapySDRDataSource::setSampleRate);
        connect(set, &Settings::vfoFrequencyChanged, this, &SoapySDRDataSource::setFrequency);

    } catch (const std::exception &ex) {
        qCritical() << "SoapySDRDataSource init error:" << ex.what();
    }
}

void SoapySDRDataSource::stop() {
    m_stopped = true;
    if (m_device && m_rxStream) {
        m_device->deactivateStream(m_rxStream);
        m_device->closeStream(m_rxStream);
        m_rxStream = nullptr;
    }
    if (m_device) {
        SoapySDR::Device::unmake(m_device);
        m_device = nullptr;
    }
}

void SoapySDRDataSource::runStream() {
    if (!m_device || !m_rxStream) return;

    const size_t numSamples = 512; // Typical HPSDR packet size-ish
    std::vector<float> buff(numSamples * 2); // complex samples
    void *buffs[] = {buff.data()};

    m_stopped = false;

    while (!m_stopped) {
        int flags;
        long long timeNs;
        int ret = m_device->readStream(m_rxStream, buffs, numSamples, flags, timeNs, 100000);

        if (ret > 0) {
            QList<double> samples;
            samples.reserve(ret * 2);
            for (int i = 0; i < ret; ++i) {
                samples.append((double)buff[i*2]);
                samples.append((double)buff[i*2+1]);
            }
            io->data_queue.enqueue(samples);
            emit readydata();
        } else if (ret < 0) {
            qCritical() << "SoapySDRDataSource: readStream error" << ret;
        }
    }
}

void SoapySDRDataSource::setSampleRate(int value) {
    if (m_device) {
        m_sampleRate = value;
        m_device->setSampleRate(SOAPY_SDR_RX, 0, m_sampleRate);
    }
}

void SoapySDRDataSource::setFrequency(int rx, long frequency) {
    Q_UNUSED(rx);
    if (m_device) {
        m_device->setFrequency(SOAPY_SDR_RX, 0, (double)frequency);
    }
}

#endif // HAVE_SOAPYSDR
