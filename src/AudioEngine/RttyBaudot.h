#ifndef RTTY_BAUDOT_H
#define RTTY_BAUDOT_H

#include <QChar>
#include <QtGlobal>

/**
 * ITA2 / CCITT-2 Baudot (weather/commercial). FIGS Z is '+' not the
 * US-TTY '"'. Amateur contest loggers (Diddle, fldigi) often use US-TTY.
 */
namespace RttyBaudot {

inline constexpr char LTRS[32] = {
    '\0', 'E',  '\n', 'A',  ' ',  'S',  'I',  'U',
    '\r', 'D',  'R',  'J',  'N',  'F',  'C',  'K',
    'T',  'Z',  'L',  'W',  'H',  'Y',  'P',  'Q',
    'O',  'B',  'G',  '\0', 'M',  'X',  'V',  '\0'
};

inline constexpr char FIGS[32] = {
    '\0', '3',  '\n', '-',  ' ',  '\'', '8',  '7',
    '\r', '$',  '4',  '\a', ',',  '!',  ':',  '(',
    '5',  '+',  ')',  '2',  '#',  '6',  '0',  '1',
    '9',  '?',  '&',  '\0', '.',  '/',  ';',  '\0'
};

inline QChar decode(quint8 code, bool figs)
{
    code &= 0x1F;
    if (code == 0x1F || code == 0x1B)
        return QChar('\0');
    const char c = figs ? FIGS[code] : LTRS[code];
    return (c != '\0') ? QChar(QLatin1Char(c)) : QChar('\0');
}

/** 425/450 Hz commercial and meteorological broadcasts run 50 baud without USOS. */
inline bool isWeatherShiftHz(float shiftHz)
{
    const float s = (shiftHz >= 0.0f) ? shiftHz : -shiftHz;
    return (s > 415.0f && s < 435.0f) || (s > 440.0f && s < 460.0f);
}

} // namespace RttyBaudot

#endif // RTTY_BAUDOT_H
