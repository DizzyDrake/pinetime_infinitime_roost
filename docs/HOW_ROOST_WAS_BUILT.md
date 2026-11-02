# How Roost was put together

Roost began as a breathing app designed by Aurora for the PineTime watch, using InfiniTime 1.16.0. Aurora specified the screen, breathing patterns, haptic behavior, editor, persistence, and navigation. AI collaborators and Work Queue workers researched the InfiniTime integration, implemented and reviewed native C++ code, prepared the icon glyph, and developed host regressions. Aurora built firmware with Docker, flashed it through Gadgetbridge, tested it on a physical watch, and manually validated the **personal** version.

The public Roost integration adds the breathing app while retaining stock Paint and Metronome. It does not bring over Aurora's separate personal watch-face changes.

## Design and code map

- `RoostCore.hpp`: normalized breathing patterns and pure phase/timing calculations. Slots of zero seconds are skipped; all-zero custom patterns are rejected.
- `RoostService.h/.cpp`: persistent app service, FreeRTOS phase timer, shared motor, high-priority alarm/call events and alternating LittleFS persistence records. This lives longer than a single screen instance so haptics continue when the display sleeps.
- `RoostPreferences.hpp`: compact, versioned 12-byte preference payload; the service writes an 18-byte generation+CRC record to alternating files, helping preserve earlier valid settings if a write is interrupted.
- `RoostEditorDraft.hpp`: an in-memory editor buffer; editing does not automatically change the live pattern until saved.
- `Roost.h/.cpp`: LVGL user interface with gradient background, growing/shrinking white circle, Start/Stop and custom editor. Intentionally leaving the app stops the session; ordinary screen sleep does not.
- `RoostLauncherFont.h/.cpp`: adapter registering the supplied icon as a small private-use glyph, while delegating other glyphs to InfiniTime's existing font.

The SVG is kept in its original editable form. Its rounded-square-and-circle artwork was rasterized into an **18×18 one-bit glyph using 41 bytes** (324 bits rounded upward), then wired into the launcher font interface. The source includes that glyph array; no extra font binary is required.

## Where to change things

Presets/phase duration rules: `RoostCore.hpp`. Saved settings: `RoostPreferences.hpp` + `RoostService.cpp`. Colors/controls: `Roost.cpp`. Motor timing and alarm/call priority: `RoostService.cpp` together with upstream call/alarm ingress. Launcher symbol: original `artwork/infinitime_roost_icon.svg` and `RoostLauncherFont.cpp`. `docs/annotated-source/src/` mirrors runtime source with 47 explanatory notes identifying important dependencies and consequences of changes.

The original source packet was assembled from the original four integration patches, then Worker C's two reviewed startup/phase-boundary urgency guards and a comment correction. The original patch sequence and engineering notes are archived in `history/`.
