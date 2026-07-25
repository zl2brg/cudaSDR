#ifndef CUSDR_DSDPROCESSOR_H
#define CUSDR_DSDPROCESSOR_H

#include <QString>
#include <QVector>
#include <vector>

namespace DSDcc {
class DSDDecoder;
}

/**
 * DsdProcessor — RX-only digital speech decode via f4exb/dsdcc.
 *
 * Input:  mono float audio at 48 kHz (FM discriminator / NFM audio from WDSP).
 * Output: stereo float audio at 48 kHz (decoded speech when mbelib is available).
 *
 * Default decode mode is D-STAR @ 4.8 ksym/s. No TX / encode path.
 */
class DsdProcessor {
public:
	DsdProcessor();
	~DsdProcessor();

	DsdProcessor(const DsdProcessor &) = delete;
	DsdProcessor &operator=(const DsdProcessor &) = delete;

	/** Feed n samples of 48 kHz mono float; returns stereo 48 kHz float (may be silence). */
	QVector<float> processSamples(const float *audio48k, int n);

	bool isSync() const { return m_sync; }
	QString statusText() const { return m_status; }
	bool hasVoiceDecode() const { return m_voiceDecode; }

private:
	DSDcc::DSDDecoder *m_decoder = nullptr;
	bool m_sync = false;
	bool m_voiceDecode = false;
	bool m_invertInput = false;
	QString m_status;
	std::vector<float> m_outHold; // leftover upsampled audio between calls

	void appendUpsampledStereo(const short *pcm, int n, int upsample, QVector<float> &out);
};

#endif // CUSDR_DSDPROCESSOR_H
