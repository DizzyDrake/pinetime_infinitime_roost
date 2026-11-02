# Build Roost from source

Roost integrates with the complete InfiniTime 1.16.0 firmware; it is not an independently loadable watch app. You'll need Git, Python 3, and Docker with the `infinitime/infinitime-build` image.

## One-command integration, one-command build

Clone this Roost repository, then run these commands from its root:

```sh
python tools/install_into_infinitime.py --checkout ../InfiniTime-Roost
python tools/build_firmware.py --checkout ../InfiniTime-Roost
```

`install_into_infinitime.py` creates a separate InfiniTime checkout if the destination doesn't exist, pins it to `0fabfe99d9115c5e258a14fd0c985d300ee41f2d`, initializes its submodules, applies the integration touchpoints automatically, copies the nine canonical Roost source files, and fixes four LittleFS logging macros needed by this pinned base in the established Docker build environment.

For a checkout you already have at that exact commit, pass its path with `--checkout`; the tool requires a clean starting tree to avoid changing unrelated customizations. It doesn't need the historical patches.

`build_firmware.py` runs Docker and streams compiler output to your terminal and to `roost-arm-build.log` inside the checkout. The finished DFU package is typically at `build/output/pinetime-mcuboot-app-dfu-1.16.0.zip`; a second byte-identical copy may appear in `build/src/`. The script reports hashes instead of assuming there is only one copy.

On Windows, use PowerShell and `python` or `py -3`, whichever invokes Python 3 on your computer. Docker Desktop should be running with Linux containers enabled.

## Project map

- `src/` — the nine runtime Roost source files
- `integration/changes/` — existing InfiniTime integration points (used automatically)
- `tools/install_into_infinitime.py` — pinned-source preparation
- `tools/build_firmware.py` — Docker ARM compilation
- `docs/annotated-source/` — in-depth annotated source
- `history/patches/` — the original seven engineering patches, kept for provenance

## Compatibility

The integration changes target **InfiniTime 1.16.0** at the exact commit above. Other InfiniTime revisions may have different application registration and service APIs. The build includes the rest of InfiniTime, including stock Paint and Metronome. It does not import Aurora's separate personal watch-face modifications.

## Troubleshooting

When compilation fails, inspect `roost-arm-build.log` and report the **first** compiler error. For a clean pinned 1.16.0 build, the integration script already handles the known `LOG_INTERNAL_7` LittleFS logging mismatch.
