#!/usr/bin/env python3
"""Build the InfiniTime + Roost firmware with Docker and report DFU checksums."""
import argparse
import hashlib
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--checkout', type=Path, default=ROOT.parent / 'InfiniTime-Roost')
    parser.add_argument('--log', type=Path)
    args = parser.parse_args()
    checkout = args.checkout.expanduser().resolve()
    if not (checkout / 'src').is_dir():
        raise RuntimeError(f'Missing firmware checkout: {checkout}')
    if not (checkout / 'src/components/roost/RoostService.cpp').is_file():
        raise RuntimeError('Roost integration is not present in this checkout')
    log = (args.log.expanduser().resolve() if args.log else checkout/'roost-arm-build.log')
    if log.exists():
        raise RuntimeError(f'Existing build log: {log}; choose --log to keep it')
    command = ['docker', 'run', '--rm', '-v', str(checkout) + ':/sources', 'infinitime/infinitime-build']
    print('BUILD_MODE=full InfiniTime firmware + Roost', flush=True)
    print(f'BUILD_LOG={log}', flush=True)
    with log.open('w', encoding='utf-8', errors='replace') as out:
        process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        for raw in iter(process.stdout.readline, b''):
            line = raw.decode('utf-8', errors='replace')
            out.write(line); out.flush()
            print(line, end='', flush=True)
        code = process.wait()
    print(f'BUILD_EXIT_CODE={code}')
    if code:
        raise RuntimeError(f'ARM compilation failed; see {log}')
    dfus = sorted((checkout/'build').rglob('pinetime-mcuboot-app-dfu-*.zip'))
    if not dfus:
        raise RuntimeError('Docker exited successfully but no DFU ZIP was found')
    groups = {}
    for file in dfus:
        data = file.read_bytes()
        h = hashlib.sha256(data).hexdigest()
        groups.setdefault(h, []).append(file)
        print(f'DFU_ZIP={file}')
        print(f'DFU_BYTES={len(data)}')
        print(f'DFU_SHA256={h}')
    print(f'DFU_COPIES={len(dfus)}')
    print(f'DFU_DISTINCT_HASHES={len(groups)}')
    if len(groups) > 1:
        raise RuntimeError('Different DFU output hashes: inspect before distributing')
    print('BUILD_OUTPUT_CHECK=PASS')

if __name__ == '__main__':
    try:
        main()
    except (RuntimeError, OSError, subprocess.CalledProcessError) as exc:
        print(f'BUILD_FAILED: {exc}', file=sys.stderr)
        sys.exit(1)
