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
    , m_rfSampleRate(set->getSampleRate())
    , m_numChannels(1)
    , m_minFrequency(0)
    , m_maxFrequency(0)
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

        // Query device frequency range before any hardware configuration so we
        // know the valid range even if subsequent calls throw.
        try {
            SoapySDR::RangeList ranges = m_device->getFrequencyRange(SOAPY_SDR_RX, 0);
            if (!ranges.empty()) {
                m_minFrequency = static_cast<long>(ranges.front().minimum());
                m_maxFrequency = static_cast<long>(ranges.back().maximum());
                qDebug() << "SoapySDRDataSource: Device freq range"
                         << m_minFrequency / 1.0e6 << "to" << m_maxFrequency / 1.0e6 << "MHz";
                set->setMaxFrequency(m_maxFrequency);
            }
        } catch (const std::exception &e) {
            qDebug() << "SoapySDRDataSource: Could not query frequency range:" << e.what();
        }

        // Use the sample rate from settings (e.g. 48000) to keep DSP in sync.
        // Many SoapySDR devices (e.g. LimeSDR-Mini) require a minimum RF sample
        // rate higher than the WDSP/audio rate, so query the device and clamp.
        m_sampleRate = set->getSampleRate();
        m_rfSampleRate = m_sampleRate;
        try {
            SoapySDR::RangeList srRanges = m_device->getSampleRateRange(SOAPY_SDR_RX, 0);
            if (!srRanges.empty()) {
                long minSR = static_cast<long>(srRanges.front().minimum());
                long maxSR = static_cast<long>(srRanges.back().maximum());
                qDebug() << "SoapySDRDataSource: Device sample rate range"
                         << minSR / 1.0e6 << "to" << maxSR / 1.0e6 << "MSPS";
                if (m_rfSampleRate < minSR) {
                    m_rfSampleRate = static_cast<int>(minSR);
                    qWarning() << "SoapySDRDataSource: DSP rate" << m_sampleRate
                               << "Hz below device minimum; using" << m_rfSampleRate
                               << "Hz for hardware. DSP filter bandwidth will be scaled.";
                }
            }
        } catch (const std::exception &e) {
            qDebug() << "SoapySDRDataSource: Could not query sample rate range:" << e.what();
        }
        m_device->setSampleRate(SOAPY_SDR_RX, 0, m_rfSampleRate);

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

            // Set high gain
            if (m_device->getHardwareKey() == "LimeSDR-Mini") {
                m_device->setGain(SOAPY_SDR_RX, 0, "LNA", 30.0);
                m_device->setGain(SOAPY_SDR_RX, 0, "TIA", 12.0);
                m_device->setGain(SOAPY_SDR_RX, 0, "PGA", 19.0);
            } else {
                m_device->setGain(SOAPY_SDR_RX, 0, 60.0);
            }
            m_device->setBandwidth(SOAPY_SDR_RX, 0, 5e6);
        } catch (const std::exception &e) {
            qDebug() << "SoapySDRDataSource: HW init warning:" << e.what();
        }
        
        m_rxStream = m_device->setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32);
        m_device->activateStream(m_rxStream);

        // Tune to current VFO frequency, clamped to the device's valid range
        m_device->setSampleRate(SOAPY_SDR_RX, 0, m_rfSampleRate);
        long vfo = set->getVfoFrequency(0);
        if (vfo <= 0) vfo = 14200000L;
        if (m_minFrequency > 0 && vfo < m_minFrequency) {
            qDebug() << "SoapySDRDataSource: VFO" << vfo / 1.0e6
                     << "MHz below device minimum, clamping to" << m_minFrequency / 1.0e6 << "MHz";
            vfo = m_minFrequency;
        }
        if (m_maxFrequency > 0 && vfo > m_maxFrequency)
            vfo = m_maxFrequency;
        m_device->setFrequency(SOAPY_SDR_RX, 0, static_cast<double>(vfo));

        qDebug() << "SoapySDRDataSource: Stream activated at RF" << m_rfSampleRate
                 << "Hz (DSP" << m_sampleRate << "Hz), freq:" << vfo / 1.0e6 << "MHz";

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
    if (m_device) {
        long clamped = frequency;
        if (m_minFrequency > 0 && clamped < m_minFrequency) clamped = m_minFrequency;
        if (m_maxFrequency > 0 && clamped > m_maxFrequency) clamped = m_maxFrequency;
        m_device->setFrequency(SOAPY_SDR_RX, 0, static_cast<double>(clamped));
        if (clamped != frequency)
            qDebug() << "SoapySDRDataSource: Freq" << frequency / 1.0e6 << "MHz clamped to" << clamped / 1.0e6 << "MHz";
        else
            qDebug() << "SoapySDRDataSource: Frequency set to" << clamped / 1.0e6 << "MHz";
    }
}

#endif // HAVE_SOAPYSDR
