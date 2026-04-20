import re

with open('src/cusdr_settings.cpp', 'r') as f:
    content = f.read()

original = content

# ── 1. Remove `this,` from all direct Settings method calls ──────────────
# Handles: setFoo(this, arg) → setFoo(arg)
# Handles: emit signal(this, arg) → emit signal(arg)
# Catches both -> and non--> call patterns
content = re.sub(r'\b((?:emit\s+)?\w+\s*\(\s*)this\s*,\s*', r'\1', content)

# ── 2. Fix broken qDebug line (line ~3699) ───────────────────────────────
# qDebug() << ... << "" << ;  → remove entire line
content = re.sub(r'[ \t]*qDebug\(\)[^\n]*<<\s*""\s*<<\s*;\n', '', content)

# ── 3. Fix broken if condition (line ~3700) ──────────────────────────────
# if (... == band && != this) → if (... == band)
content = content.replace(
    'if (m_receiverDataList[rx].hamBand == band && != this)',
    'if (m_receiverDataList[rx].hamBand == band)')

# ── 4. Fix broken SETTINGS_DEBUG line (line ~3885) ───────────────────────
# SETTINGS_DEBUG << "set ... (: " << << ")";  → remove the broken << << part
content = re.sub(r'(SETTINGS_DEBUG\s*<<[^\n]*"[^"]*\(:\s*"\s*<<)\s*<<\s*("\)")', r'\1 \2', content)

# ── 5. Remove commented broken debug lines ───────────────────────────────
# //SETTINGS_DEBUG << ": " << ;
content = re.sub(r'[ \t]*//SETTINGS_DEBUG\s*<<\s*":\s*"\s*<<\s*;\n', '', content)

# ── 6. Also handle SETTINGS_DEBUG << ... << ;  (non-commented) ───────────
content = re.sub(r'(SETTINGS_DEBUG\s*<<[^\n]*)\s*<<\s*;\n', r'\1;\n', content)

lines_changed = sum(1 for a, b in zip(original.splitlines(), content.splitlines()) if a != b)
print(f"Lines changed: {lines_changed}")
print(f"Original length: {len(original.splitlines())}, New length: {len(content.splitlines())}")

# Sanity check - look for remaining issues
remaining = []
for i, line in enumerate(content.splitlines(), 1):
    if '<< ;' in line and not line.strip().startswith('//'):
        remaining.append(f"  {i}: {line.rstrip()}")
    if '!= this)' in line:
        remaining.append(f"  {i}: {line.rstrip()}")
    if re.search(r'emit\s+\w+\s*\(\s*this\s*,', line) and not line.strip().startswith('//'):
        remaining.append(f"  {i}: {line.rstrip()}")
    if re.search(r'\bset\w+\s*\(\s*this\s*,', line) and not line.strip().startswith('//'):
        remaining.append(f"  {i}: {line.rstrip()}")

if remaining:
    print("REMAINING ISSUES:")
    for r in remaining:
        print(r)
else:
    print("No remaining issues found.")

with open('src/cusdr_settings.cpp', 'w') as f:
    f.write(content)
print("Written.")
