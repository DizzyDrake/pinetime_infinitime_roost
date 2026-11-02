#!/usr/bin/env python3
"""Integrate canonical Roost source into a clean, pinned InfiniTime 1.16.0 checkout."""
import argparse
import hashlib
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
BASE = '0fabfe99d9115c5e258a14fd0c985d300ee41f2d'
UPSTREAM = 'https://github.com/InfiniTimeOrg/InfiniTime.git'
CHANGES = ('01-infinitime-touchpoints.patch', '02-launcher-touchpoints.patch')
LOG_MACROS = (
    ('NRF_LOG_DEBUG("[LFS] %s:%d:trace: " fmt "%s\\n", __FILE__, __LINE__, __VA_ARGS__)',
     'NRF_LOG_DEBUG("[LFS] trace: " fmt "%s\\n", __VA_ARGS__)'),
    ('NRF_LOG_DEBUG("[LFS] %s:%d:debug: " fmt "%s\\n", __FILE__, __LINE__, __VA_ARGS__)',
     'NRF_LOG_DEBUG("[LFS] debug: " fmt "%s\\n", __VA_ARGS__)'),
    ('NRF_LOG_WARNING("[LFS] %s:%d:warn: " fmt "%s\\n", __FILE__, __LINE__, __VA_ARGS__)',
     'NRF_LOG_WARNING("[LFS] warn: " fmt "%s\\n", __VA_ARGS__)'),
    ('NRF_LOG_ERROR("[LFS] %s:%d:error: " fmt "%s\\n", __FILE__, __LINE__, __VA_ARGS__)',
     'NRF_LOG_ERROR("[LFS] error: " fmt "%s\\n", __VA_ARGS__)'),
)

def run(*args, cwd=None, capture=False):
    cmd = [str(x) for x in args]
    result = subprocess.run(cmd, cwd=cwd, text=True, stdout=subprocess.PIPE if capture else None,
                            check=True)
    return result.stdout.strip() if capture else None

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--checkout', type=Path, default=ROOT.parent / 'InfiniTime-Roost')
    args = parser.parse_args()
    path = args.checkout.expanduser().resolve()
    if not path.exists():
        print(f'Cloning InfiniTime into {path}')
        run('git', 'clone', UPSTREAM, str(path))
        run('git', 'checkout', BASE, cwd=path)
    if not (path / '.git').exists():
        raise RuntimeError(f'Not a Git checkout: {path}')
    head = run('git', 'rev-parse', 'HEAD', cwd=path, capture=True)
    if head != BASE:
        raise RuntimeError(f'Expected InfiniTime 1.16.0 at {BASE}; found {head}')
    status = run('git', 'status', '--porcelain', '--untracked-files=normal', cwd=path, capture=True)
    if status:
        raise RuntimeError('Checkout has existing changes. Use a fresh destination or integrate manually.\n' + status)
    run('git', 'submodule', 'update', '--init', '--recursive', cwd=path)
    # Prepare and verify all copied files before changing the checkout.
    roost_files = sorted(p for p in (ROOT / 'src').rglob('*') if p.is_file())
    if len(roost_files) != 9:
        raise RuntimeError(f'Expected nine Roost source files; found {len(roost_files)}')
    for src in roost_files:
        relative = src.relative_to(ROOT / 'src')
        if (path / 'src' / relative).exists():
            raise RuntimeError(f'Existing Roost file would be overwritten: {relative}')
    cfg = path / 'src/libs/lfs_config.h'
    original = cfg.read_text(encoding='utf-8')
    fixed = original
    for old, new in LOG_MACROS:
        if old not in fixed:
            raise RuntimeError('LittleFS logging macro differs from the known pinned base: ' + old)
        fixed = fixed.replace(old, new)
    for filename in CHANGES:
        patch = ROOT / 'integration/changes' / filename
        print('Applying integration:', filename, flush=True)
        run('git', 'apply', '--check', str(patch), cwd=path)
        run('git', 'apply', str(patch), cwd=path)
    for src in roost_files:
        dst = path / 'src' / src.relative_to(ROOT / 'src')
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(src, dst)
    cfg.write_text(fixed, encoding='utf-8')
    for src in roost_files:
        dst = path / 'src' / src.relative_to(ROOT / 'src')
        if hashlib.sha256(src.read_bytes()).digest() != hashlib.sha256(dst.read_bytes()).digest():
            raise RuntimeError(f'Source parity failed: {src.name}')
    run('git', 'diff', '--check', cwd=path)
    print('PINNED_BASE=PASS')
    print('SOURCE_PARITY=9/9 PASS')
    print('LITTLEFS_LOGGING=FIXED_4_MACROS')
    print('ROOST_INTEGRATION=PASS')
    print(f'CHECKOUT={path}')

if __name__ == '__main__':
    try:
        main()
    except (subprocess.CalledProcessError, RuntimeError, OSError) as exc:
        print(f'INTEGRATION_FAILED: {exc}', file=sys.stderr)
        sys.exit(1)
