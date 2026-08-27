#!/usr/bin/env python3
"""Repair receiver keys in a cudaSDR settings.ini that were overwritten with
garbage (out-of-range values persisted from uninitialised memory).

Only keys whose value is outside its valid range are rewritten; everything else
(frequencies, band memories, colours) is left untouched.

Usage: repair_settings_ini.py <settings.ini> [--dry-run]
"""

import re
import shutil
import sys

# key -> (validator, replacement)
INT_RANGES = {
    "fftSize": (0, 7, 1),
    "PanAverageMode": (0, 3, 1),
    "PanDetectorMode": (0, 4, 1),
    "averagingCnt": (1, 200, 5),
    "framesPerSecond": (1, 200, 25),
    "freqRulerPosition": (1, 10, 5),
    "audioVolume": (1, 100, 25),
    "agcGain": (-20, 120, 100),
    "agcMaximumGain": (0, 120, 30),
    "agcFixedGain": (0, 120, 30),
    "agcSlope": (0, 20, 0),
    "agcAttacktime": (1, 200, 2),
    "agcDecaytime": (1, 2000, 250),
    "agcHangtime": (0, 5000, 100),
}

# filterLo must stay below filterHi; a zero-width filter mutes the receiver.
FILTER_DEFAULT = {"filterLo": -3050, "filterHi": -150}


def repair(path, dry_run=False):
    with open(path, "r", encoding="utf-8") as handle:
        lines = handle.readlines()

    section = ""
    fixes = []
    filters = {}

    for index, line in enumerate(lines):
        header = re.match(r"^\[(.+)\]\s*$", line)
        if header:
            section = header.group(1)
            filters = {}
            continue
        if not section.startswith("receiver"):
            continue
        match = re.match(r"^([A-Za-z0-9_]+)=(.*)$", line.rstrip("\n"))
        if not match:
            continue
        key, raw = match.group(1), match.group(2)

        if key in INT_RANGES:
            low, high, default = INT_RANGES[key]
            try:
                value = int(raw)
            except ValueError:
                continue
            if value < low or value > high:
                fixes.append((index, section, key, raw, default))
        elif key in FILTER_DEFAULT:
            try:
                filters[key] = (index, int(raw))
            except ValueError:
                continue
            if len(filters) == 2:
                lo_index, lo = filters["filterLo"]
                hi_index, hi = filters["filterHi"]
                if lo >= hi:
                    fixes.append((lo_index, section, "filterLo", str(lo),
                                  FILTER_DEFAULT["filterLo"]))
                    fixes.append((hi_index, section, "filterHi", str(hi),
                                  FILTER_DEFAULT["filterHi"]))

    for index, section, key, old, new in fixes:
        print(f"[{section}] {key}: {old} -> {new}")
        lines[index] = f"{key}={new}\n"

    if not fixes:
        print("nothing to repair")
        return 0

    if dry_run:
        print(f"\n{len(fixes)} key(s) would be repaired (dry run)")
        return 0

    shutil.copy2(path, path + ".corrupt.bak")
    with open(path, "w", encoding="utf-8") as handle:
        handle.writelines(lines)
    print(f"\n{len(fixes)} key(s) repaired; original saved as {path}.corrupt.bak")
    return 0


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(2)
    sys.exit(repair(sys.argv[1], "--dry-run" in sys.argv))
