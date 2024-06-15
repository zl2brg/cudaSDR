
// *********************************************************************
// Wide band data processor



#include "cusdr_dataEngine.h"
#include "cusdr_WidebandProcessor.h"

void AudioOutProcessor::processDeviceData() {

	forever {
		m_mutex.lock();
		if (m_stopped) {
			m_stopped = false;
			m_mutex.unlock();
			break;
		}
		m_mutex.unlock();
	}
}

void AudioOutProcessor::processData() {

	forever {

		//m_dataEngine->processFileBuffer(m_dataEngine->io.data_queue.dequeue());

		m_mutex.lock();
		if (m_stopped) {
			m_stopped = false;
			m_mutex.unlock();
			break;
		}
		m_mutex.unlock();
	}
}

WideBandDataProcessor::WideBandDataProcessor(THPSDRParameter *ioData, QSDR::_ServerMode serverMode, int size)
	: QObject()
	, io(ioData)
	, set(Settings::instance())
	, m_serverMode(serverMode)
	, m_size(size)
	, m_bytes(0)
	, m_stopped(false)
{
	int result;
    char str[]="";
	m_wbSpectrumAveraging = set->getSpectrumAveragingCnt(-1);
	cpxWBIn.resize(WIDEBAND_BUFFER_SIZE);
    specBuf.resize(NUM_PIXELS);
    XCreateAnalyzer(WIDEBAND_DISPLAY_NUMBER, &result, 262144, 1, 1,str);
	if(result != 0) {
        WIDEBAND_PROCESSOR_DEBUG <<  "wideband XCreateAnalyzer failed:";
	} else {
		initWidebandAnalyzer();
	}
	setWbSpectrumAveraging(this, -1,m_wbSpectrumAveraging);
}

void  WideBandDataProcessor::initWidebandAnalyzer() {

		int flp[] = {0};
		double keep_time = 0.1;
		int n_pixout = 1;
		int spur_elimination_ffts = 1;
        int data_type = 0;
        int fft_size = 8192;
		int window_type = 4;
		double kaiser_pi = 14.0;
		int overlap = 0; //1024; //4096;
		int clip = 0;
		int span_clip_l = 0;
		int span_clip_h = 0;
        int pixels = NUM_PIXELS;
		int stitches = 1;
		int calibration_data_set = 0;
		double span_min_freq = 0.0;
		double span_max_freq = 0.0;

		int max_w = fft_size + (int) min(keep_time * (double) 10, keep_time * (double) fft_size * (double) 10);
		//overlap = (int)max(0.0, ceil(fft_size - (double)w->sample_rate / (double)w->fps));
		SetAnalyzer(WIDEBAND_DISPLAY_NUMBER,
					n_pixout,
					spur_elimination_ffts, //number of LO frequencies = number of ffts used in elimination
					data_type, //0 for real input data (I only); 1 for complex input data (I & Q)
					flp, //vector with one elt for each LO frequency, 1 if high-side LO, 0 otherwise
					fft_size, //size of the fft, i.e., number of input samples
                    WIDEBAND_BUFFER_SIZE, //number of samples transferred for each OpenBuffer()/CloseBuffer()
					window_type, //integer specifying which window function to use
					kaiser_pi, //PiAlpha parameter for Kaiser window
					overlap, //number of samples each fft (other than the first) is to re-use from the previous
					clip, //number of fft output bins to be clipped from EACH side of each sub-span
					span_clip_l, //number of bins to clip from low end of entire span
					span_clip_h, //number of bins to clip from high end of entire span
					pixels,  //number of pixel values to return.  may be either <= or > number of bins
					stitches, //number of sub-spans to concatenate to form a complete span
					calibration_data_set, //identifier of which set of calibration data to use
					span_min_freq, //frequency at first pixel value8192
					span_max_freq, //frequency at last pixel value
					max_w //max samples to hold in input ring buffers
		);

		SetDisplayDetectorMode(WIDEBAND_DISPLAY_NUMBER, 0, DETECTOR_MODE_AVERAGE);
		SetDisplayAverageMode(WIDEBAND_DISPLAY_NUMBER, 0,  AVERAGE_MODE_RECURSIVE);
}

WideBandDataProcessor::~WideBandDataProcessor() {

	DestroyAnalyzer(WIDEBAND_DISPLAY_NUMBER);
	cpxWBIn.clear();
}

void WideBandDataProcessor::stop() {

	m_stopped = true;
}

void WideBandDataProcessor::processWideBandData() {

	forever {
		processWideBandInputBuffer(io->wb_queue.dequeue());

		m_mutex.lock();
		if (m_stopped) {
			m_stopped = false;
			m_mutex.unlock();
			break;
		}
		m_mutex.unlock();
	}
}

void WideBandDataProcessor::processWideBandInputBuffer(const QByteArray &buffer) {
	int size;
    float sample;
    int s;
	//if (m_mercuryFW > 32 || m_hermesFW > 16)
	if (io->mercuryFW > 32 || io->hermesFW > 11)
		size = 2 * BIGWIDEBANDSIZE;
	else
		size = 2 * SMALLWIDEBANDSIZE;
    float norm = 1.0f / (4 * size);

	qint64 length = buffer.length();
	if (buffer.length() != size) {

		WIDEBAND_PROCESSOR_DEBUG << "wrong wide band buffer length: " << length << "size " << size <<  "ver " << io->hermesFW ;
		return;
	}


	for (int i = 0; i < length; i += 2) {
		s = (int) ((buffer.at(i + 1) << 8)  + (int)(buffer.at(i) & 0xFF));

        sample = (float)(s * norm) * 1.0f;

		cpxWBIn[i/2].re = 0;
		cpxWBIn[i/2].im = (double)sample;
	}
	getSpectrumData();
}

void WideBandDataProcessor::getSpectrumData(){
	int spectrumDataReady;

	m_mutex.lock();
    Spectrum0(1, WIDEBAND_DISPLAY_NUMBER, 0, 0,(double *) cpxWBIn.data());
	GetPixels(WIDEBAND_DISPLAY_NUMBER,0,specBuf.data(), &spectrumDataReady);
    if (spectrumDataReady)
        emit wbSpectrumBufferChanged(specBuf);
	m_mutex.unlock();
}

void WideBandDataProcessor::setWbSpectrumAveraging(QObject* sender, int rx, int value) {

	Q_UNUSED (sender)
	if (rx != -1) return;
	m_mutex.lock();
	m_wbSpectrumAveraging = value;
	double t=0.001*m_wbSpectrumAveraging;
	double  m_display_avb = exp(-1.0 / ((double)10 * t));
	int  m_display_average = max(2, (int)min(60, (double)10 * t));
	SetDisplayAvBackmult(WIDEBAND_DISPLAY_NUMBER, 0, m_display_avb);
	SetDisplayNumAverage(WIDEBAND_DISPLAY_NUMBER, 0, m_display_average);
	m_mutex.unlock();
}

