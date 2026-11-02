# How Roost works

**Pure model:** `RoostCore.hpp` normalizes up to four phase slots, computes time from a monotonic millisecond clock, and returns generation-tagged cue requests. A skipped phase is omitted; all-Skip is invalid. This logic has no FreeRTOS or LVGL dependency and is host-testable.

**Native service:** `RoostService` owns the FreeRTOS one-shot timer, motor controller reference, mutex and persistent preferences. The timer can continue while the screen is asleep. `MarkUrgent` uses an atomic epoch to record a call/alarm before ordinary event delivery; an in-flight cue is rejected when its generation/epoch is stale. It never stops a motor vibration owned by an alert. A 30 ms pulse is requested at each phase boundary in this source candidate.

**Screen:** `RoostScreen` owns LVGL widgets only. It polls `Observe()` to size the circle, and offers pattern selection, Start/Stop and long-press custom editing. Destruction during sleep or a notification overlay is not a session-end signal. Physical-button exit explicitly stops the service.

**Persistence:** `RoostPreferences.hpp` encodes a 12-byte payload; `RoostService.cpp` wraps it in two alternating 18-byte LittleFS records with generation counters and CRC. The last valid generation is restored without auto-starting a session. `EditorDraft` keeps unsaved edits separate from the active pattern.

**Integration:** The automated integration step wires Roost into CMake, `Apps`/`AppTraits`, `DisplayApp` controllers, `SystemTask`, native alarm and BLE incoming-call paths, and launcher glyph handling. The original SVG was rasterized into an 18×18 1-bpp private glyph. The SVG is preserved unchanged for future modifications.

The preference-header comment documents the two alternating LittleFS record paths.
