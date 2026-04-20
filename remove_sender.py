#!/usr/bin/env python3
"""Remove QObject* sender pattern from all source files in src/."""

import os
import re
import sys

SRC = '/home/sae/Projects/Personal/cudaSDR/src'

def process(path, content):
    original = content

    # ── 1. Remove QObject *sender as first param (more params follow) ─────────
    # Handles:  void foo(QObject *sender, int x)  →  void foo(int x)
    # Handles:  void foo(QObject* sender, int x)  →  void foo(int x)
    content = re.sub(r'QObject\s*\*\s*sender\s*,\s*', '', content)

    # ── 2. Remove QObject *sender as only param ───────────────────────────────
    # Handles:  void foo(QObject *sender)  →  void foo()
    # Also catches residual from SIGNAL(foo(QObject *)) → SIGNAL(foo())
    content = re.sub(r'\(\s*QObject\s*\*\s*sender\s*\)', '()', content)

    # ── 3. Remove Q_UNUSED(sender) lines ─────────────────────────────────────
    content = re.sub(r'[ \t]*Q_UNUSED\s*\(\s*sender\s*\)\s*\n', '', content)

    # ── 4. Remove emit foo(sender, ...) → emit foo(...)
    content = re.sub(r'(emit\s+\w+\s*\(\s*)sender\s*,\s*', r'\1', content)

    # ── 5. Remove emit foo(this, ...) → emit foo(...)
    #       Only safe for emit statements (not connect() etc.)
    content = re.sub(r'(emit\s+\w+\s*\(\s*)this\s*,\s*', r'\1', content)

    # ── 6. Remove ->setXxx(sender, ...) call sites ──────────────────────────
    content = re.sub(r'(->\s*\w+\s*\(\s*)sender\s*,\s*', r'\1', content)
    content = re.sub(r'(->\s*\w+\s*\(\s*)this\s*,\s*', r'\1', content)

    # ── 7. Remove plain funcName(sender, ...) call sites ─────────────────────
    #       (non-method calls, e.g. rcveIQEvent(sender, value))
    #       Guard: don't touch connect(this, SIGNAL...) etc.
    content = re.sub(r'(?<!\bconnect)(?<!\bdisconnect)(?<!\bnew)\b(\w+Event\s*\(\s*)sender\s*,\s*', r'\1', content)
    content = re.sub(r'(?<!\bconnect)(?<!\bdisconnect)(?<!\bnew)\b(\w+Event\s*\(\s*)this\s*,\s*', r'\1', content)

    # ── 8. SIGNAL/SLOT strings: remove QObject * as first type arg ───────────
    # SIGNAL(foo(QObject *,  →  SIGNAL(foo(
    content = re.sub(r'((?:SIGNAL|SLOT)\s*\(\s*\w+\s*\(\s*)QObject\s*\*\s*,\s*', r'\1', content)
    # SIGNAL(foo(QObject *))  →  SIGNAL(foo())  [already handled by step 2 via empty parens]
    content = re.sub(r'((?:SIGNAL|SLOT)\s*\(\s*\w+\s*\(\s*)QObject\s*\*\s*\)', r'\1)', content)

    # ── 9. Special case: if (sender == this) return;  → remove line ──────────
    content = re.sub(r'[ \t]*if\s*\(\s*sender\s*==\s*this\s*\)\s*return\s*;\s*\n', '', content)

    # ── 10. Special case: receiver.cpp: sender != this guard ─────────────────
    content = content.replace(
        'if (sender != this && m_audioMode == mode) return;',
        'if (m_audioMode == mode) return;')

    # ── 11. Special case: settings.cpp sender != this guard ──────────────────
    content = content.replace(
        'if (m_receiverDataList[rx].hamBand == band && sender != this)',
        'if (m_receiverDataList[rx].hamBand == band)')

    # ── 12. Special case: qDebug with sender in settings.cpp ─────────────────
    content = re.sub(r'[ \t]*qDebug\(\)\s*<<[^\n]*"sender"[^\n]*sender[^\n]*\n', '', content)

    return content, content != original

changed = 0
missed_sender = []

for root, dirs, files in os.walk(SRC):
    # Skip build dirs
    dirs[:] = [d for d in dirs if d not in ('build', 'build-release')]
    for fname in files:
        if not (fname.endswith('.cpp') or fname.endswith('.h')):
            continue
        fpath = os.path.join(root, fname)
        with open(fpath, 'r', errors='replace') as f:
            content = f.read()
        new_content, was_changed = process(fpath, content)
        if was_changed:
            with open(fpath, 'w') as f:
                f.write(new_content)
            changed += 1
            rel = os.path.relpath(fpath, SRC)
            print(f"  updated: {rel}")

        # Check for any remaining raw 'sender' references after processing
        # (excluding comments and string literals is hard, so just flag files)
        if 'sender' in new_content and fname.endswith('.cpp'):
            # Only flag if sender appears outside comments
            lines_with_sender = [
                (i+1, l.strip()) for i, l in enumerate(new_content.splitlines())
                if 'sender' in l and not l.strip().startswith('//')
                   and 'setSender' not in l and 'getSender' not in l
                   and 'senderWidget' not in l and 'senderName' not in l
            ]
            if lines_with_sender:
                missed_sender.append((os.path.relpath(fpath, SRC), lines_with_sender[:5]))

print(f"\nModified {changed} files.")
if missed_sender:
    print("\nFiles with remaining 'sender' references (non-comment, check manually):")
    for f, lines in missed_sender:
        print(f"  {f}:")
        for lineno, line in lines:
            print(f"    {lineno}: {line}")
