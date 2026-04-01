//
// Created by simon on 3/07/18.
//

#ifndef CUDASDR_CUSDR_WIDEBANDPROCESSOR_H
#define CUDASDR_CUSDR_WIDEBANDPROCESSOR_H

#include "cusdr_settings.h"
#include "QtWDSP/qtwdsp_dspEngine.h"

inline constexpr int NUM_PIXELS = 4096;
inline constexpr int WIDEBAND_BUFFER_SIZE = 16384;
inline constexpr int WIDEBAND_DISPLAY_NUMBER = 9;
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

    QSDR::_ServerMode		m_serverMode;
    QVector<float> 		specBuf;

    int				m_size;
    int				m_bytes;

    int 			m_wbSpectrumAveraging;
    volatile bool	m_stopped;

    void initWidebandAnalyzer();

signals:
    void	messageEvent(QString message);
    void	wbSpectrumBufferChanged(const qVectorFloat &buffer);
};




#endif //CUDASDR_CUSDR_WIDEBANDPROCESSOR_H
