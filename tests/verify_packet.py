#!/usr/bin/env python3
"""Verify canonical Roost source, annotation parity, and provenance files."""
from pathlib import Path
import hashlib, re, sys
root=Path(__file__).resolve().parents[1]
src=root/'src'
notes=root/'docs/annotated-source/src'
files=sorted(p for p in src.rglob('*') if p.is_file())
assert len(files)==9,f'Expected 9 Roost source files; found {len(files)}'
for p in files:
    annotated=notes/p.relative_to(src)
    assert annotated.is_file(), f'Annotation missing: {annotated}'
    stripped=''.join(s for s in annotated.read_text().splitlines(keepends=True)
                     if not re.match(r'^// \[READER NOTE (?:WQB038|RC2)\]',s))
    assert stripped.encode('utf-8')==p.read_bytes(), f'Annotated source drift: {p}'
assert len(list((root/'history/patches').glob('*.patch')))==7
assert len(list((root/'integration/changes').glob('*.patch')))==2
print('SOURCE_PARITY=9/9 PASS')
print('HISTORY_PATCHES=7')
print('INTEGRATION_DIFFS=2')
