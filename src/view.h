#ifndef GOOME_GD_VIEW_H
#define GOOME_GD_VIEW_H

#include "gd.h"
#include "goome.h"

typedef struct gd_view gd_view;

typedef struct {
    uint32_t key;
    int kind, open, seen;
    double v[4];
    char *buf;
    int cap;
} gd_slot;

typedef struct {
    int64_t id;
    int w, h;
    goome_font *font;
    goome_bitmap bm;
    gd_var keep;
} gd_asset;

typedef struct {
    GDExtensionObjectPtr obj;
    gd_view *view;
    int changed, warned, entered;
    char mask[16];
    uint32_t ids[32];
    int nids;
    goome_canvas *canvas;
    gd_asset **assets;
    int nassets, capassets;
} gd_ui;

struct gd_view {
    GDExtensionObjectPtr obj;
    gd_var me, uivar, img, part, tex, mat;
    gd_ui *ui;
    goome *g;
    goome_ui *u;
    int w, h, texw, texh, in_frame, dirty, frames;
    double scale, pixel_scale;
    int redraw_every_frame, take_keyboard, sync_clipboard, light_theme;
    int shape, wants, sticky, buttons, pressed_on_ui, inside, check_clip;
    int fixed_step, a11y, ime_active;
    goome_rect ime_at;
    double mx, my;
    char *clip;
    gd_slot *slots;
    int nslots, capslots;
};

int gd_goome_ime(goome *g, goome_rect *caret);
int gd_goome_ime_push(goome *g, int type, const char *s, int cursor);
void gd_goome_fixed_time(goome_ui *u, int on);

gd_ui *ui_make(gd_view *v);
void ui_drop(gd_ui *ui);
gd_slot *view_slot(gd_view *v, int kind, const char *id, uint32_t seed, int *fresh);

#endif
