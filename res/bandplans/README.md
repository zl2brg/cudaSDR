# Band plans for cudaSDR

## Allocation bars
`international.xml` — International SDR# band plan from
[zl2brg/SDR-Band-Plans](https://github.com/zl2brg/SDR-Band-Plans)
(Arrin / KN1E), **CC0 1.0** (`LICENSE.CC0`).

## Spot / frequency markers
`kiwi-dx.json` — default KiwiSDR DX label database (`dist.dx.json` from
[jks-prv/Beagle_SDR_GPS](https://github.com/jks-prv/Beagle_SDR_GPS) /
KiwiSDR). Frequencies in kHz. KiwiSDR is a **GPL/LGPL** project; this file
ships under those terms — keep attribution if redistributing.

`digimode-spots.json` — IARU Region 3 digimode dial frequencies (FT8, WSPR,
FT4, JS8, …) from AetherSDR `iaru-region3.json` spots. Merged on top of the
Kiwi set so FT8 etc. still appear where the stock Kiwi DB is sparse.

`eibi.csv` — EiBi shortwave broadcast and utility schedule database
(from [eibispace.de](http://www.eibispace.de/), by Eike Bierwirth).
Frequencies in kHz with UTC time schedules, language, target, and ITU country codes.
Free for community SDR use (`LICENSE.eibi.txt`).

## Runtime mix
- Coloured strip = SDR-Band-Plans ranges  
- Hanging labels = Kiwi DX + EiBi schedules + digimode merge
