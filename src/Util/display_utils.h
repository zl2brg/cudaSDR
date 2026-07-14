#ifndef DISPLAY_UTILS_H
#define DISPLAY_UTILS_H

#include <QVector>

namespace DisplayUtils {

// WDSP TX analyzer levels run hot vs RX; offset panadapter display (dB).
constexpr float kTxPanadapterDisplayDbOffset = -26.0f;

inline void applyTxPanadapterDisplayOffset(QVector<float> &spectrum)
{
    for (float &bin : spectrum)
        bin += kTxPanadapterDisplayDbOffset;
}

} // namespace DisplayUtils

#endif // DISPLAY_UTILS_H
