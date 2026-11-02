# Port Roost to a newer InfiniTime release

Roost is a breathing app for PineTime. The current integration was built for **InfiniTime 1.16.0** at commit `0fabfe99d9115c5e258a14fd0c985d300ee41f2d`. This guide is for developers adapting it to a later InfiniTime release or their own firmware fork.

You don't need to replay Roost's seven original development patches. The complete current Roost source is in [`src/`](../src/). The small diffs in [`integration/changes/`](../integration/changes/) document how the 1.16.0 integration connects to InfiniTime; treat them as **references**, not as patches guaranteed to apply to a newer version.

## 1. Start with the newer firmware source

Use a separate checkout of the InfiniTime release you want to support, including its matching submodules. For example, from outside your Roost repository:

```sh
git clone https://github.com/InfiniTimeOrg/InfiniTime.git InfiniTime-Roost-next
cd InfiniTime-Roost-next
git checkout <release-tag-or-commit>
git submodule update --init --recursive
```

Record the exact commit with `git rev-parse HEAD`. First build that upstream revision *without Roost* using the build instructions supplied with that revision. This separates upstream/toolchain issues from Roost integration issues.

The `tools/install_into_infinitime.py` command in this repository deliberately requires the pinned 1.16.0 commit; it is **not** a general-purpose porting tool. It should refuse a newer checkout rather than silently change it.

## 2. Copy Roost's own files

Copy the nine files from this repository's `src/` into the matching paths under the newer InfiniTime checkout:

| Roost source | Role |
| --- | --- |
| `components/roost/RoostCore.hpp` | Breathing patterns and timing-independent session logic |
| `components/roost/RoostEditorDraft.hpp` | Custom-pattern editing rules |
| `components/roost/RoostPreferences.hpp` | Persisted preferences format |
| `components/roost/RoostService.h`, `.cpp` | Long-lived haptics, timers, motor, settings, alert priority |
| `displayapp/screens/Roost.h`, `.cpp` | Watch interface and LVGL interactions |
| `displayapp/RoostLauncherFont.h`, `.cpp` | Launcher icon font wrapper |

Keep [`artwork/infinitime_roost_icon.svg`](../artwork/infinitime_roost_icon.svg) with the port; it's the original artwork source. Check the launcher-font wrapper if the newer firmware changes its font or LVGL API.

## 3. Reconnect Roost to InfiniTime

Use [`docs/INTEGRATION_TOUCHPOINTS.md`](INTEGRATION_TOUCHPOINTS.md) and the two reference diffs in `integration/changes/` as a map. The following are the *functions* the port must preserve, regardless of how the newer firmware organizes its files:

| Integration area | What to adapt in the target version |
| --- | --- |
| Build system | Compile `Roost.cpp`, `RoostService.cpp`, and `RoostLauncherFont.cpp`, together with their headers |
| App registry | Add `Apps::Roost` and register its icon/creator; keep the target firmware's existing user apps |
| Screen construction | Pass a reference to the Roost service when opening the Roost screen |
| Service lifetime | Own `RoostService` outside the temporary screen, initialize it after the motor and storage are ready, and keep sessions alive across display sleep |
| Call and alarm priority | End a session when an incoming call or native watch alarm arrives, **before** the alert begins using the shared motor |
| Exit behavior | Stop a session on deliberate app exit, without stopping it merely because an overlay temporarily replaces the screen |
| Launcher artwork | Map Roost's private icon glyph without changing other application icons |
| Settings | Verify Roost's selection/custom-duration save/load behavior against the target filesystem API |

### App registration may be different

The 1.16.0 integration adds Roost to `Apps.h.in`, the default user-app list, `DisplayApp`, and `Controllers.h`. On newer versions, the application-registry mechanism may use `AppTraits`, generated descriptors, or other wiring. **Follow the target version's own app model** rather than mechanically copying 1.16.0 launcher or screen-construction code.

Likewise, the 1.16.0 `DisplayApp` holds the long-lived Roost service. A future firmware may offer a more appropriate service owner. What's important is the service's lifetime and alert handling, not retaining a specific field location.

## 4. Review the parts most likely to change

- **LVGL:** `Roost.cpp` currently uses LVGL's older `lv_obj_*` widget, style and event APIs. If the target updates LVGL, adapt these calls together; the breathing core is independent of them.
- **RTOS timing:** `RoostService::NowMs()` derives milliseconds from `xTaskGetTickCount()` and `configTICK_RATE_HZ`. Check tick-rate conversion, timer callback context, and timer-command behavior on the target.
- **Haptics:** Recheck `MotorController` calls and motor ownership. Roost must not silence an incoming-call or alarm vibration when ending its session.
- **Filesystem:** Adapt `FS` reads/writes if necessary; preserve existing saved preferences when that is a goal for your upgrade. Test first-run defaults and interrupted writes.
- **Generated app selection and image size:** Confirm Roost appears in the generated app list and that your selected app set fits the target image's flash/RAM limits.
- **LittleFS logging:** The pinned 1.16.0 Docker build needed a four-macro `lfs_config.h` compatibility correction. **Don't apply that workaround automatically** to future firmware; check whether its logging implementation already differs.

## 5. Validate the port

Run the portable host-side checks first (see [`tests/HOST_TESTS.md`](../tests/HOST_TESTS.md)). Then build the complete firmware and check the generated app registration and DFU archive. On an actual watch, verify:

1. Roost opens and its launcher glyph displays correctly; other apps, including Paint and Metronome, remain available.
2. Preset patterns and a custom pattern with zero-length omitted phases work as intended, including the all-zero rejection.
3. Haptic phase cues continue with the display asleep and stop when Roost is deliberately exited.
4. Incoming calls and native alarms take priority without stray Roost motor pulses, including near a phase boundary.
5. Selected pattern and custom durations survive restart; the firmware's own update/validation procedure works.

Record the target commit, toolchain, build outcome, firmware hash, and watch observations. If something differs, document the exact changed API or behavior so the next port is easier.

## 6. Keep the port easy to maintain

Once the updated firmware works, keep its integration edits as a **small, version-labeled diff** against that exact upstream release. Don't mix them with unrelated watch-face changes or personal configuration. Add the newly supported InfiniTime version to a compatibility table in the Roost README after it has been built and tested.

The ready-to-install Gadgetbridge ZIP provided for InfiniTime 1.16.0 is a **complete firmware image**, not an app package you can drop into another firmware. A newer-version download must be built from that newer firmware source with Roost integrated.

### Compatibility record template

| InfiniTime base | Upstream commit | Integration source | ARM build | Watch test |
| --- | --- | --- | --- | --- |
| 1.16.0 | `0fabfe99d9115c5e258a14fd0c985d300ee41f2d` | Current RC2 source | Completed by Aurora | Refer to release records |
| New release | *(commit)* | *(port branch/diff)* | *(result)* | *(result)* |

For a different watch operating system—not a later InfiniTime version—the breathing core may still be reusable, but the UI, motor, scheduling, storage, and alert integrations will need a separate platform port.
