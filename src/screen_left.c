/*
 * Left (central) screen, upright 64 x 128:
 *
 *   USB / BT1 and whether the Mac is connected
 *   battery of both halves
 *   ----
 *   layer name
 *   ----
 *   Mac modifiers, lit while held
 *   Bongo Cat, tapping along with every key press on either half
 *   typing speed
 */

#include <stdio.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <lvgl.h>

#include <zmk/battery.h>
#include <zmk/ble.h>
#include <zmk/display.h>
#include <zmk/endpoints.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/endpoint_changed.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/events/wpm_state_changed.h>
#include <zmk/hid.h>
#include <zmk/keymap.h>
#include <zmk/usb.h>
#include <zmk/wpm.h>

#include "art.h"
#include "screen.h"

#define CAT_X 7
#define CAT_Y 84
#define CAT_W 50
#define CAT_H 26

struct left_state {
    bool usb;
    uint8_t profile;
    bool connected;
    bool bonded;
    uint8_t battery;
    bool charging;
    int peripheral_battery; // -1 until the right half reports in
    char layer[12];
    zmk_mod_flags_t mods;
    int wpm;
};

static struct screen screen;
static lv_obj_t *cat;
static bool left_paw;
static struct left_state state = {.peripheral_battery = -1};
static K_MUTEX_DEFINE(state_lock);

static void read_state(struct left_state *s) {
    struct zmk_endpoint_instance ep = zmk_endpoints_selected();
    s->usb = ep.transport == ZMK_TRANSPORT_USB;
    s->profile = ep.transport == ZMK_TRANSPORT_BLE ? ep.ble.profile_index : 0;
    s->connected = zmk_ble_active_profile_is_connected();
    s->bonded = !zmk_ble_active_profile_is_open();
    s->battery = zmk_battery_state_of_charge();
    s->charging = zmk_usb_is_powered();

    zmk_keymap_layer_index_t index = zmk_keymap_highest_layer_active();
    const char *name = zmk_keymap_layer_name(zmk_keymap_layer_index_to_id(index));
    if (name && name[0]) {
        snprintf(s->layer, sizeof(s->layer), "%s", name);
    } else {
        snprintf(s->layer, sizeof(s->layer), "LAYER %d", index);
    }
    for (char *c = s->layer; *c; c++) {
        if (*c >= 'a' && *c <= 'z') {
            *c -= 'a' - 'A';
        }
    }

    s->mods = zmk_hid_get_explicit_mods();
    s->wpm = zmk_wpm_get_state();
}

static void draw_mod(int x, const char *const icon[], bool held) {
    if (held) {
        screen_rect(&screen, x, 62, 15, 13, true);
    }
    screen_bitmap(&screen, x + 3, 64, icon, ART_MOD_SIZE, ART_MOD_SIZE, held);
}

static void draw(struct k_work *work) {
    struct left_state s;
    k_mutex_lock(&state_lock, K_FOREVER);
    s = state;
    k_mutex_unlock(&state_lock);

    char text[16];
    screen_clear(&screen);

    // Where keystrokes go: USB, or a Bluetooth profile with a dot that's filled
    // when the Mac is connected (hollow when not, or not paired yet).
    if (s.usb) {
        screen_text(&screen, 0, 0, SCREEN_W, &lv_font_unscii_8, LV_TEXT_ALIGN_LEFT, "USB");
    } else {
        snprintf(text, sizeof(text), "BT%d", s.profile + 1);
        screen_text(&screen, 0, 0, SCREEN_W, &lv_font_unscii_8, LV_TEXT_ALIGN_LEFT, text);
        screen_rect(&screen, 28, 1, 6, 6, s.connected);
    }
    if (s.charging) {
        screen_text(&screen, 0, 0, SCREEN_W, &lv_font_unscii_8, LV_TEXT_ALIGN_RIGHT, "CHG");
    }

    // Batteries: left half, then right half.
    screen_text(&screen, 0, 12, 8, &lv_font_unscii_8, LV_TEXT_ALIGN_LEFT, "L");
    screen_battery(&screen, 8, 12, 23, 8, s.battery);
    screen_text(&screen, 33, 12, 8, &lv_font_unscii_8, LV_TEXT_ALIGN_LEFT, "R");
    screen_battery(&screen, 41, 12, 23, 8, s.peripheral_battery);
    snprintf(text, sizeof(text), "%d", s.battery);
    screen_text(&screen, 8, 22, 23, &lv_font_unscii_8, LV_TEXT_ALIGN_CENTER, text);
    if (s.peripheral_battery >= 0) {
        snprintf(text, sizeof(text), "%d", s.peripheral_battery);
    } else {
        snprintf(text, sizeof(text), "--");
    }
    screen_text(&screen, 41, 22, 23, &lv_font_unscii_8, LV_TEXT_ALIGN_CENTER, text);

    screen_hline(&screen, 33);
    screen_text(&screen, 0, 38, SCREEN_W, strlen(s.layer) <= 8 ? &lv_font_unscii_16 : &lv_font_unscii_8,
                LV_TEXT_ALIGN_CENTER, s.layer);
    screen_hline(&screen, 57);

    // Modifiers in the same order as the home row: Ctrl, Opt, Cmd, Shift.
    draw_mod(0, art_mod_ctrl, s.mods & (MOD_LCTL | MOD_RCTL));
    draw_mod(16, art_mod_opt, s.mods & (MOD_LALT | MOD_RALT));
    draw_mod(32, art_mod_cmd, s.mods & (MOD_LGUI | MOD_RGUI));
    draw_mod(48, art_mod_shift, s.mods & (MOD_LSFT | MOD_RSFT));

    snprintf(text, sizeof(text), "%d WPM", s.wpm);
    screen_text(&screen, 0, 116, SCREEN_W, &lv_font_unscii_8, LV_TEXT_ALIGN_CENTER, text);

    screen_commit(&screen);
}

static K_WORK_DEFINE(draw_work, draw);

static void cat_tap(struct k_work *work) {
    left_paw = !left_paw;
    lv_img_set_src(cat, left_paw ? &bongo_cat_tap1_03 : &bongo_cat_tap2_03);
}

static void cat_rest(struct k_work *work) { lv_img_set_src(cat, &bongo_cat_double_tap1_06); }

static K_WORK_DEFINE(cat_tap_work, cat_tap);
static K_WORK_DELAYABLE_DEFINE(cat_rest_work, cat_rest);

static int on_event(const zmk_event_t *eh) {
    if (!zmk_display_is_initialized()) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    const struct zmk_position_state_changed *pos = as_zmk_position_state_changed(eh);
    if (pos) {
        if (pos->state) {
            k_work_submit_to_queue(zmk_display_work_q(), &cat_tap_work);
            k_work_reschedule_for_queue(zmk_display_work_q(), &cat_rest_work, K_MSEC(150));
        }
        return ZMK_EV_EVENT_BUBBLE;
    }

    k_mutex_lock(&state_lock, K_FOREVER);
    struct left_state before = state;
    read_state(&state);
    const struct zmk_peripheral_battery_state_changed *pb =
        as_zmk_peripheral_battery_state_changed(eh);
    if (pb) {
        state.peripheral_battery = pb->state_of_charge;
    }
    bool changed = memcmp(&before, &state, sizeof(state)) != 0;
    k_mutex_unlock(&state_lock);

    if (changed) {
        k_work_submit_to_queue(zmk_display_work_q(), &draw_work);
    }
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(waterfowl_left_screen, on_event);
ZMK_SUBSCRIPTION(waterfowl_left_screen, zmk_endpoint_changed);
ZMK_SUBSCRIPTION(waterfowl_left_screen, zmk_ble_active_profile_changed);
ZMK_SUBSCRIPTION(waterfowl_left_screen, zmk_battery_state_changed);
ZMK_SUBSCRIPTION(waterfowl_left_screen, zmk_peripheral_battery_state_changed);
ZMK_SUBSCRIPTION(waterfowl_left_screen, zmk_usb_conn_state_changed);
ZMK_SUBSCRIPTION(waterfowl_left_screen, zmk_layer_state_changed);
ZMK_SUBSCRIPTION(waterfowl_left_screen, zmk_keycode_state_changed);
ZMK_SUBSCRIPTION(waterfowl_left_screen, zmk_wpm_state_changed);
ZMK_SUBSCRIPTION(waterfowl_left_screen, zmk_position_state_changed);

// Called by ZMK on the display queue when the display starts.
lv_obj_t *zmk_display_status_screen(void) {
    lv_obj_t *root = screen_create(&screen);

    cat = lv_img_create(screen.obj);
    lv_img_set_src(cat, &bongo_cat_double_tap1_06);
    screen_place_unrotated(cat, CAT_X, CAT_Y, CAT_W, CAT_H);

    k_mutex_lock(&state_lock, K_FOREVER);
    read_state(&state);
    k_mutex_unlock(&state_lock);
    draw(NULL);
    return root;
}
