/*
 * Bongo Cat for the right half's screen.
 *
 * nice-oled's Bongo Cat reacts to typing speed, which only the left half knows.
 * This one taps a paw whenever a key on this half is pressed, alternating
 * paws, and goes back to resting shortly after. It only redraws on key
 * presses, so it costs little battery.
 *
 * Frames come from nice-oled (assets/bongo_cat_images.c, MIT licensed), which
 * builds them into the right half too. They are already drawn for the
 * screen's unrotated orientation, so they're placed directly on the screen.
 *
 * LVGL is not thread-safe, so every LVGL call runs on ZMK's display work queue.
 */

#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <lvgl.h>

#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>

LV_IMG_DECLARE(bongo_cat_double_tap1_06);
LV_IMG_DECLARE(bongo_cat_tap1_03);
LV_IMG_DECLARE(bongo_cat_tap2_03);

static lv_obj_t *cat;
static bool left_paw;

static void create_cat(struct k_work *work) {
    cat = lv_img_create(lv_scr_act());
    lv_img_set_src(cat, &bongo_cat_double_tap1_06);
    lv_obj_align(cat, LV_ALIGN_TOP_LEFT, CONFIG_WATERFOWL_PERIPHERAL_BONGO_CAT_X,
                 CONFIG_WATERFOWL_PERIPHERAL_BONGO_CAT_Y);
}

static void rest(struct k_work *work) {
    if (cat) {
        lv_img_set_src(cat, &bongo_cat_double_tap1_06);
    }
}

static void tap(struct k_work *work) {
    if (cat) {
        left_paw = !left_paw;
        lv_img_set_src(cat, left_paw ? &bongo_cat_tap1_03 : &bongo_cat_tap2_03);
    }
}

static K_WORK_DELAYABLE_DEFINE(create_work, create_cat);
static K_WORK_DELAYABLE_DEFINE(rest_work, rest);
static K_WORK_DEFINE(tap_work, tap);

static int bongo_listener(const zmk_event_t *eh) {
    const struct zmk_position_state_changed *ev = as_zmk_position_state_changed(eh);
    if (ev && ev->state) {
        k_work_submit_to_queue(zmk_display_work_q(), &tap_work);
        k_work_reschedule_for_queue(zmk_display_work_q(), &rest_work,
                                    K_MSEC(CONFIG_WATERFOWL_PERIPHERAL_BONGO_CAT_REST_MS));
    }
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(peripheral_bongo_cat, bongo_listener);
ZMK_SUBSCRIPTION(peripheral_bongo_cat, zmk_position_state_changed);

// nice-oled builds and loads its screen on the display queue during startup;
// add the cat on top once that's done.
static int bongo_init(void) {
    k_work_schedule_for_queue(zmk_display_work_q(), &create_work, K_MSEC(1000));
    return 0;
}

SYS_INIT(bongo_init, APPLICATION, 99);
