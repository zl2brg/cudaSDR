#ifndef SOAPYSDRDATASOURCE_H
#define SOAPYSDRDATASOURCE_H

#ifdef HAVE_SOAPYSDR

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QElapsedTimer>
#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Types.hpp>
#include "cusdr_settings.h"
#include "Util/cusdr_queue.h"

class SoapySDRDataSource : public QObject {
    Q_OBJECT

public:
    explicit SoapySDRDataSource(THPSDRParameter *ioData);
    ~SoapySDRDataSource();

public slots:
    void init();
    void stop();
    void runStream();

private slots:
    void setSampleRate(int value);
    void setFrequency(int rx, long frequency);

private:
    Settings* set;
    THPSDRParameter* io;
    SoapySDR::Device* m_device;
    SoapySDR::Stream* m_rxStream;

    volatile bool m_stopped;
    int m_sampleRate;
    size_t m_numChannels;

signals:
    void messageEvent(QString message);
    void readydata();
};

#endif // HAVE_SOAPYSDR

#endif // SOAPYSDRDATASOURCE_H
