"""Turns a layout saved by the keymap editor page (docs/keymap-editor.html)
into config/waterfowl.keymap.

    python3 tools/build_keymap.py config/keymap.json

config/keymap.json is the layout as the page saves it; keep it in the repo so
the keymap can always be regenerated. Edit the layout on the page (or in that
file), not the generated .keymap.
"""

import json
import re
import sys

HOME_LEFT = {10: "L", 11: "L", 12: "L", 13: "L"}
HOME_RIGHT = {16: "R", 17: "R", 18: "R", 19: "R"}
MOD_KEY = {"cmd": "GUI", "opt": "ALT", "ctrl": "CTRL", "shift": "SHFT"}
MOD_FN = {"cmd": "LG", "opt": "LA", "ctrl": "LC", "shift": "LS"}
LABEL_MOD = {"cmd": "Cmd", "opt": "Opt", "ctrl": "Ctrl", "shift": "Shift"}

# Rotations: (cw binding, ccw binding) as sensor-rotate behaviours or &inc_dec_kp args.
ENC = {
    "volume": ("kp", "C_VOL_DN", "C_VOL_UP"),
    "brightness": ("kp", "C_BRI_DN", "C_BRI_UP"),
    "arrows_lr": ("kp", "LEFT", "RIGHT"),
    "arrows_ud": ("kp", "DOWN", "UP"),
    "pages": ("kp", "PG_DN", "PG_UP"),
    "scroll_v": ("beh", "&msc MOVE_Y(-ROLLER_SCROLL)", "&msc MOVE_Y(ROLLER_SCROLL)", "tap-ms = <30>;"),
    "scroll_h": ("beh", "&msc MOVE_X(ROLLER_SCROLL)", "&msc MOVE_X(-ROLLER_SCROLL)", "tap-ms = <30>;"),
    "tabs": ("beh", "&kp TAB_NEXT", "&kp TAB_PREV", None),
    "words": ("beh", "&kp WORD_FWD", "&kp WORD_BCK", None),
    "zoom": ("beh", "&kp LG(EQUAL)", "&kp LG(MINUS)", None),
    "undo": ("beh", "&kp MAC_UNDO", "&kp MAC_REDO", None),
    "none": ("beh", "&none", "&none", None),
}
ENC_NAME = {"scroll_v": "scroll_enc", "scroll_h": "hscroll_enc", "tabs": "tab_enc", "words": "word_enc",
            "zoom": "zoom_enc", "undo": "undo_enc", "none": "none_enc"}


def chord(key, mods):
    out = key
    for m in ("ctrl", "opt", "shift", "cmd"):
        if m in (mods or []):
            out = f"{MOD_FN[m]}({out})"
    return out


def side(pos):
    return "R" if pos % 10 >= 5 and pos < 30 or pos in (35, 36, 37, 38, 39) else "L"


def binding(b, pos, layer_const):
    t = b["t"]
    if t == "key":
        return f"&kp {chord(b['key'], b.get('mods'))}"
    if t == "hold":
        tap = chord(b["tap"]["key"], b["tap"].get("mods"))
        h = b["hold"]
        if "layer" in h:
            return f"&lt {layer_const[h['layer']]} {tap}"
        if pos in HOME_LEFT:
            return f"&hml L{MOD_KEY[h['mod']]} {tap}"
        if pos in HOME_RIGHT:
            return f"&hmr R{MOD_KEY[h['mod']]} {tap}"
        return f"&mt {side(pos)}{MOD_KEY[h['mod']]} {tap}"
    if t == "layer":
        return {"hold": "&mo", "switch": "&to", "toggle": "&tog", "once": "&sl"}[b["how"]] + " " + layer_const[b["layer"]]
    if t == "mod":
        return ("&sk " if b.get("sticky") else "&kp ") + side(pos) + MOD_KEY[b["mod"]]
    if t == "bt":
        return {"sel": f"&bt BT_SEL {b.get('n', 0)}", "clr": "&bt BT_CLR", "next": "&bt BT_NXT", "prev": "&bt BT_PRV"}[b["action"]]
    if t == "sys":
        return {"bootloader": "&bootloader", "reset": "&sys_reset", "studio": "&studio_unlock",
                "capsword": "&caps_word", "repeat": "&key_repeat", "usbbt": "&out OUT_TOG"}[b["action"]]
    if t == "mouse":
        return "&mkp " + {"left": "LCLK", "right": "RCLK", "middle": "MCLK"}[b["button"]]
    if t == "trans":
        return "&trans"
    if t == "none":
        return "&none"
    raise ValueError(f"unknown binding {b}")


def short(b):
    """A few characters for the comment diagram above each layer."""
    t = b["t"]
    if t == "key":
        k = b["key"].replace("C_", "").replace("N", "", 1) if re.fullmatch(r"N\d", b["key"]) else b["key"]
        return "".join(LABEL_MOD[m] + "+" for m in b.get("mods") or []) + k
    if t == "hold":
        h = b["hold"]
        return b["tap"]["key"] + "/" + (h.get("layer") or LABEL_MOD[h["mod"]])
    if t == "layer":
        return b["how"] + ":" + b["layer"]
    if t == "mod":
        return LABEL_MOD[b["mod"]] + ("*" if b.get("sticky") else "")
    if t == "bt":
        return "BT" + (str(b.get("n", 0) + 1) if b["action"] == "sel" else b["action"])
    if t == "sys":
        return b["action"]
    if t == "mouse":
        return b["button"] + "clk"
    return {"trans": "-", "none": "x"}[t]


def build(doc):
    layers = doc["layers"]
    consts = {}
    for L in layers:
        c = re.sub(r"[^A-Z0-9]", "_", L["id"].upper())
        consts[L["id"]] = c if c not in consts.values() else c + "_" + str(len(consts))

    # Encoder bindings per layer, resolving "Same as Base".
    used = {}
    def enc_binding(e, li):
        enc = layers[li]["enc"][e]
        if enc["preset"] == "inherit":
            enc = layers[0]["enc"][e]
        p, rev = enc["preset"], bool(enc.get("reverse"))
        spec = ENC[p]
        if spec[0] == "kp":
            a, b = (spec[2], spec[1]) if rev else (spec[1], spec[2])
            return f"&inc_dec_kp {a} {b}"
        name = ENC_NAME[p] + ("_rev" if rev else "")
        used[name] = (p, rev)
        return "&" + name

    sensor = [[enc_binding(e, li) for e in range(4)] for li in range(len(layers))]

    out = []
    w = out.append
    w("/*\n * Waterfowl keymap for macOS.\n *\n")
    w(" * GENERATED by tools/build_keymap.py from config/keymap.json, which the\n")
    w(" * keymap editor page saves. Change the layout there, not here.\n *\n")
    w(" * Each layer has 40 bindings, laid out as on the board:\n")
    w(" *   10 top row  |  10 home row  |  10 bottom row\n")
    w(" *   L roller press, L thumbs (outer, middle, inner), L dial press |\n")
    w(" *   R dial press, R thumbs (inner, middle, outer), R roller press\n *\n")
    w(" * sensor-bindings are the rotations, in order:\n")
    w(" *   left roller, left dial, right dial, right roller\n")
    w(" * The rollers spin side to side.\n */\n\n")
    for inc in ("behaviors.dtsi", "dt-bindings/zmk/keys.h", "dt-bindings/zmk/bt.h", "dt-bindings/zmk/outputs.h",
                "dt-bindings/zmk/pointing.h"):
        w(f"#include <{inc}>\n")
    w("\n")
    for i, L in enumerate(layers):
        w(f"#define {consts[L['id']]:<5} {i}\n")
    w("""
// Key positions, for home-row mods that only trigger with the other hand.
#define KEYS_L 0 1 2 3 4 10 11 12 13 14 20 21 22 23 24
#define KEYS_R 5 6 7 8 9 15 16 17 18 19 25 26 27 28 29
#define THUMBS 30 31 32 33 34 35 36 37 38 39

// macOS shortcuts used by the roller and dial behaviours below
#define MAC_UNDO  LG(Z)
#define MAC_REDO  LG(LS(Z))
#define WORD_BCK  LA(LEFT)
#define WORD_FWD  LA(RIGHT)
#define TAB_PREV  LC(LS(TAB))
#define TAB_NEXT  LC(TAB)

// One roller click = one scroll step. Each click is a 30 ms tap and mouse
// scrolling runs in 16 ms ticks, so the speed has to cover a whole step in one
// tick (63 x 16 / 1000 = 1); ZMK's default of 10 never adds up to a step.
#define ROLLER_SCROLL 63

&lt {
    quick-tap-ms = <175>;
};

/ {
    // Studio needs the physical layout from waterfowl_layout.dtsi, so drop the
    // stock shield's matrix-transform here (keymaps load after shield files).
    chosen {
        /delete-property/ zmk,matrix-transform;
    };

    behaviors {
        // "Timeless" home-row mods: a mod only triggers when the next key is on
        // the other hand, and never in the middle of a fast run of typing.
""")
    for name, label, keys in (("hml", "left", "KEYS_R"), ("hmr", "right", "KEYS_L")):
        w(f"""        {name}: homerow_mods_{label} {{
            compatible = "zmk,behavior-hold-tap";
            display-name = "Home Row Mod ({label})";
            #binding-cells = <2>;
            flavor = "balanced";
            tapping-term-ms = <280>;
            quick-tap-ms = <175>;
            require-prior-idle-ms = <150>;
            bindings = <&kp>, <&kp>;
            hold-trigger-key-positions = <{keys} THUMBS>;
            hold-trigger-on-release;
        }};

""")
    for name, (p, rev) in sorted(used.items()):
        _, cw, ccw, extra = ENC[p]
        if rev:
            cw, ccw = ccw, cw
        node = name.replace("_enc", "_encoder")
        w(f"        {name}: {node} {{\n")
        w('            compatible = "zmk,behavior-sensor-rotate";\n')
        w("            #sensor-binding-cells = <0>;\n")
        w(f"            bindings = <{cw}>, <{ccw}>;\n")
        if extra:
            w(f"            {extra}\n")
        w("        };\n\n")
    w("    };\n\n    keymap {\n        compatible = \"zmk,keymap\";\n")
    for li, L in enumerate(layers):
        b = [binding(L["keys"][p], p, consts) for p in range(40)]
        lab = [short(L["keys"][p]) for p in range(40)]
        w(f"\n        {re.sub(r'[^a-z0-9_]', '_', L['id'].lower())}_layer {{\n")
        w(f"            display-name = \"{L['name']}\";\n")
        w("/*\n")
        for r in range(3):
            w(" *  " + " ".join(f"{x:<8}" for x in lab[r * 10:r * 10 + 5]) + "   " + " ".join(f"{x:<8}" for x in lab[r * 10 + 5:r * 10 + 10]) + "\n")
        w(" *  " + " ".join(f"{x:<8}" for x in lab[30:35]) + "   " + " ".join(f"{x:<8}" for x in lab[35:40]) + "\n */\n")
        w("            bindings = <\n")
        for r in range(4):
            row = b[r * 10:r * 10 + 10]
            w("    " + " ".join(f"{x:<16}" for x in row[:5]) + "  " + " ".join(f"{x:<16}" for x in row[5:]) + "\n")
        w("            >;\n\n")
        w(f"            sensor-bindings = <{' '.join(sensor[li])}>;\n")
        w("        };\n")
    w("""
        // Spare layers for ZMK Studio.
        extra1 {
            status = "reserved";
        };

        extra2 {
            status = "reserved";
        };

        extra3 {
            status = "reserved";
        };
    };
};
""")
    return "\n".join(line.rstrip() for line in "".join(out).split("\n"))


def main():
    doc = json.load(open(sys.argv[1]))
    for L in doc["layers"]:
        assert len(L["keys"]) == 40 and len(L["enc"]) == 4, L["name"]
    open("config/waterfowl.keymap", "w").write(build(doc))


if __name__ == "__main__":
    main()
