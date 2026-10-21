# Waterfowl ZMK config

Firmware config for a Waterfowl split keyboard (nice!nano v2 on both halves, rollers, dials and OLEDs) running ZMK v0.3 with **ZMK Studio** turned on.

The layout is a Mac-flavored take on Miryoku: home-row modifiers (Ctrl, Opt, Cmd, Shift), and thumb keys that type when tapped and open a layer when held. See [docs/layout-plan.html](docs/layout-plan.html) for drawings of every layer.

| File | What it is |
|---|---|
| `config/keymap.json` | The layout as the keymap editor page saves it |
| `config/waterfowl.keymap` | Generated from `keymap.json` by `tools/build_keymap.py` |
| `docs/keymap-editor.html` | The keymap editor page |
| `config/waterfowl_layout.dtsi` | Real key positions and thumb angles (from the designer's switch plate drawing), used by Studio and the editor |
| `config/waterfowl.conf` | Turns on the rollers, dials and screens (both halves) |
| `config/waterfowl_left.conf` | Left-half extras: roller scrolling, fetching the right half's battery |
| `config/waterfowl_right.overlay` | Flips the right screen, which is mounted the other way round |
| `src/` | The two OLED screens, drawn for the Waterfowl's upright 64 × 128 panels |
| `tools/make_art.py` | The duck head and modifier icons, written to `src/art.c` |
| `config/info.json` | Same key positions, for Keymap Editor |
| `build.yaml` | What GitHub builds: left (with Studio), right, settings reset |
| `firmware/` | Firmware built locally with Docker (ignored by git) |

## Changing the layout

1. Open the [Waterfowl Keymap editor](https://claude.ai/artifact/ASKiWb19dV3EnRj4GaPEqy). It draws the board at its real key positions, with every layer, roller and dial.
2. Click a key and pick what it does. Changes save as you go, and the page shows what's changed since the last flash.
3. Tell Claude **"flash my keymap"**. Claude reads the saved layout into `config/keymap.json`, runs `python3 tools/build_keymap.py config/keymap.json` to regenerate `config/waterfowl.keymap`, builds, pushes and flashes. Keymap changes only need the left half.

`config/waterfowl.keymap` is generated, so don't edit it by hand. Edit the layout on the page, or in `config/keymap.json`, then rerun the script.

ZMK Studio still works (unlock with **Del + G**), but anything saved there takes priority over the keymap file until you choose **Restore Stock Settings** in Studio. Prefer the editor page.

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

Both screens are this repo's own code (`src/`), laid out for the Waterfowl's 128×64 OLEDs mounted upright. Each screen is drawn on a 64×128 canvas and rotated onto the panel.

- **Left:** USB or Bluetooth profile (dot filled when the Mac is connected), battery for both halves, layer name, Ctrl/Opt/Cmd/Shift while held, Bongo Cat tapping along with every key press, and typing speed.
- **Right:** link to the left half, its battery, and the severed duck head from the PCB, traced to pixels. It only redraws when the battery or link changes.

Both screens turn off when you stop typing. To change the duck head or the icons, edit the bitmaps in `tools/make_art.py` and run `python3 tools/make_art.py`. Bongo Cat's frames come from [zmk-nice-oled](https://github.com/mctechnology17/zmk-nice-oled) (MIT).
