#ifndef WATERFALLPALETTE_H
#define WATERFALLPALETTE_H

#include <QColor>

#include "../Settings/SettingsTypes.h"

/** Same dBm-to-colour map the waterfall shader uses. */
struct SignalPalette {
    WaterfallColorMode mode = Simple;
    float lowerThreshold = -160.0f;
    float upperThreshold = 0.0f;
    float colorRange = 160.0f;
    QColor lo = QColor(0, 0, 0);
    QColor mid = QColor(192, 124, 255);
    QColor hi = QColor(192, 124, 255);
    float alpha = 1.0f;
};

struct PaletteRgb {
    int r = 0;
    int g = 0;
    int b = 0;
};

inline float waterfallPaletteT(float dbm, const SignalPalette &palette)
{
    const float span = (palette.mode == Enhanced)
                           ? qMax(palette.colorRange, 0.001f)
                           : qMax(palette.upperThreshold - palette.lowerThreshold, 0.001f);
    float t = (dbm - palette.lowerThreshold) / span;
    if (t < 0.0f)
        t = 0.0f;
    else if (t > 1.0f)
        t = 1.0f;
    return t;
}

inline int paletteLerp(int a, int b, float t)
{
    return int((1.0f - t) * float(a) + t * float(b));
}

inline PaletteRgb waterfallPaletteColor(const SignalPalette &palette, float t)
{
    auto pack = [](int r, int g, int b) {
        PaletteRgb c;
        c.r = qBound(0, r, 255);
        c.g = qBound(0, g, 255);
        c.b = qBound(0, b, 255);
        return c;
    };

    t = qBound(0.0f, t, 1.0f);
    const QColor &lo = palette.lo;
    const QColor &mid = palette.mid;
    const QColor &hi = palette.hi;

    if (palette.mode == Simple) {
        if (t <= 0.0f)
            return pack(lo.red(), lo.green(), lo.blue());
        if (t >= 1.0f)
            return pack(255, 255, 255);
        if (t <= 0.5f) {
            const float p = t * 2.0f;
            return pack(paletteLerp(lo.red(), mid.red(), p),
                        paletteLerp(lo.green(), mid.green(), p),
                        paletteLerp(lo.blue(), mid.blue(), p));
        }
        const float p = (t - 0.5f) * 2.0f;
        return pack(paletteLerp(mid.red(), 255, p),
                    paletteLerp(mid.green(), 255, p),
                    paletteLerp(mid.blue(), 255, p));
    }

    // Enhanced — PowerSDR / KISS Konsole segments. Matches WaterfallRenderer.
    if (t <= 0.0f)
        return pack(lo.red(), lo.green(), lo.blue());
    if (t >= 1.0f)
        return pack(hi.red(), hi.green(), hi.blue());

    int r, g, b;
    if (t < 2.0f / 9.0f) {
        const float p = t / (2.0f / 9.0f);
        r = int((1.0f - p) * lo.red());
        g = int((1.0f - p) * lo.green());
        b = int(lo.blue() + p * (255 - lo.blue()));
    } else if (t < 3.0f / 9.0f) {
        const float p = (t - 2.0f / 9.0f) / (1.0f / 9.0f);
        r = 0;
        g = int(p * 255);
        b = 255;
    } else if (t < 4.0f / 9.0f) {
        const float p = (t - 3.0f / 9.0f) / (1.0f / 9.0f);
        r = 0;
        g = 255;
        b = int((1.0f - p) * 255);
    } else if (t < 5.0f / 9.0f) {
        const float p = (t - 4.0f / 9.0f) / (1.0f / 9.0f);
        r = int(p * 255);
        g = 255;
        b = 0;
    } else if (t < 7.0f / 9.0f) {
        const float p = (t - 5.0f / 9.0f) / (2.0f / 9.0f);
        r = 255;
        g = int((1.0f - p) * 255);
        b = 0;
    } else if (t < 8.0f / 9.0f) {
        const float p = (t - 7.0f / 9.0f) / (1.0f / 9.0f);
        r = 255;
        g = 0;
        b = int(p * 255);
    } else {
        const float p = (t - 8.0f / 9.0f) / (1.0f / 9.0f);
        r = int((0.75f + 0.25f * (1.0f - p)) * 255);
        g = int(p * 255 * 0.5f);
        b = 255;
    }
    return pack(r, g, b);
}

#endif // WATERFALLPALETTE_H
