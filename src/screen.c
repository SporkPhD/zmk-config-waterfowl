#include "screen.h"

#define BG lv_color_white()
#define FG lv_color_black()

// Square so the 90 degree rotation fits; only the left 64 columns are drawn on.
static lv_color_t cbuf[SCREEN_H * SCREEN_H];
static lv_color_t cbuf_tmp[SCREEN_H * SCREEN_H];

lv_obj_t *screen_create(struct screen *s) {
    lv_obj_t *root = lv_obj_create(NULL);
    s->obj = lv_obj_create(root);
    lv_obj_set_size(s->obj, SCREEN_H, SCREEN_W);
    lv_obj_align(s->obj, LV_ALIGN_TOP_LEFT, 0, 0);
    s->canvas = lv_canvas_create(s->obj);
    lv_obj_align(s->canvas, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_canvas_set_buffer(s->canvas, cbuf, SCREEN_H, SCREEN_H, LV_IMG_CF_TRUE_COLOR);
    return root;
}

void screen_clear(struct screen *s) { lv_canvas_fill_bg(s->canvas, BG, LV_OPA_COVER); }

void screen_commit(struct screen *s) {
    memcpy(cbuf_tmp, cbuf, sizeof(cbuf_tmp));
    lv_img_dsc_t img = {
        .header.cf = LV_IMG_CF_TRUE_COLOR,
        .header.w = SCREEN_H,
        .header.h = SCREEN_H,
        .data = (const uint8_t *)cbuf_tmp,
        .data_size = sizeof(cbuf_tmp),
    };
    lv_canvas_fill_bg(s->canvas, BG, LV_OPA_COVER);
    lv_canvas_transform(s->canvas, &img, 900, LV_IMG_ZOOM_NONE, -1, 0, SCREEN_H / 2, SCREEN_H / 2,
                        false);
}

void screen_text(struct screen *s, int x, int y, int w, const lv_font_t *font,
                 lv_text_align_t align, const char *text) {
    lv_draw_label_dsc_t dsc;
    lv_draw_label_dsc_init(&dsc);
    dsc.color = FG;
    dsc.font = font;
    dsc.align = align;
    lv_canvas_draw_text(s->canvas, x, y, w, &dsc, text);
}

void screen_rect(struct screen *s, int x, int y, int w, int h, bool filled) {
    lv_draw_rect_dsc_t dsc;
    lv_draw_rect_dsc_init(&dsc);
    dsc.bg_color = FG;
    dsc.bg_opa = filled ? LV_OPA_COVER : LV_OPA_TRANSP;
    dsc.border_color = FG;
    dsc.border_width = 1;
    dsc.border_opa = LV_OPA_COVER;
    lv_canvas_draw_rect(s->canvas, x, y, w, h, &dsc);
}

void screen_hline(struct screen *s, int y) {
    for (int x = 0; x < SCREEN_W; x++) {
        lv_canvas_set_px_color(s->canvas, x, y, FG);
    }
}

void screen_bitmap(struct screen *s, int x, int y, const char *const rows[], int w, int h,
                   bool invert) {
    for (int r = 0; r < h; r++) {
        for (int c = 0; c < w; c++) {
            if (rows[r][c] == '#') {
                lv_canvas_set_px_color(s->canvas, x + c, y + r, invert ? BG : FG);
            }
        }
    }
}

void screen_battery(struct screen *s, int x, int y, int w, int h, int percent) {
    screen_rect(s, x, y, w - 2, h, false);
    // The terminal nub on the right.
    screen_rect(s, x + w - 2, y + h / 3, 2, h - 2 * (h / 3), true);
    if (percent > 0) {
        int fill = (w - 6) * (percent > 100 ? 100 : percent) / 100;
        if (fill > 0) {
            screen_rect(s, x + 2, y + 2, fill, h - 4, true);
        }
    }
}

void screen_place_unrotated(lv_obj_t *img, int x, int y, int upright_w, int upright_h) {
    // Inverse of the canvas rotation: upright (x, y) -> panel (H - y - h, x).
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, SCREEN_H - y - upright_h, x);
}
