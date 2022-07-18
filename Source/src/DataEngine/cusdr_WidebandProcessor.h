//
// Created by simon on 3/07/18.
//


#include "cusdr_settings.h"
#include "cusdr_dataIO.h"
#include "cusdr_receiver.h"
#include "cusdr_audioReceiver.h"
#include "cusdr_discoverer.h"
#include "Util/qcircularbuffer.h"
#include "QtWDSP/qtwdsp_dspEngine.h"

#ifndef CUDASDR_CUSDR_WIDEBANDPROCESSOR_H
#define CUDASDR_CUSDR_WIDEBANDPROCESSOR_H

#define NUM_PIXELS 4096
#define WIDEBAND_BUFFER_SIZE 16384
#define WIDEBAND_DISPLAY_NUMBER 9
// *********************************************************************
// Wide band data processor class

class WideBandDataProcessor : public QObject {

Q_OBJECT

public:
    explicit WideBandDataProcessor(THPSDRParameter *ioData = nullptr, QSDR::_ServerMode serverMode = QSDR::NoServerMode, int size = 0);
    ~WideBandDataProcessor() override;
    void	setWbSpectrumAveraging(QObject* sender, int rx, int value);

public slots:
    void	stop();
    void	processWideBandData();

private slots:
    void	processWideBandInputBuffer(const QByteArray &buffer);

    void 	getSpectrumData();

private:
    THPSDRParameter*	io;
    Settings*			set;

    CPX					cpxWBIn;
    QMutex				m_mutex;
    QByteArray			m_WBDatagram;
    QString				m_message;

    QSDR::_ServerMode		m_serverMode;
    QVector<float> 		specBuf;

    int				m_size;
    int				m_bytes;

    int 			m_wbSpectrumAveraging;
    volatile bool	m_stopped;

    unsigned char	m_ibuffer[IO_BUFFER_SIZE * IO_BUFFERS];
    void initWidebandAnalyzer();

signals:
    void	messageEvent(QString message);
    void	wbSpectrumBufferChanged(const qVectorFloat &buffer);
};




#endif //CUDASDR_CUSDR_WIDEBANDPROCESSOR_H
