#!/usr/bin/env python3
"""Run deterministic urgent call/watch-alarm regressions against this packet's REAL RoostService.cpp.
Uses C++17 and deliberately minimal FreeRTOS/motor stubs; not watch/ARM testing.
"""
from pathlib import Path
import subprocess, tempfile
root=Path(__file__).resolve().parents[1]
tests=root/'tests/urgent_timing'
src=root/'src'
def run(args):
    p=subprocess.run(args,text=True,capture_output=True)
    if p.returncode: raise RuntimeError(f'{args}\n{p.stdout}\n{p.stderr}')
    print(p.stdout.strip())
with tempfile.TemporaryDirectory(prefix='roost-host-') as d:
    for test in ['test_ontimer_alert_window.cpp','test_start_urgent_window.cpp']:
        out=Path(d)/test.removesuffix('.cpp')
        run(['g++','-std=c++17','-Wall','-Wextra','-Werror','-O0','-I',str(tests/'include'),'-I',str(src),str(src/'components/roost/RoostService.cpp'),str(tests/test),'-o',str(out)])
        run([str(out)])
print('PASS: combined source, four OnTimer urgent scenarios, eight Start scenarios and normal timing control')
