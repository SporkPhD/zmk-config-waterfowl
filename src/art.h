#pragma once

#include <lvgl.h>

// 1-bit bitmaps from tools/make_art.py: one string per row, '#' = lit pixel.
#define ART_DUCK_W 64
#define ART_DUCK_H 75
extern const char *const art_duck[];

#define ART_MOD_SIZE 9
extern const char *const art_mod_cmd[];
extern const char *const art_mod_opt[];
extern const char *const art_mod_ctrl[];
extern const char *const art_mod_shift[];

// Bongo Cat frames (src/bongo_cat_frames.c), 26 x 50 in landscape pixels.
LV_IMG_DECLARE(bongo_cat_double_tap1_06);
LV_IMG_DECLARE(bongo_cat_tap1_03);
LV_IMG_DECLARE(bongo_cat_tap2_03);
