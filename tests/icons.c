#define GOOME_IMPLEMENTATION
#include "goome.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void icon_px(uint32_t *px, int i)
{
    for (int y = 0; y < 16; y++)
        for (int x = 0; x < 16; x++) {
            int dx = x - 8, dy = y - 8, v = (x * 7 + y * 13 + i * 29) & 255;
            uint32_t a = dx * dx + dy * dy < 40 + i ? 255u : (x + y + i) % 5 == 0 ? 120u : 0u;
            px[y * 16 + x] = a << 24 | (uint32_t)v << 16 | (uint32_t)(255 - v) << 8 | (uint32_t)(i * 11 & 255);
        }
}

static void skin_px(uint32_t *px)
{
    for (int y = 0; y < 12; y++)
        for (int x = 0; x < 12; x++) px[y * 12 + x] = x < 3 || y < 3 || x > 8 || y > 8 ? 0xFF7B4FD0u : 0xFFF0E8FFu;
}

static goome_bitmap icons[GOOME_ICON_COUNT], heart, skin_bm;
static int check = 1, check2 = 0, radio = 1, on = 1, sel = 1, tab = 0;
static double val = 0.4, num = 3, wx = 6, wy = 6, ww = 288, wh = 248;
static int wopen = 1;

static void dress(goome_theme *t, int on, int tint)
{
    for (int i = 0; i < GOOME_ICON_COUNT; i++) t->icon[i] = on ? &icons[i] : NULL;
    goome_skin s = {on ? &skin_bm : NULL, 3, 3, 3, 3};
    t->skin_checkbox = t->skin_radio = t->skin_toggle = t->skin_slider = t->skin_slider_fill = t->skin_scrollbar = s;
    t->skin_tab = t->skin_titlebar = t->skin_tooltip = t->skin_menu = t->skin_popup = s;
    t->icon_tint = (uint32_t)tint;
}

static void scene(goome_ui *u, int mode)
{
    static const char *const items[] = {"one", "two", "three"};
    static const char *const tabs[] = {"One", "Two"};
    goome_theme *t = goome_ui_theme(u);
    goome_ui_push_font(u, NULL);
    if (goome_ui_window(u, "icons", &wx, &wy, &ww, &wh, GOOME_WIN_MOVE | GOOME_WIN_RESIZE | GOOME_WIN_COLLAPSE | GOOME_WIN_CLOSE, &wopen)) {
        if (mode == 3) {
            goome_skin s = {&skin_bm, 3, 3, 3, 3};
            goome_ui_push_icon(u, GOOME_ICON_CHECK, &heart);
            goome_ui_push_skin(u, GOOME_SKIN_CHECKBOX, &s);
        }
        if (mode == 4) {
            t->icon[GOOME_ICON_CHECK] = &heart;
            t->skin_checkbox = (goome_skin){&skin_bm, 3, 3, 3, 3};
        }
        goome_ui_checkbox(u, "Check", &check);
        if (mode == 3) goome_ui_pop(u, 2);
        if (mode == 4) t->icon[GOOME_ICON_CHECK] = NULL, t->skin_checkbox = (goome_skin){0};
        goome_ui_checkbox(u, "Other", &check2);
        goome_ui_radio(u, "A", &radio, 0);
        goome_ui_same_line(u);
        goome_ui_radio(u, "B", &radio, 1);
        goome_ui_toggle(u, "On", &on);
        goome_ui_slider(u, "Val", &val, 0, 1, "%.2f");
        goome_ui_combo(u, "Pick", &sel, items, 3);
        goome_ui_tabs(u, "t", tabs, 2, &tab);
        goome_ui_number(u, "N", &num, 1, "%.0f");
        if (goome_ui_collapsing(u, "More", NULL)) goome_ui_label(u, "inside");
    }
    goome_ui_window_end(u);
    goome_ui_pop(u, 1);
}

int main(void)
{
    int err;
    goome *g = goome_open(&(goome_opts){.mode = GOOME_MODE_OFFSCREEN, .title = "godot", .width = 300, .height = 260, .scale = 1}, &err);
    if (!g) return 1;
    uint32_t px[256];
    for (int i = 0; i < GOOME_ICON_COUNT; i++) {
        icon_px(px, i);
        if (goome_bitmap_from_argb(px, 16, 16, &icons[i]) != GOOME_OK) return 1;
    }
    icon_px(px, 40);
    goome_bitmap_from_argb(px, 16, 16, &heart);
    skin_px(px);
    goome_bitmap_from_argb(px, 12, 12, &skin_bm);
    goome_ui *u = goome_ui_new(g);
    goome_theme *t = goome_ui_theme(u);
    t->anim_ms = 0, t->font_size = 14;
    static const char *const names[] = {"base", "dressed", "as_is", "pushed", "theme_swap", "after"};
    static const int modes[][3] = {{0, 0, 0}, {1, 1, 0}, {1, 1, 1}, {3, 0, 0}, {4, 0, 0}, {0, 0, 0}};
    for (int k = 0; k < 6; k++) {
        for (int f = 0; f < 5; f++) {
            goome_event ev;
            while (goome_poll(g, &ev, 0) > 0) goome_ui_feed(u, &ev);
            if (!goome_ui_begin(u, 0)) continue;
            dress(t, modes[k][1], modes[k][2]);
            scene(u, modes[k][0]);
            goome_ui_end(u);
            memset(&ev, 0, sizeof(ev));
            ev.type = GOOME_MOTION, ev.x = ev.y = -100000;
            goome_ui_feed(u, &ev);
        }
        goome_image fr;
        if (goome_frame(g, &fr) != GOOME_OK) return 1;
        unsigned char *rgba = malloc((size_t)fr.w * fr.h * 4);
        if (!rgba || goome_frame_copy(g, rgba, fr.w * 4, GOOME_FMT_RGBA_PREMUL, NULL) != GOOME_OK) return 1;
        uint32_t v = 2166136261u;
        for (size_t i = 0; i < (size_t)fr.w * fr.h * 4; i++) v = (v ^ rgba[i]) * 16777619u;
        free(rgba);
        printf("%s %08x\n", names[k], v);
    }
    goome_ui_free(u);
    for (int i = 0; i < GOOME_ICON_COUNT; i++) goome_bitmap_free(&icons[i]);
    goome_bitmap_free(&heart);
    goome_bitmap_free(&skin_bm);
    goome_close(g);
    return 0;
}
