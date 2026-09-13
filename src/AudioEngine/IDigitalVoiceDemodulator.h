/* Copyright (C)
 *
 * Simon Eatough ZL2BRG
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef CUSDR_IDIGITAL_VOICE_DEMODULATOR_H
#define CUSDR_IDIGITAL_VOICE_DEMODULATOR_H

#include <QVector>

/**
 * @class IDigitalVoiceDemodulator
 * @brief Common interface for digital voice demodulators (FreeDV, RADE).
 *
 * Ingests 48 kHz mono demodulated baseband audio from WDSP and decodes speech
 * to 48 kHz stereo float audio for output.
 */
class IDigitalVoiceDemodulator {
public:
    virtual ~IDigitalVoiceDemodulator() = default;

    /**
     * @brief Process n samples of demodulated USB audio at 48 kHz.
     * @param audio48k Pointer to 48 kHz mono float audio samples
     * @param n Number of samples in audio48k
     * @return Decoded speech as 48 kHz stereo float (may be empty if accumulating frames)
     */
    virtual QVector<float> processSamples(const float* audio48k, int n) = 0;

    /**
     * @brief Returns true if receiver has modem frame synchronization.
     */
    virtual bool isSync() const = 0;

    /**
     * @brief Estimated Signal-to-Noise Ratio in dB.
     */
    virtual float getSNR() const = 0;
};

#endif // CUSDR_IDIGITAL_VOICE_DEMODULATOR_H
