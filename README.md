# Waterfowl ZMK config

Firmware config for a Waterfowl split keyboard (nice!nano v2 on both halves, rollers, dials and OLEDs) running ZMK v0.3 with **ZMK Studio** turned on.

| File | What it is |
|---|---|
| `config/waterfowl.keymap` | The stock keymap: 4 layers plus 3 spare layers for Studio |
| `config/waterfowl_layout.dtsi` | The key-position map Studio uses to draw the keyboard (included by `waterfowl_left.overlay` and `waterfowl_right.overlay`) |
| `config/waterfowl.conf` | Turns on the rollers, dials and screens |
| `config/info.json` | Same key positions, for Keymap Editor |
| `build.yaml` | What GitHub builds: left (with Studio), right, settings reset |
| `firmware/` | Firmware built locally with Docker (ignored by git) |

## Everyday editing: ZMK Studio

1. Plug the **left** half into the Mac with USB.
2. Open the [ZMK Studio app](https://zmk.studio/download) or [zmk.studio](https://zmk.studio/) in Chrome or Edge, then pick "Waterfowl".
3. Unlock it when asked: hold **DEL** (left thumb) and tap **W**.
4. Click a key, choose what it should do. Changes take effect right away and stay on the keyboard after unplugging. No reflashing needed.
5. Layers: use the layer list on the left to switch between layers, rename them, or turn on one of the three spare layers.

Studio can't edit these yet: combos, macros, tap-dance, hold-tap timing, and what the rollers and dials do when turned. Use Keymap Editor for those (below).

## Bigger changes: Keymap Editor

1. Open [Keymap Editor](https://nickcoutsos.github.io/keymap-editor/) and connect it to this GitHub repo.
2. Make the change and commit. GitHub Actions builds new firmware in about 5 minutes (repo → **Actions** tab → latest run → **firmware** download).
3. Flash `waterfowl_left-nice_nano_v2-zmk.uf2` to the left half. The keymap lives on the left half, so the right half only needs reflashing after a ZMK upgrade or when its own settings change.

**Important:** once you've saved anything in Studio, the keyboard ignores the keymap file. After flashing a Keymap Editor change, open Studio and choose **Restore Stock Settings** to load it. That wipes your Studio-only edits, so make big edits in one tool at a time.

## Flashing a half

1. Put the half into flashing mode, either way:
   - Hold **DEL** and tap the top-left key (left half) or the bottom-right key (right half). That's the **BOOT** key on the Function layer.
   - Or quickly double-tap the reset button on that half (needed the first time, since the old firmware has no BOOT key).
2. A drive named `NICENANO` shows up. Copy the matching `.uf2` file onto it. The half restarts by itself.
   macOS may say the disk wasn't ejected properly; that's normal. If Finder refuses to copy, use Terminal:

   ```bash
   cp -X ~/Downloads/firmware/waterfowl_left-nice_nano_v2-zmk.uf2 /Volumes/NICENANO/
   ```

The first time, flash **both** halves so they run the same ZMK version.

## If the halves stop talking to each other

1. Flash `settings_reset-nice_nano_v2-zmk.uf2` to **both** halves. This clears Bluetooth pairings and Studio edits.
2. Flash the normal left and right files again.
3. Restart both halves at the same time. They should pair within a few seconds.
4. Re-pair any computers you used over Bluetooth.

## Function layer (hold DEL)

| Key | Does |
|---|---|
| Q position | BOOT (left half flashing mode) |
| W position | Unlock Studio |
| E position | Forget the current Bluetooth device |
| R position | Toggle output between USB and Bluetooth |
| T / Y positions | Restart the keyboard |
| Home row, left | Bluetooth device 0–4 |
| Bottom-right (/ position) | BOOT (right half flashing mode) |
