# Call/alarm timing host regression

Run `python3 tests/run_urgent_timing.py` from the repository root with a native C++17 compiler (`g++`) available. This compiles the **current `src/components/roost/RoostService.cpp`** with small FreeRTOS and motor fakes and injects call/alarm events at controlled points. It checks Start and OnTimer regressions and the normal boundary pulse. No physical motor, MCU, real scheduler, or firmware is involved.

Fixtures trace to WQC219 and WQC221; preserve their source-only test provenance. A passing model is not a guarantee about real watch task ordering, shared motor ownership, or battery use. See the technical development notes in `history/`.
