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

        // Use the sample rate from settings (e.g. 48000) to keep DSP in sync
        m_sampleRate = set->getSampleRate();
        m_device->setSampleRate(SOAPY_SDR_RX, 0, m_sampleRate);

        // Try to find a better antenna for LimeSDR (LNAH is usually better for HF/VHF)
        try {
            std::vector<std::string> antennas = m_device->listAntennas(SOAPY_SDR_RX, 0);
            QString antennaList;
            for (const auto& a : antennas) antennaList += QString::fromStdString(a) + " ";
            qDebug() << "SoapySDRDataSource: Available antennas:" << antennaList;
            
            QString targetAntenna = "LNAH"; 
            bool found = false;
            for (const auto& a : antennas) {
                if (QString::fromStdString(a) == targetAntenna) {
                    found = true;
                    break;
                }
            }
            
            if (!found && !antennas.empty()) targetAntenna = QString::fromStdString(antennas[0]);
            
            m_device->setAntenna(SOAPY_SDR_RX, 0, targetAntenna.toStdString());
            qDebug() << "SoapySDRDataSource: Selected antenna:" << targetAntenna;

            // Set detailed gains if available, otherwise generic
            if (m_device->getHardwareKey() == "LimeSDR-Mini") {
                m_device->setGain(SOAPY_SDR_RX, 0, "LNA", 15.0);
                m_device->setGain(SOAPY_SDR_RX, 0, "TIA", 12.0);
                m_device->setGain(SOAPY_SDR_RX, 0, "PGA", 15.0);
            } else {
                m_device->setGain(SOAPY_SDR_RX, 0, 40.0);
            }
            m_device->setBandwidth(SOAPY_SDR_RX, 0, 5e6);
        } catch (const std::exception &e) {
            qDebug() << "SoapySDRDataSource: HW init warning:" << e.what();
        }
        
        m_rxStream = m_device->setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32);
        m_device->activateStream(m_rxStream);

        // Force hardware settings again after stream is live
        m_device->setSampleRate(SOAPY_SDR_RX, 0, m_sampleRate);
        // Force 100MHz for testing
        m_device->setFrequency(SOAPY_SDR_RX, 0, 100.0e6);

        qDebug() << "SoapySDRDataSource: Stream activated at" << m_sampleRate << "Hz, FORCED Freq: 100.0 MHz";

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

    const size_t numSamples = 1024; // Match cudaSDR's BUFFER_SIZE
    std::vector<float> buff(numSamples * 2); // complex samples
    void *buffs[] = {buff.data()};

    m_stopped = false;
    uint32_t packetCount = 0;

    while (!m_stopped) {
        int flags;
        long long timeNs;
        int ret = m_device->readStream(m_rxStream, buffs, numSamples, flags, timeNs, 1000000);

        if (ret > 0) {
            packetCount++;
            if (packetCount % 100 == 0) {
                double mag = 0;
                for(int i=0; i<ret*2; i++) mag += std::abs(buff[i]);
                mag /= (ret*2);
                qDebug() << "SoapySDRDataSource: Read" << ret << "samples, avg mag:" << mag;
            }

            QList<double> samples;
            samples.reserve(ret * 2);
            for (int i = 0; i < ret; ++i) {
                samples.append((double)buff[i*2]);
                samples.append((double)buff[i*2+1]);
            }
            io->data_queue.enqueue(samples);
            emit readydata();

            // Simple backpressure: if DataProcessor is not clearing the queue, slow down reading
            if (io->data_queue.count() > 50) {
                QThread::msleep(10);
            }
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
    Q_UNUSED(frequency);
    if (m_device) {
        // Force 100MHz regardless of GUI for now
        m_device->setFrequency(SOAPY_SDR_RX, 0, 100.0e6);
    }
}

#endif // HAVE_SOAPYSDR
