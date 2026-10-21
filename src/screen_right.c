/*
 * Right (peripheral) screen, upright 64 x 128:
 *
 *   whether this half is connected to the left half
 *   battery
 *   ----
 *   the duck
 *
 * It only redraws when the battery or the link changes, so it costs almost no
 * power. The right half doesn't know the layer, modifiers or typing speed;
 * those live on the left half.
 */

#include <stdio.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <lvgl.h>

#include <zmk/battery.h>
#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/split_peripheral_status_changed.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/split/bluetooth/peripheral.h>
#include <zmk/usb.h>

#include "art.h"
#include "screen.h"

struct right_state {
    bool connected;
    uint8_t battery;
    bool charging;
};

static struct screen screen;
static struct right_state state;
static K_MUTEX_DEFINE(state_lock);

static void read_state(struct right_state *s) {
    s->connected = zmk_split_bt_peripheral_is_connected();
    s->battery = zmk_battery_state_of_charge();
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
    s->charging = zmk_usb_is_powered();
#endif
}

static void draw(struct k_work *work) {
    struct right_state s;
    k_mutex_lock(&state_lock, K_FOREVER);
    s = state;
    k_mutex_unlock(&state_lock);

    char text[8];
    screen_clear(&screen);

    // A filled dot when linked to the left half, hollow when not.
    screen_text(&screen, 0, 0, SCREEN_W, &lv_font_unscii_8, LV_TEXT_ALIGN_LEFT, "LINK");
    screen_rect(&screen, 36, 1, 6, 6, s.connected);
    if (s.charging) {
        screen_text(&screen, 0, 0, SCREEN_W, &lv_font_unscii_8, LV_TEXT_ALIGN_RIGHT, "CHG");
    }

    screen_battery(&screen, 0, 13, 36, 10, s.battery);
    snprintf(text, sizeof(text), "%d%%", s.battery);
    screen_text(&screen, 36, 14, 28, &lv_font_unscii_8, LV_TEXT_ALIGN_RIGHT, text);

    screen_hline(&screen, 30);
    screen_bitmap(&screen, (SCREEN_W - ART_DUCK_W) / 2, SCREEN_H - ART_DUCK_H, art_duck,
                  ART_DUCK_W, ART_DUCK_H, false);

    screen_commit(&screen);
}

static K_WORK_DEFINE(draw_work, draw);

static int on_event(const zmk_event_t *eh) {
    if (!zmk_display_is_initialized()) {
        return ZMK_EV_EVENT_BUBBLE;
    }
    k_mutex_lock(&state_lock, K_FOREVER);
    struct right_state before = state;
    read_state(&state);
    bool changed = memcmp(&before, &state, sizeof(state)) != 0;
    k_mutex_unlock(&state_lock);

    if (changed) {
        k_work_submit_to_queue(zmk_display_work_q(), &draw_work);
    }
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(waterfowl_right_screen, on_event);
ZMK_SUBSCRIPTION(waterfowl_right_screen, zmk_battery_state_changed);
ZMK_SUBSCRIPTION(waterfowl_right_screen, zmk_split_peripheral_status_changed);
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(waterfowl_right_screen, zmk_usb_conn_state_changed);
#endif

// Called by ZMK on the display queue when the display starts.
lv_obj_t *zmk_display_status_screen(void) {
    lv_obj_t *root = screen_create(&screen);
    k_mutex_lock(&state_lock, K_FOREVER);
    read_state(&state);
    k_mutex_unlock(&state_lock);
    draw(NULL);
    return root;
}
