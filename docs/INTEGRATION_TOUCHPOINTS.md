# Existing InfiniTime files changed by the Roost integration

| Area | Existing source paths | Why it changes |
|---|---|---|
| Build registration | `src/CMakeLists.txt`, `src/displayapp/apps/CMakeLists.txt`, `src/displayapp/apps/Apps.h.in` | Compile Roost service/screen/icon; expose the app enum and default registration |
| Screen/controller lifetime | `src/displayapp/Controllers.h`, `DisplayApp.cpp`, `DisplayApp.h` | Hold service across screen sleep/overlay and construct the screen |
| Call/alarm priority | `src/components/ble/AlertNotificationService.cpp`, `src/components/alarm/AlarmController.cpp`, `src/systemtask/SystemTask.cpp`, `SystemTask.h` | Terminate sessions when a call or native alarm arrives |
| Launcher icon | `src/displayapp/screens/Symbols.h`, `Tile.cpp` | Use Roost private glyph without replacing other icons |

The automatic installer also **adds** nine Roost-specific files under `src/components/roost/`, `src/displayapp/screens/` and `src/displayapp/`. This packet does not remove or edit the Paint/Metronome application registrations or their source files. If a user has an existing `USERAPP_TYPES` cache override that omits Roost, adjust that build choice intentionally rather than force-removing other apps.

The installer applies the existing-file integration changes and copies the nine Roost source files from `src/`. It does not replace other InfiniTime applications.

Historical timing patches WQC219 and WQC221 modified only `RoostService.cpp`. The final historical patch made comment-only corrections.
