#pragma once

/*
 * Drawing helpers shared by both screens.
 *
 * The OLEDs are 128x64 but mounted upright, so each screen is drawn on a
 * 64-wide, 128-tall canvas and then rotated 90 degrees onto the panel (the
 * right panel is also flipped 180 degrees in waterfowl_right.overlay, so the
 * same rotation works for both). Coordinates below are always upright ones.
 */

#include <lvgl.h>

#define SCREEN_W 64
#define SCREEN_H 128

struct screen {
    lv_obj_t *obj;
    lv_obj_t *canvas;
};

// Creates the screen object and its canvas on a new LVGL screen.
lv_obj_t *screen_create(struct screen *s);

void screen_clear(struct screen *s);
// Rotates what was drawn onto the panel. Call after drawing a frame.
void screen_commit(struct screen *s);

void screen_text(struct screen *s, int x, int y, int w, const lv_font_t *font,
                 lv_text_align_t align, const char *text);
void screen_rect(struct screen *s, int x, int y, int w, int h, bool filled);
void screen_hline(struct screen *s, int y);
// Draws a '#'/'.' bitmap; `invert` draws lit pixels dark (for highlighted boxes).
void screen_bitmap(struct screen *s, int x, int y, const char *const rows[], int w, int h,
                   bool invert);
// A battery outline w wide and h tall, filled to `percent` (negative = unknown).
void screen_battery(struct screen *s, int x, int y, int w, int h, int percent);

// Places an image that is already drawn in the panel's unrotated orientation
// (like the Bongo Cat frames) so it shows upright at upright position (x, y).
void screen_place_unrotated(lv_obj_t *img, int x, int y, int upright_w, int upright_h);
