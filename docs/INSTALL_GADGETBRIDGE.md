# Install Roost with Gadgetbridge

Roost runs on PineTime as part of an InfiniTime firmware update. You can install the prebuilt firmware ZIP using Gadgetbridge on Android.

## Install

1. Open [Roost's GitHub Releases](../../../releases/latest) and download the file ending **`-DFU.zip`** to your Android phone. **Leave this ZIP zipped.** If you've downloaded the *installation bundle* instead, extract that outer ZIP and use the DFU ZIP inside it.
2. Connect your PineTime to [Gadgetbridge](https://gadgetbridge.org/).
3. On the PineTime's device card in Gadgetbridge, open **⋮ → File Installer**, then select the downloaded DFU ZIP. You can also open the file from your Android file manager with Gadgetbridge's firmware installer.
4. Follow the on-screen prompts. The watch will restart after the firmware transfer.
5. On the watch, swipe right from the watch face, open **Settings → Firmware**, and choose **Validate**. This keeps the new firmware active after a later restart.
6. Open the app launcher and select **Roost**.

## What you're installing

This is a full InfiniTime 1.16.0 firmware image that adds Roost and keeps the standard Paint and Metronome apps. It replaces the currently installed application firmware, so an existing customized InfiniTime build should be rebuilt with Roost's source instead.

## Troubleshooting

- **Installer doesn't appear:** connect the watch in Gadgetbridge first; choose *File Installer* from its device menu.
- **Firmware reverted after restarting:** finish **Settings → Firmware → Validate** on the watch after updating.
- **Want to keep your own firmware changes?** Use the [source integration guide](BUILD_FROM_SOURCE.md).

## References

- [InfiniTime — Updating with Gadgetbridge](https://github.com/InfiniTimeOrg/InfiniTime/blob/main/doc/gettingStarted/ota-gadgetbridge.md)
- [InfiniTime — Firmware validation](https://github.com/InfiniTimeOrg/InfiniTime/blob/main/doc/gettingStarted/updating-software.md)
- [Gadgetbridge — Firmware installer](https://gadgetbridge.org/internals/features/installer/)
