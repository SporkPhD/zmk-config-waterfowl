# Waterfowl ZMK config

Firmware config for a Waterfowl split keyboard (nice!nano v2 on both halves, rollers, dials and OLEDs) running ZMK v0.3 with **ZMK Studio** turned on.

The layout is a Mac-flavored take on Miryoku: home-row modifiers (Ctrl, Opt, Cmd, Shift), and thumb keys that type when tapped and open a layer when held. See [docs/layout-plan.html](docs/layout-plan.html) for drawings of every layer.

| File | What it is |
|---|---|
| `config/waterfowl.keymap` | The keymap: Base, Nav, Num, Sym and Fn layers plus 3 spare layers for Studio |
| `config/waterfowl_layout.dtsi` | The key-position map Studio uses to draw the keyboard (included by `waterfowl_left.overlay` and `waterfowl_right.overlay`) |
| `config/waterfowl.conf` | Turns on the rollers, dials and screens (both halves) |
| `config/waterfowl_left.conf`, `config/waterfowl_right.conf` | What each screen shows |
| `src/peripheral_bongo_cat.c` | Bongo Cat on the right screen, tapping along with that half's keys |
| `config/info.json` | Same key positions, for Keymap Editor |
| `build.yaml` | What GitHub builds: left (with Studio), right, settings reset |
| `firmware/` | Firmware built locally with Docker (ignored by git) |

## Everyday editing: ZMK Studio

1. Plug the **left** half into the Mac with USB.
2. Open the [ZMK Studio app](https://zmk.studio/download) or [zmk.studio](https://zmk.studio/) in Chrome or Edge, then pick "Waterfowl".
3. Unlock it when asked: hold **Del** (right outer thumb) and tap **G**.
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
   - Hold **Del** (right outer thumb) and tap **B** for the left half, or hold **Esc** (left outer thumb) and tap **/** for the right half. Those are the **BOOT** keys on the Fn layer.
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

## Layers

| Layer | How to reach it | What's on it |
|---|---|---|
| Base | always on | QWERTY. Hold A S D F or J K L ' for Ctrl, Opt, Cmd, Shift |
| Nav | hold **Space** | Arrows on H J K L, line start/end, page up/down, word jumps, tab switching, undo/cut/copy/paste/redo |
| Num | hold **Backspace** | Number pad under the left hand, 0 on the left middle thumb |
| Sym | hold **Enter** | Shifted symbols in the same spots as their Num twins |
| Fn | hold **Esc** or **Del** | F-keys on the left; Bluetooth, media and brightness on the right |

## Fn layer (hold Esc or Del)

| Key | Does |
|---|---|
| T position | Screenshot of an area (⌘⇧4) |
| G position | Unlock Studio |
| B position | BOOT (left half flashing mode) |
| Y U I O | Bluetooth device 1–4 |
| P | Forget the current Bluetooth device |
| H J K L ' | Previous, volume down, volume up, next, play/pause |
| N M , | Mute, brightness down, brightness up |
| . position | Toggle output between USB and Bluetooth |
| / position | BOOT (right half flashing mode) |

## Screens

The left screen shows the layer, Bluetooth profile or USB, both batteries, Mac modifier symbols, Caps Lock and typing speed. The right screen shows its battery, its link to the left half and a Bongo Cat that taps a paw for each key you press on the right half. Both screens turn off when you stop typing.
