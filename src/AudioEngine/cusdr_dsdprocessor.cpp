#include "cusdr_dsdprocessor.h"

#include <QtGlobal>
#include <algorithm>
#include <cmath>
#include <cstring>

#ifdef HAVE_DSDCC
#include "dsd_decoder.h"
#endif

DsdProcessor::DsdProcessor()
{
#ifdef HAVE_DSDCC
	m_decoder = new DSDcc::DSDDecoder();
	m_decoder->setQuiet();
	// IMPORTANT: DSDDecodeNone only clears other modes when on=true.
	// Defaults enable DMR/P25/NXDN/etc.; leaving them on causes false sync (e.g. "DMR").
	m_decoder->setDecodeMode(DSDcc::DSDDecoder::DSDDecodeNone, true);
	m_decoder->setDecodeMode(DSDcc::DSDDecoder::DSDDecodeDStar, true);
	m_decoder->setDataRate(DSDcc::DSDDecoder::DSDRate4800);
	m_decoder->enableAudioOut(true);
	m_decoder->setAudioGain(1.0f);
	// SDRangel: matched filter can hurt D-STAR symbol sync on strong signals.
	m_decoder->enableCosineFiltering(false);
	m_decoder->setSymbolPLLLock(true);
	m_decoder->useHPMbelib(true);
	// Prefer 48 kHz speech out (8 kHz × 6) when mbelib voice path is active.
	m_decoder->setUpsampling(6);
	// Optional polarity flip if voice is still garbled with solid DST sync:
	// CUSDR_DSTAR_INVERT=1
	m_invertInput = qEnvironmentVariableIntValue("CUSDR_DSTAR_INVERT") != 0;
#ifdef DSD_USE_MBELIB
	m_decoder->enableMbelib(true);
	m_voiceDecode = true;
#else
	m_decoder->enableMbelib(false);
	m_voiceDecode = false;
#endif
	m_status = m_voiceDecode
		? QStringLiteral("D-STAR: searching")
		: QStringLiteral("D-STAR: framing only (no mbelib)");
#else
	m_status = QStringLiteral("D-STAR: DSDcc not built");
#endif
}

DsdProcessor::~DsdProcessor()
{
#ifdef HAVE_DSDCC
	delete m_decoder;
	m_decoder = nullptr;
#endif
}

void DsdProcessor::appendUpsampledStereo(const short *pcm, int n, int upsample, QVector<float> &out)
{
	if (!pcm || n <= 0)
		return;
	const int factor = qMax(1, upsample);
	out.reserve(out.size() + n * factor * 2);
	for (int i = 0; i < n; ++i) {
		const float s = static_cast<float>(pcm[i]) / 32768.0f;
		for (int u = 0; u < factor; ++u) {
			out.append(s);
			out.append(s);
		}
	}
}

QVector<float> DsdProcessor::processSamples(const float *audio48k, int n)
{
	QVector<float> out;
	if (!audio48k || n <= 0)
		return out;

	out.reserve(n * 2);

#ifdef HAVE_DSDCC
	if (!m_decoder) {
		for (int i = 0; i < n; ++i) {
			out.append(0.0f);
			out.append(0.0f);
		}
		return out;
	}

	// Drain any held audio first so output length tracks input cadence.
	if (!m_outHold.empty()) {
		const int take = qMin(static_cast<int>(m_outHold.size()), n * 2);
		for (int i = 0; i < take; ++i)
			out.append(m_outHold[static_cast<size_t>(i)]);
		m_outHold.erase(m_outHold.begin(), m_outHold.begin() + take);
	}

	for (int i = 0; i < n; ++i) {
		// Leave headroom — hard ±1.0 clip into int16 garbles AMBE frames.
		float f = audio48k[i] * 0.7f;
		f = std::max(-0.95f, std::min(0.95f, f));
		if (m_invertInput)
			f = -f;
		const short sample = static_cast<short>(f * 32767.0f);
		m_decoder->run(sample);

		int nb = 0;
		short *pcm = m_decoder->getAudio1(nb);
		if (pcm && nb > 0) {
			QVector<float> chunk;
			// DSDcc upsampling>=2 already yields ~48 kHz PCM; otherwise ZOH 8→48 kHz.
			if (m_decoder->upsampling() >= 2)
				appendUpsampledStereo(pcm, nb, 1, chunk);
			else
				appendUpsampledStereo(pcm, nb, 6, chunk);
			m_decoder->resetAudio1();

			// Prefer filling this call's stereo frame; hold the rest.
			const int need = n * 2 - out.size();
			const int give = qMin(need, chunk.size());
			for (int k = 0; k < give; ++k)
				out.append(chunk.at(k));
			for (int k = give; k < chunk.size(); ++k)
				m_outHold.push_back(chunk.at(k));
		}
	}

	const auto sync = m_decoder->getSyncType();
	m_sync = (sync == DSDcc::DSDDecoder::DSDSyncDStarP
		  || sync == DSDcc::DSDDecoder::DSDSyncDStarN
		  || sync == DSDcc::DSDDecoder::DSDSyncDStarHeaderP
		  || sync == DSDcc::DSDDecoder::DSDSyncDStarHeaderN);

	char status[128] = {};
	m_decoder->formatStatusText(status);
	if (status[0] != '\0')
		m_status = QString::fromLatin1(status);
	else if (m_sync)
		m_status = QStringLiteral("D-STAR: sync");
	else if (!m_voiceDecode)
		m_status = QStringLiteral("D-STAR: framing only (no mbelib)");
	else
		m_status = QStringLiteral("D-STAR: searching");

	while (out.size() < n * 2) {
		out.append(0.0f);
		out.append(0.0f);
	}
	if (out.size() > n * 2)
		out.resize(n * 2);
#else
	Q_UNUSED(m_decoder);
	for (int i = 0; i < n; ++i) {
		out.append(0.0f);
		out.append(0.0f);
	}
#endif
	return out;
}
