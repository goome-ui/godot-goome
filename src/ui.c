#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "view.h"

#define UI                           \
    gd_ui *ui = inst;                \
    goome_ui *u = live(ui);          \
    if (!u) return;                  \
    ui->canvas = NULL
#define DRAW                                          \
    gd_ui *ui = inst;                                 \
    goome_ui *u = live(ui);                           \
    if (!u) return;                                   \
    if (!ui->canvas) ui->canvas = goome_ui_canvas(u); \
    goome_canvas *c = ui->canvas;                     \
    if (!c) return
#define S(i) (a->s[i])
#define I(i) ((int)gd_as_int(a->v[i]))
#define F(i) gd_as_float(a->v[i])
#define B(i) gd_as_bool(a->v[i])
#define OPT(i) (a->s[i][0] ? a->s[i] : NULL)

static goome_ui *live(gd_ui *ui)
{
    if (ui && ui->view && ui->view->in_frame && ui->view->u) return ui->view->u;
    if (ui && !ui->warned) {
        ui->warned = 1;
        gd_err("goome: the ui only works inside _goome_frame(ui) or the goome_frame signal");
    }
    return NULL;
}

static void rb(gd_var *r, int x)
{
    gd.destroy(r);
    gd_var_bool(r, x);
}

static void ri(gd_var *r, int64_t x)
{
    gd.destroy(r);
    gd_var_int(r, x);
}

static void rf(gd_var *r, double x)
{
    gd.destroy(r);
    gd_var_float(r, x);
}

static void rs(gd_var *r, const char *s)
{
    gd.destroy(r);
    gd_var_cstr(r, s);
}

static void rv(gd_var *r, int x, int y)
{
    gd.destroy(r);
    gd_var_vec2i(r, x, y);
}

static uint32_t seed(gd_ui *ui) { return ui->nids ? ui->ids[ui->nids - 1] : 0; }

static gd_slot *slot(gd_ui *ui, int kind, const char *id, int use_seed, int *fresh)
{
    if (!ui->view) return NULL;
    return view_slot(ui->view, kind, id, use_seed ? seed(ui) : 0, fresh);
}

static void layout_row(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_row(u, I(0));
}

static void layout_same_line(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_same_line(u);
}

static void layout_space(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_space(u, F(0));
}

static void layout_separator(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_separator(u);
}

static void layout_width(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_width(u, F(0));
}

static void text_label(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_label(u, "%s", S(0));
}

static void text_heading(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_heading(u, S(0));
}

static void text_wrapped(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_wrapped(u, S(0));
}

static void w_button(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    int r = goome_ui_button(u, S(0));
    ui->changed = r;
    rb(ret, r);
}

static void flag(void *inst, gd_args *a, gd_var *ret, int (*fn)(goome_ui *, const char *, int *))
{
    UI;
    int v = B(1);
    ui->changed = fn(u, S(0), &v) != 0;
    rb(ret, v);
}

static void w_checkbox(void *inst, gd_args *a, gd_var *ret) { flag(inst, a, ret, goome_ui_checkbox); }

static void w_toggle(void *inst, gd_args *a, gd_var *ret) { flag(inst, a, ret, goome_ui_toggle); }

static void w_radio(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    int v = I(1);
    ui->changed = goome_ui_radio(u, S(0), &v, I(2)) != 0;
    ri(ret, v);
}

static void w_slider(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    double v = F(1);
    ui->changed = goome_ui_slider(u, S(0), &v, F(2), F(3), OPT(4)) != 0;
    rf(ret, v);
}

static void w_slider_int(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    int v = I(1);
    ui->changed = goome_ui_slider_int(u, S(0), &v, I(2), I(3)) != 0;
    ri(ret, v);
}

static void input_with(void *inst, gd_args *a, gd_var *ret, int flags, const char *hint)
{
    UI;
    int cap = I(2) + 1;
    if (cap < 2) cap = 2;
    if (cap > 1 << 20) cap = 1 << 20;
    char *buf = malloc((size_t)cap * 4);
    if (!buf) return;
    cap *= 4;
    size_t n = strlen(S(1));
    if (n > (size_t)cap - 1) n = (size_t)cap - 1;
    memcpy(buf, S(1), n);
    buf[n] = 0;
    int r = goome_ui_input_ex(u, S(0), buf, cap, flags, hint);
    ui->changed = r != 0;
    ui->entered = r == 2;
    rs(ret, buf);
    free(buf);
}

static void w_input(void *inst, gd_args *a, gd_var *ret) { input_with(inst, a, ret, 0, NULL); }

static void w_input_ex(void *inst, gd_args *a, gd_var *ret) { input_with(inst, a, ret, I(3), OPT(4)); }

static void w_password(void *inst, gd_args *a, gd_var *ret) { input_with(inst, a, ret, GOOME_INPUT_PASSWORD | GOOME_INPUT_ENTER_RETURNS, OPT(3)); }

static void w_entered(void *inst, gd_args *a, gd_var *ret)
{
    gd_ui *ui = inst;
    rb(ret, ui->entered);
}

static void list_widget(void *inst, gd_args *a, gd_var *ret, int tabs)
{
    UI;
    char **items;
    int n = gd_strs(a->v[tabs ? 1 : 2], &items);
    int v = I(tabs ? 2 : 1);
    if (tabs) ui->changed = goome_ui_tabs(u, S(0), (const char *const *)items, n, &v) != 0;
    else ui->changed = goome_ui_combo(u, S(0), &v, (const char *const *)items, n) != 0;
    gd_strs_free(items, n);
    ri(ret, v);
}

static void w_combo(void *inst, gd_args *a, gd_var *ret) { list_widget(inst, a, ret, 0); }

static void w_tabs(void *inst, gd_args *a, gd_var *ret) { list_widget(inst, a, ret, 1); }

static void w_selectable(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    int r = goome_ui_selectable(u, S(0), B(1));
    ui->changed = r;
    rb(ret, r);
}

static int *open_slot(gd_ui *ui, int kind, gd_args *a)
{
    int fresh;
    gd_slot *s = slot(ui, kind, a->s[0], 1, &fresh);
    if (!s) return NULL;
    if (fresh) s->open = gd_as_bool(a->v[1]);
    return &s->open;
}

static void w_collapsing(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    rb(ret, goome_ui_collapsing(u, S(0), open_slot(ui, 'f', a)));
}

static void w_tree(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    rb(ret, goome_ui_tree(u, S(0), open_slot(ui, 'r', a)));
}

static void w_tree_end(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_tree_end(u);
}

static void w_progress(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_progress(u, F(0), OPT(1));
}

static void w_color(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    uint32_t c = argb_of(gd_as_color(a->v[1]));
    ui->changed = goome_ui_color(u, S(0), &c) != 0;
    gd.destroy(ret);
    gd_var_color(ret, ui->changed ? color_of(c) : gd_as_color(a->v[1]));
}

static void w_tooltip(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_tooltip(u, S(0));
}

static void w_hovered(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    rb(ret, goome_ui_hovered(u));
}

static void w_focused(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    rb(ret, goome_ui_focused(u));
}

static void w_set_focus(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_set_focus(u);
}

static void w_scroll_begin(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    rb(ret, goome_ui_scroll_begin(u, S(0), F(1)));
}

static void w_scroll_end(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_scroll_end(u);
}

static uint32_t fnv(uint32_t h, const char *s)
{
    if (!h) h = 2166136261u;
    for (; *s; s++) h = (h ^ (uint8_t)*s) * 16777619u;
    return h;
}

static void w_push_id(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_push_id(u, S(0));
    if (ui->nids < 32) {
        ui->ids[ui->nids] = fnv(seed(ui), S(0));
        ui->nids++;
    }
}

static void w_pop_id(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_pop_id(u);
    if (ui->nids > 0) ui->nids--;
}

static gd_slot *win_slot(gd_ui *ui, const char *title, const gd_var *rect)
{
    int fresh;
    gd_slot *s = slot(ui, 'w', title, 0, &fresh);
    if (s && fresh) {
        gd_rect2 r = rect ? gd_as_rect2(rect) : (gd_rect2){40, 40, 300, 0};
        s->v[0] = r.x, s->v[1] = r.y, s->v[2] = r.w, s->v[3] = r.h;
        s->open = 1;
    }
    return s;
}

static void w_window(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    gd_slot *s = win_slot(ui, S(0), a->v[1]);
    if (!s) return;
    int flags = I(2);
    rb(ret, goome_ui_window(u, S(0), &s->v[0], &s->v[1], &s->v[2], &s->v[3], flags, (flags & GOOME_WIN_CLOSE) ? &s->open : NULL));
}

static void w_window_end(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_window_end(u);
}

static void w_get_window_rect(void *inst, gd_args *a, gd_var *ret)
{
    gd_slot *s = win_slot(inst, S(0), NULL);
    if (!s) return;
    gd.destroy(ret);
    gd_var_rect2(ret, (gd_rect2){(float)s->v[0], (float)s->v[1], (float)s->v[2], (float)s->v[3]});
}

static void w_set_window_rect(void *inst, gd_args *a, gd_var *ret)
{
    gd_slot *s = win_slot(inst, S(0), NULL);
    if (!s) return;
    gd_rect2 r = gd_as_rect2(a->v[1]);
    s->v[0] = r.x, s->v[1] = r.y, s->v[2] = r.w, s->v[3] = r.h;
}

static void w_is_window_open(void *inst, gd_args *a, gd_var *ret)
{
    gd_slot *s = win_slot(inst, S(0), NULL);
    rb(ret, s && s->open);
}

static void w_set_window_open(void *inst, gd_args *a, gd_var *ret)
{
    gd_slot *s = win_slot(inst, S(0), NULL);
    if (s) s->open = B(1);
}

static void w_app_begin(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    rb(ret, goome_ui_app(u, OPT(0), I(1)));
}

static void w_app_end(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_app_end(u);
}

static void w_menubar_begin(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    rb(ret, goome_ui_menubar_begin(u));
}

static void w_menubar_end(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_menubar_end(u);
}

static void w_menu_begin(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    rb(ret, goome_ui_menu_begin(u, S(0)));
}

static void w_menu_end(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_menu_end(u);
}

static void w_menu_item(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    int r = goome_ui_menu_item(u, S(0), OPT(1), NULL);
    ui->changed = r;
    rb(ret, r);
}

static void w_menu_check(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    int v = B(1);
    ui->changed = goome_ui_menu_item(u, S(0), OPT(2), &v) != 0;
    rb(ret, v);
}

static void w_menu_separator(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_menu_separator(u);
}

static void w_context_begin(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    rb(ret, goome_ui_context_begin(u, S(0)));
}

static void w_context_end(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_context_end(u);
}

static void w_popup_open(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_popup_open(u, S(0));
}

static void w_popup_begin(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    rb(ret, goome_ui_popup_begin(u, S(0)));
}

static void w_popup_end(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_popup_end(u);
}

static void w_popup_close(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_popup_close(u);
}

static gd_slot *modal_slot(gd_ui *ui, const char *title)
{
    int fresh;
    gd_slot *s = slot(ui, 'm', title, 1, &fresh);
    if (s && fresh) s->open = 0;
    return s;
}

static void w_modal_open(void *inst, gd_args *a, gd_var *ret)
{
    gd_slot *s = modal_slot(inst, S(0));
    if (s) s->open = 1;
}

static void w_is_modal_open(void *inst, gd_args *a, gd_var *ret)
{
    gd_slot *s = modal_slot(inst, S(0));
    rb(ret, s && s->open);
}

static void w_modal_begin(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    gd_slot *s = modal_slot(ui, S(0));
    if (!s) return;
    rb(ret, goome_ui_modal_begin(u, S(0), &s->open));
}

static void w_modal_end(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_modal_end(u);
}

static void w_toast(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_toast(u, S(0), I(1));
}

static void w_statusbar(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_statusbar(u, S(0));
}

static void w_text_edit(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    int fresh;
    gd_slot *s = slot(ui, 't', S(0), 1, &fresh);
    if (!s) return;
    size_t n = strlen(S(1));
    if (!s->buf || (int)n * 4 > s->cap * 3) {
        int cap = (int)n * 4 > 65536 ? (int)n * 4 : 65536;
        char *b = realloc(s->buf, (size_t)cap);
        if (!b) return;
        if (!s->buf) b[0] = 0;
        s->buf = b, s->cap = cap;
    }
    if (strcmp(s->buf, S(1))) memcpy(s->buf, S(1), n + 1);
    ui->changed = goome_ui_text_edit(u, S(0), s->buf, s->cap, F(2), I(3)) != 0;
    rs(ret, s->buf);
}

static void w_table_begin(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    rb(ret, goome_ui_table_begin(u, S(0), I(1), F(2), I(3)));
}

static void w_table_column(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_table_column(u, S(0), F(1));
}

static void w_table_sort(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    gd_vec2i v = gd_as_vec2i(a->v[0]);
    int col = v.x, desc = v.y;
    ui->changed = goome_ui_table_sort(u, &col, &desc) != 0;
    rv(ret, col, desc);
}

static void w_table_row(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_table_row(u);
}

static void w_table_next(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_table_next(u);
}

static void w_table_end(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_table_end(u);
}

static void w_list_clip(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    int first = 0, last = 0;
    goome_ui_list_clip(u, I(0), F(1), &first, &last);
    rv(ret, first, last);
}

static void w_drag_float(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    double v = F(1);
    ui->changed = goome_ui_drag_float(u, S(0), &v, F(2), F(3), F(4), OPT(5)) != 0;
    rf(ret, v);
}

static void w_drag_int(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    int v = I(1);
    ui->changed = goome_ui_drag_int(u, S(0), &v, F(2), I(3), I(4)) != 0;
    ri(ret, v);
}

static void w_number(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    double v = F(1);
    ui->changed = goome_ui_number(u, S(0), &v, F(2), OPT(3)) != 0;
    rf(ret, v);
}

static void plot(void *inst, gd_args *a, gd_var *ret, int bars)
{
    UI;
    float *v;
    int n = gd_floats(a->v[1], &v);
    if (bars) goome_ui_plot_bars(u, S(0), v, n, F(2), F(3), F(4));
    else goome_ui_plot_lines(u, S(0), v, n, F(2), F(3), F(4));
    free(v);
}

static void w_plot_lines(void *inst, gd_args *a, gd_var *ret) { plot(inst, a, ret, 0); }

static void w_plot_bars(void *inst, gd_args *a, gd_var *ret) { plot(inst, a, ret, 1); }

static void w_split_begin(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    int fresh;
    gd_slot *s = slot(ui, 's', S(0), 1, &fresh);
    if (!s) return;
    if (fresh) s->v[0] = F(2);
    rb(ret, goome_ui_split_begin(u, S(0), B(1), &s->v[0]));
}

static void w_split_next(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_split_next(u);
}

static void w_split_end(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_split_end(u);
}

static void w_spinner(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_spinner(u, F(0));
}

static void w_link(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    int r = goome_ui_link(u, S(0));
    ui->changed = r;
    rb(ret, r);
}

static void w_badge(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_badge(u, S(0), argb_of(gd_as_color(a->v[1])));
}

static void w_changed(void *inst, gd_args *a, gd_var *ret)
{
    gd_ui *ui = inst;
    rb(ret, ui->changed);
}

static void w_wants_mouse(void *inst, gd_args *a, gd_var *ret)
{
    gd_ui *ui = inst;
    rb(ret, ui->view && ui->view->u && (goome_ui_wants(ui->view->u) & GOOME_WANT_MOUSE));
}

static void w_wants_keyboard(void *inst, gd_args *a, gd_var *ret)
{
    gd_ui *ui = inst;
    rb(ret, ui->view && ui->view->u && (goome_ui_wants(ui->view->u) & GOOME_WANT_KEYBOARD));
}

typedef struct {
    const char *name;
    size_t off;
    int kind, s;
} theme_field;

enum { TF_NUM, TF_COLOR, TF_INT, TF_FONT };

#define TC(n, s) {#n, offsetof(goome_theme, n), TF_COLOR, s}
#define TN(n, s) {#n, offsetof(goome_theme, n), TF_NUM, s}

static const theme_field fields[] = {
    TC(bg, GOOME_S_BG), TC(panel, GOOME_S_PANEL), TC(border, GOOME_S_BORDER), TC(text, GOOME_S_TEXT), TC(text_dim, GOOME_S_TEXT_DIM),
    TC(accent, GOOME_S_ACCENT), TC(accent_text, GOOME_S_ACCENT_TEXT), TC(hover, GOOME_S_HOVER), TC(active, GOOME_S_ACTIVE),
    TC(input_bg, GOOME_S_INPUT_BG), TC(shadow, GOOME_S_SHADOW),
    TN(radius, GOOME_S_RADIUS), TN(pad, GOOME_S_PAD), TN(gap, GOOME_S_GAP), TN(border_width, GOOME_S_BORDER_WIDTH), TN(font_size, GOOME_S_FONT_SIZE),
    TN(shadow_blur, GOOME_S_SHADOW_BLUR), TN(anim_ms, GOOME_S_ANIM_MS),
    TC(focus, GOOME_S_FOCUS), TC(select, GOOME_S_SELECT), TC(caret, GOOME_S_CARET), TC(link, GOOME_S_LINK), TC(badge, GOOME_S_BADGE),
    TC(toast, GOOME_S_TOAST), TC(tooltip, GOOME_S_TOOLTIP), TC(danger, GOOME_S_DANGER), TC(knob, GOOME_S_KNOB), TC(dim, GOOME_S_DIM),
    TN(check_size, GOOME_S_CHECK_SIZE), TN(check_radius, GOOME_S_CHECK_RADIUS), TN(toggle_w, GOOME_S_TOGGLE_W), TN(toggle_h, GOOME_S_TOGGLE_H),
    TN(slider_h, GOOME_S_SLIDER_H), TN(slider_track, GOOME_S_SLIDER_TRACK), TN(knob_size, GOOME_S_KNOB_SIZE), TN(scroll_w, GOOME_S_SCROLL_W),
    TN(scroll_min, GOOME_S_SCROLL_MIN), TN(item_h, GOOME_S_ITEM_H), TN(menu_item_h, GOOME_S_MENU_ITEM_H), TN(tab_h, GOOME_S_TAB_H),
    TN(win_title_h, GOOME_S_WIN_TITLE_H), TN(app_title_h, GOOME_S_APP_TITLE_H), TN(menubar_h, GOOME_S_MENUBAR_H),
    TN(statusbar_h, GOOME_S_STATUSBAR_H), TN(focus_w, GOOME_S_FOCUS_W), TN(focus_gap, GOOME_S_FOCUS_GAP), TN(sep_w, GOOME_S_SEP_W),
    TN(grip, GOOME_S_GRIP), TN(indent, GOOME_S_INDENT), TN(plot_w, GOOME_S_PLOT_W), TN(spinner, GOOME_S_SPINNER), TN(caret_w, GOOME_S_CARET_W),
    TN(win_min_w, GOOME_S_WIN_MIN_W), TN(win_min_h, GOOME_S_WIN_MIN_H), TN(pop_gap, GOOME_S_POP_GAP), TN(menu_gap, GOOME_S_MENU_GAP),
    TN(tip_pad_x, GOOME_S_TIP_PAD_X), TN(tip_pad_y, GOOME_S_TIP_PAD_Y), TN(tip_x, GOOME_S_TIP_X), TN(tip_y, GOOME_S_TIP_Y),
    TN(tip_radius, GOOME_S_TIP_RADIUS), TN(shadow_x, GOOME_S_SHADOW_X), TN(shadow_y, GOOME_S_SHADOW_Y), TN(align, GOOME_S_ALIGN),
    TN(heading_size, GOOME_S_HEADING_SIZE), TN(small_size, GOOME_S_SMALL_SIZE), TN(ease, GOOME_S_EASE), TN(hover_ms, GOOME_S_HOVER_MS),
    TN(press_ms, GOOME_S_PRESS_MS), TN(popup_ms, GOOME_S_POPUP_MS), TN(collapse_ms, GOOME_S_COLLAPSE_MS), TN(toast_fade_ms, GOOME_S_TOAST_FADE_MS),
    {"icon_tint", offsetof(goome_theme, icon_tint), TF_INT, -1},
    {"font", offsetof(goome_theme, font), TF_FONT, -1},
    {"font_heading", offsetof(goome_theme, font_heading), TF_FONT, -1},
    {"font_mono", offsetof(goome_theme, font_mono), TF_FONT, -1},
    {"font_small", offsetof(goome_theme, font_small), TF_FONT, -1},
    {NULL, 0, 0, 0},
};

static goome_theme *theme_of(gd_ui *ui) { return ui->view && ui->view->u ? goome_ui_theme(ui->view->u) : NULL; }

static const theme_field *field(const char *name)
{
    for (const theme_field *f = fields; f->name; f++)
        if (!strcmp(f->name, name)) return f;
    gd_err("goome: thats not a theme name, look at theme_set in the readme");
    return NULL;
}

static void put_field(gd_ui *ui, goome_theme *t, const theme_field *f, const gd_var *v);

static void touch(gd_ui *ui)
{
    if (ui->view) ui->view->dirty = 1;
}

static void w_theme_set(void *inst, gd_args *a, gd_var *ret)
{
    goome_theme *t = theme_of(inst);
    const theme_field *f = t ? field(S(0)) : NULL;
    if (!f) return;
    put_field(inst, t, f, a->v[1]);
    touch(inst);
}

static void w_theme_get(void *inst, gd_args *a, gd_var *ret)
{
    goome_theme *t = theme_of(inst);
    const theme_field *f = t ? field(S(0)) : NULL;
    if (!f) return;
    char *p = (char *)t + f->off;
    gd.destroy(ret);
    if (f->kind == TF_FONT) gd_var_bool(ret, *(goome_font **)p != NULL);
    else if (f->kind == TF_INT) gd_var_int(ret, *(uint32_t *)p);
    else if (f->kind == TF_COLOR) gd_var_color(ret, color_of(*(uint32_t *)p));
    else gd_var_float(ret, *(double *)p);
}

static void w_theme_dark(void *inst, gd_args *a, gd_var *ret)
{
    goome_theme *t = theme_of(inst);
    if (t) goome_ui_theme_dark(t);
    if (((gd_ui *)inst)->view) ((gd_ui *)inst)->view->dirty = 1;
}

static void w_theme_light(void *inst, gd_args *a, gd_var *ret)
{
    goome_theme *t = theme_of(inst);
    if (t) goome_ui_theme_light(t);
    if (((gd_ui *)inst)->view) ((gd_ui *)inst)->view->dirty = 1;
}

static goome_font *font_of(gd_ui *ui, const gd_var *v);

static void put_field(gd_ui *ui, goome_theme *t, const theme_field *f, const gd_var *v)
{
    char *p = (char *)t + f->off;
    if (f->kind == TF_FONT) *(goome_font **)p = font_of(ui, v);
    else if (f->kind == TF_INT) *(uint32_t *)p = (uint32_t)gd_as_int(v);
    else if (f->kind == TF_COLOR) *(uint32_t *)p = argb_of(gd_as_color(v));
    else *(double *)p = gd_as_float(v);
}

static goome_radii radii_of(const gd_var *v)
{
    if (gd_is(v, T_VEC4)) {
        gd_vec4 r = gd_as_vec4(v);
        return (goome_radii){r.x, r.y, r.z, r.w};
    }
    if (gd_is(v, T_NIL)) return goome_radius(0);
    return goome_radius(gd_as_float(v));
}

static int64_t instance_id(gd_var *v)
{
    gd_var r;
    gd_call(v, "get_instance_id", &r, 0);
    int64_t id = gd_as_int(&r);
    gd.destroy(&r);
    return id;
}

static gd_asset *asset(gd_ui *ui, int64_t id, int w, int h)
{
    for (int i = 0; i < ui->nassets; i++)
        if (ui->assets[i]->id == id && ui->assets[i]->w == w && ui->assets[i]->h == h) return ui->assets[i];
    if (ui->nassets == ui->capassets) {
        int cap = ui->capassets ? ui->capassets * 2 : 8;
        gd_asset **n = realloc(ui->assets, sizeof(gd_asset *) * (size_t)cap);
        if (!n) return NULL;
        ui->assets = n, ui->capassets = cap;
    }
    gd_asset *a = calloc(1, sizeof(*a));
    if (!a) return NULL;
    gd_var_nil(&a->keep);
    a->id = id, a->w = w, a->h = h;
    ui->assets[ui->nassets++] = a;
    return a;
}

static int int_of(gd_var *self, const char *method)
{
    gd_var r;
    gd_call(self, method, &r, 0);
    int x = (int)gd_as_int(&r);
    gd.destroy(&r);
    return x;
}

static goome_font *system_font(gd_ui *ui, const gd_var *v)
{
    char *name = gd_as_cstr(v);
    if (!name) return NULL;
    int64_t id = 0;
    for (const char *c = name; *c; c++) id = id * 31 + (unsigned char)*c;
    gd_asset *a = asset(ui, id, -2, -2);
    if (a && !a->font) {
        a->font = goome_font_system(name);
        if (!a->font) gd_err("goome: couldnt find that system font");
    }
    free(name);
    return a ? a->font : NULL;
}

static goome_font *bytes_font(gd_ui *ui, const gd_var *v)
{
    gd_packed raw;
    gd.to[T_BYTES](&raw, (GDExtensionVariantPtr)v);
    gd_var copy;
    gd.copy(&copy, v);
    int n = int_of(&copy, "size");
    gd.destroy(&copy);
    uint8_t *p = n > 0 ? gd.bytes_at(&raw, 0) : NULL;
    uint32_t h = 2166136261u;
    for (int i = 0; p && i < n; i++) h = (h ^ p[i]) * 16777619u;
    gd_asset *a = p ? asset(ui, ((int64_t)n << 32) | h, -3, -3) : NULL;
    if (a && !a->font) {
        a->font = goome_font_load_mem(p, (size_t)n, 1);
        if (!a->font) gd_err("goome: those bytes arent a ttf or otf font");
    }
    gd.dtor[T_BYTES](&raw);
    return a ? a->font : NULL;
}

static goome_font *font_of(gd_ui *ui, const gd_var *v)
{
    if (gd_is(v, T_STRING) || gd_is(v, T_SN)) return system_font(ui, v);
    if (gd_is(v, T_BYTES)) return bytes_font(ui, v);
    if (!gd_as_obj(v)) return NULL;
    gd_var f, data, name;
    gd.copy(&f, v);
    gd_asset *a = asset(ui, instance_id(&f), -1, -1);
    if (a && !a->font) {
        gd_var_sn(&name, "data");
        gd_call(&f, "get", &data, 1, &name);
        gd.destroy(&name);
        gd_packed raw;
        gd_var bytes;
        gd_convert(&data, T_BYTES, &bytes);
        gd.to[T_BYTES](&raw, &bytes);
        int n = int_of(&bytes, "size");
        uint8_t *p = n > 0 ? gd.bytes_at(&raw, 0) : NULL;
        if (p) a->font = goome_font_load_mem(p, (size_t)n, 1);
        if (!a->font) gd_err("goome: push_font got something that isnt a ttf or otf FontFile");
        gd.dtor[T_BYTES](&raw);
        gd.destroy(&bytes);
        gd.destroy(&data);
    }
    gd.destroy(&f);
    return a ? a->font : NULL;
}

static int is_class(gd_var *obj, const char *cls)
{
    gd_var n, r;
    gd_var_sn(&n, cls);
    gd_call(obj, "is_class", &r, 1, &n);
    int ok = gd_as_bool(&r);
    gd.destroy(&n);
    gd.destroy(&r);
    return ok;
}

static const goome_bitmap *bitmap_of(gd_ui *ui, const gd_var *v)
{
    gd_var loaded;
    gd_var_nil(&loaded);
    if (gd_is(v, T_STRING) || gd_is(v, T_SN)) {
        gd_var loader;
        gd_var_obj(&loader, gd.singleton(SN("ResourceLoader")));
        gd_call(&loader, "load", &loaded, 1, v);
        gd.destroy(&loader);
        if (!gd_as_obj(&loaded)) {
            gd_err("goome: couldnt load that image path, it has to be something godot can load as a Texture2D or Image");
            gd.destroy(&loaded);
            return NULL;
        }
        v = &loaded;
    }
    if (!gd_as_obj(v)) return NULL;
    gd_var img;
    gd.copy(&img, v);
    int w = int_of(&img, "get_width"), h = int_of(&img, "get_height");
    gd_asset *a = w > 0 && h > 0 ? asset(ui, instance_id(&img), w, h) : NULL;
    if (a && !a->bm.px) {
        gd_var src, copy, fmt, data;
        if (is_class(&img, "Image")) gd.copy(&src, &img);
        else gd_call(&img, "get_image", &src, 0);
        if (gd_as_obj(&src)) {
            gd_call(&src, "duplicate", &copy, 0);
            gd_var_int(&fmt, 5);
            gd_call(&copy, "convert", NULL, 1, &fmt);
            gd.destroy(&fmt);
            gd_call(&copy, "get_data", &data, 0);
            gd_packed raw;
            gd.to[T_BYTES](&raw, &data);
            uint8_t *p = gd.bytes_at(&raw, 0);
            uint32_t *argb = p ? malloc((size_t)w * h * 4) : NULL;
            for (int i = 0; argb && i < w * h; i++) argb[i] = (uint32_t)p[i * 4 + 3] << 24 | (uint32_t)p[i * 4] << 16 | (uint32_t)p[i * 4 + 1] << 8 | p[i * 4 + 2];
            if (argb) goome_bitmap_from_argb(argb, w, h, &a->bm);
            free(argb);
            gd.dtor[T_BYTES](&raw);
            gd.destroy(&data);
            gd.destroy(&copy);
        } else {
            gd_err("goome: that isnt an Image or a Texture2D");
        }
        gd.destroy(&src);
        if (a->bm.px && gd_is(&loaded, T_OBJECT)) gd.copy(&a->keep, &loaded);
    }
    gd.destroy(&img);
    gd.destroy(&loaded);
    return a && a->bm.px ? &a->bm : NULL;
}

static goome_skin *skin_of(goome_theme *t, int kind)
{
    switch (kind) {
    case GOOME_SKIN_BUTTON: return &t->skin_button;
    case GOOME_SKIN_PANEL: return &t->skin_panel;
    case GOOME_SKIN_INPUT: return &t->skin_input;
    case GOOME_SKIN_CHECKBOX: return &t->skin_checkbox;
    case GOOME_SKIN_RADIO: return &t->skin_radio;
    case GOOME_SKIN_TOGGLE: return &t->skin_toggle;
    case GOOME_SKIN_SLIDER_TRACK: return &t->skin_slider;
    case GOOME_SKIN_SLIDER_FILL: return &t->skin_slider_fill;
    case GOOME_SKIN_SCROLLBAR: return &t->skin_scrollbar;
    case GOOME_SKIN_TAB: return &t->skin_tab;
    case GOOME_SKIN_TITLEBAR: return &t->skin_titlebar;
    case GOOME_SKIN_TOOLTIP: return &t->skin_tooltip;
    case GOOME_SKIN_MENU: return &t->skin_menu;
    case GOOME_SKIN_POPUP: return &t->skin_popup;
    }
    gd_err("goome: thats not a skin kind, use one of the GoomeUI.SKIN_ constants");
    return NULL;
}

static void w_theme_set_icon(void *inst, gd_args *a, gd_var *ret)
{
    gd_ui *ui = inst;
    goome_theme *t = theme_of(ui);
    int k = I(0);
    if (k < 0 || k >= GOOME_ICON_COUNT) {
        gd_err("goome: thats not an icon, use one of the GoomeUI.ICON_ constants");
        return;
    }
    if (!t) return;
    t->icon[k] = bitmap_of(ui, a->v[1]);
    if (ui->view) ui->view->dirty = 1;
}

static void w_theme_has_icon(void *inst, gd_args *a, gd_var *ret)
{
    goome_theme *t = theme_of(inst);
    int k = I(0);
    rb(ret, t && k >= 0 && k < GOOME_ICON_COUNT && t->icon[k]);
}

static void w_theme_set_skin(void *inst, gd_args *a, gd_var *ret)
{
    gd_ui *ui = inst;
    goome_theme *t = theme_of(ui);
    goome_skin *k = t ? skin_of(t, I(0)) : NULL;
    if (!k) return;
    const goome_bitmap *bm = bitmap_of(ui, a->v[1]);
    *k = (goome_skin){bm, F(2), F(3), F(4), F(5)};
    if (ui->view) ui->view->dirty = 1;
}

typedef struct {
    const char *name;
    size_t off;
    int kind;
} cfg_field;

enum { CF_INT, CF_NUM, CF_BOOL, CF_CURSOR };

#define CI(n) {#n, offsetof(goome_ui_config, n), CF_INT}
#define CB(n) {#n, offsetof(goome_ui_config, n), CF_BOOL}
#define CC(n, k) {"cursor_" #n, offsetof(goome_ui_config, cursor) + sizeof(int) * k, CF_CURSOR}

static const cfg_field cfg_fields[] = {
    CI(tip_delay_ms), CI(dbl_click_ms), CI(dbl_click_px), CI(drag_px), CI(repeat_delay_ms), CI(repeat_ms), CI(blink_ms), CI(menu_delay_ms), CI(toast_ms),
    {"scroll_lines", offsetof(goome_ui_config, scroll_lines), CF_NUM}, CB(raise_on_click), CB(esc_closes), CB(tab_nav), CB(focus_on_mouse),
    CC(button, GOOME_CUR_BUTTON), CC(input, GOOME_CUR_INPUT), CC(link, GOOME_CUR_LINK), CC(drag_value, GOOME_CUR_DRAG_VALUE),
    CC(split_x, GOOME_CUR_SPLIT_X), CC(split_y, GOOME_CUR_SPLIT_Y), CC(grip, GOOME_CUR_GRIP), CC(column, GOOME_CUR_COLUMN),
    CC(drag_drop, GOOME_CUR_DRAG_DROP), CC(custom_x, GOOME_CUR_CUSTOM_X), CC(custom_y, GOOME_CUR_CUSTOM_Y), CC(custom_xy, GOOME_CUR_CUSTOM_XY),
    {NULL, 0, 0},
};

static goome_ui_config *cfg_of(gd_ui *ui) { return ui->view && ui->view->u ? goome_ui_cfg(ui->view->u) : NULL; }

static const cfg_field *cfg_field_of(const char *name)
{
    for (const cfg_field *f = cfg_fields; f->name; f++)
        if (!strcmp(f->name, name)) return f;
    gd_err("goome: thats not a config name, look at config_set in the readme");
    return NULL;
}

static void w_config_set(void *inst, gd_args *a, gd_var *ret)
{
    gd_ui *ui = inst;
    goome_ui_config *c = cfg_of(ui);
    if (!c) return;
    if (!strcmp(S(0), "mask")) {
        char *m = gd_as_cstr(a->v[1]);
        snprintf(ui->mask, sizeof(ui->mask), "%s", m && m[0] ? m : "*");
        free(m);
        c->mask = ui->mask;
        touch(ui);
        return;
    }
    const cfg_field *f = cfg_field_of(S(0));
    if (!f) return;
    char *p = (char *)c + f->off;
    if (f->kind == CF_NUM) *(double *)p = gd_as_float(a->v[1]);
    else if (f->kind == CF_BOOL) *(int *)p = gd_as_bool(a->v[1]);
    else *(int *)p = (int)gd_as_int(a->v[1]);
    touch(ui);
}

static void w_config_get(void *inst, gd_args *a, gd_var *ret)
{
    goome_ui_config *c = cfg_of(inst);
    if (!c) return;
    if (!strcmp(S(0), "mask")) {
        rs(ret, c->mask ? c->mask : "*");
        return;
    }
    const cfg_field *f = cfg_field_of(S(0));
    if (!f) return;
    char *p = (char *)c + f->off;
    if (f->kind == CF_NUM) rf(ret, *(double *)p);
    else if (f->kind == CF_BOOL) rb(ret, *(int *)p);
    else ri(ret, *(int *)p);
}

static void w_config_reset(void *inst, gd_args *a, gd_var *ret)
{
    goome_ui_config *c = cfg_of(inst);
    if (c) goome_ui_config_default(c);
    touch(inst);
}

static void w_bind(void *inst, gd_args *a, gd_var *ret)
{
    gd_ui *ui = inst;
    int r = ui->view && ui->view->u ? goome_ui_bind(ui->view->u, I(0), OPT(1)) : GOOME_EARG;
    rb(ret, r == GOOME_OK);
}

static void st_pos(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_pos(u, F(0), F(1));
}

static void st_size(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_size(u, F(0), F(1));
}

static void st_push_color(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_push_color(u, I(0), argb_of(gd_as_color(a->v[1])));
}

static void st_push_num(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_push_num(u, I(0), F(1));
}

static void st_push_radii(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_push_radii(u, (goome_radii){F(0), F(1), F(2), F(3)});
}

static void st_push_font(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_push_font(u, font_of(ui, a->v[0]));
}

static void st_push_skin(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    const goome_bitmap *bm = bitmap_of(ui, a->v[1]);
    goome_skin sk = {bm, F(2), F(3), F(4), F(5)};
    goome_ui_push_skin(u, I(0), bm ? &sk : NULL);
}

static void st_push_icon(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_push_icon(u, I(0), bitmap_of(ui, a->v[1]));
}

static const char *const presets[] = {"dark", "light", "midnight", "paper", "mint", "high_contrast", "rose"};

static int preset_of(const char *name)
{
    for (int i = 0; i < GOOME_PRESET_COUNT; i++)
        if (!strcmp(name, presets[i])) return i;
    return -1;
}

static void apply_changes(gd_ui *ui, goome_theme *t, const gd_var *changes)
{
    if (!changes || !gd_is(changes, T_DICT)) return;
    gd_var keys, k, v;
    gd_call((gd_var *)changes, "keys", &keys, 0);
    int n = int_of(&keys, "size");
    for (int i = 0; i < n; i++) {
        GDExtensionBool ok = 0, oob = 0;
        gd.get_indexed(&keys, i, &k, &ok, &oob);
        char *name = gd_as_cstr(&k);
        const theme_field *f = name ? field(name) : NULL;
        gd_call((gd_var *)changes, "get", &v, 1, &k);
        if (f) put_field(ui, t, f, &v);
        gd.destroy(&v);
        free(name);
        gd.destroy(&k);
    }
    gd.destroy(&keys);
}

static void st_push_theme(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_theme t = *goome_ui_theme(u);
    if (S(0)[0]) {
        int p = preset_of(S(0));
        if (p < 0) gd_err("goome: push_theme base is \"\" or a preset name like \"dark\", \"light\" or \"midnight\"");
        else goome_ui_theme_preset(&t, p);
    }
    apply_changes(ui, &t, a->v[1]);
    goome_ui_push_theme(u, &t);
}

static void st_push_preset(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_theme t = *goome_ui_theme(u);
    if (goome_ui_theme_preset(&t, I(0)) != GOOME_OK) gd_err("goome: thats not a preset, use one of the GoomeUI.PRESET_ constants");
    apply_changes(ui, &t, a->v[1]);
    goome_ui_push_theme(u, &t);
}

static void st_push_theme_make(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_theme t = *goome_ui_theme(u);
    goome_ui_theme_make(&t, argb_of(gd_as_color(a->v[0])), B(1), F(2));
    apply_changes(ui, &t, a->v[3]);
    goome_ui_push_theme(u, &t);
}

static void w_theme_make(void *inst, gd_args *a, gd_var *ret)
{
    goome_theme *t = theme_of(inst);
    if (t) goome_ui_theme_make(t, argb_of(gd_as_color(a->v[0])), B(1), F(2));
    touch(inst);
}

static void w_theme_preset(void *inst, gd_args *a, gd_var *ret)
{
    goome_theme *t = theme_of(inst);
    int ok = t && goome_ui_theme_preset(t, I(0)) == GOOME_OK;
    if (t && !ok) gd_err("goome: thats not a preset, use one of the GoomeUI.PRESET_ constants");
    touch(inst);
    rb(ret, ok);
}

static void w_scale(void *inst, gd_args *a, gd_var *ret)
{
    gd_ui *ui = inst;
    if (ui->view && ui->view->u) goome_ui_scale(ui->view->u, F(0));
    touch(ui);
}

static void st_push_style(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    gd_var keys, k, v;
    int pushed = 0;
    gd_call((gd_var *)a->v[0], "keys", &keys, 0);
    int n = int_of(&keys, "size");
    for (int i = 0; i < n; i++) {
        GDExtensionBool ok = 0, oob = 0;
        gd.get_indexed(&keys, i, &k, &ok, &oob);
        char *name = gd_as_cstr(&k);
        gd_call((gd_var *)a->v[0], "get", &v, 1, &k);
        if (name && !strcmp(name, "radii")) goome_ui_push_radii(u, radii_of(&v)), pushed++;
        else if (name && !strcmp(name, "font")) goome_ui_push_font(u, font_of(ui, &v)), pushed++;
        else if (name) {
            const theme_field *f = field(name);
            if (f && f->s < 0) gd_err("goome: that one cant go in push_style, put it in push_theme");
            else if (f && f->kind == TF_COLOR) goome_ui_push_color(u, f->s, argb_of(gd_as_color(&v))), pushed++;
            else if (f) goome_ui_push_num(u, f->s, gd_as_float(&v)), pushed++;
        }
        gd.destroy(&v);
        free(name);
        gd.destroy(&k);
    }
    gd.destroy(&keys);
    ri(ret, pushed);
}

static void st_pop(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_pop(u, I(0));
}

static void put_num(gd_var *d, const char *key, double x)
{
    gd_var v;
    gd_var_float(&v, x);
    gd_dict_put(d, key, &v);
}

static void put_flag(gd_var *d, const char *key, int x)
{
    gd_var v;
    gd_var_bool(&v, x);
    gd_dict_put(d, key, &v);
}

static void w_custom(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_item it;
    goome_ui_custom(u, S(0), F(1), F(2), I(3), &it);
    gd_var v;
    gd.destroy(ret);
    gd_dict_var(ret);
    put_num(ret, "x", it.x), put_num(ret, "y", it.y), put_num(ret, "w", it.w), put_num(ret, "h", it.h);
    put_flag(ret, "hovered", it.hovered), put_flag(ret, "pressed", it.pressed), put_flag(ret, "held", it.held), put_flag(ret, "released", it.released);
    put_flag(ret, "clicked", it.clicked), put_flag(ret, "double_clicked", it.double_clicked), put_flag(ret, "right_clicked", it.right_clicked), put_flag(ret, "focused", it.focused);
    put_num(ret, "mx", it.mx), put_num(ret, "my", it.my), put_num(ret, "dx", it.dx), put_num(ret, "dy", it.dy);
    put_num(ret, "drag_x", it.drag_x), put_num(ret, "drag_y", it.drag_y);
    gd_var_int(&v, it.key);
    gd_dict_put(ret, "key", &v);
    gd_var_rect2(&v, (gd_rect2){(float)it.x, (float)it.y, (float)it.w, (float)it.h});
    gd_dict_put(ret, "rect", &v);
}

static void w_store(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    int n = I(1);
    double *s = goome_ui_store(u, S(0), n);
    gd.destroy(ret);
    gd_doubles_var(ret, s, s ? n : 0);
}

static void w_store_set(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    double *x;
    int n = gd_doubles(a->v[1], &x);
    double *s = n > 0 ? goome_ui_store(u, S(0), n) : NULL;
    if (s) memcpy(s, x, sizeof(double) * (size_t)n);
    free(x);
}

static uint32_t col(gd_args *a, int i) { return argb_of(gd_as_color(a->v[i])); }

static gd_rect2 box(gd_args *a, int i) { return gd_as_rect2(a->v[i]); }

static gd_vec2 at(gd_args *a, int i) { return gd_as_vec2(a->v[i]); }

static void d_rect(void *inst, gd_args *a, gd_var *ret)
{
    DRAW;
    gd_rect2 r = box(a, 0);
    if (gd_is(a->v[2], T_NIL)) goome_draw_rect(c, r.x, r.y, r.w, r.h, col(a, 1));
    else goome_draw_rrect(c, r.x, r.y, r.w, r.h, radii_of(a->v[2]), col(a, 1));
}

static void d_outline(void *inst, gd_args *a, gd_var *ret)
{
    DRAW;
    gd_rect2 r = box(a, 0);
    goome_draw_rrect_line(c, r.x, r.y, r.w, r.h, radii_of(a->v[3]), F(2), col(a, 1));
}

static void d_gradient(void *inst, gd_args *a, gd_var *ret)
{
    DRAW;
    gd_rect2 r = box(a, 0);
    goome_draw_rrect_grad(c, r.x, r.y, r.w, r.h, radii_of(a->v[3]), col(a, 1), col(a, 2), B(4));
}

static void d_shadow(void *inst, gd_args *a, gd_var *ret)
{
    DRAW;
    gd_rect2 r = box(a, 0);
    goome_draw_shadow(c, r.x, r.y, r.w, r.h, F(2), F(3), col(a, 1));
}

static void d_circle(void *inst, gd_args *a, gd_var *ret)
{
    DRAW;
    gd_vec2 p = at(a, 0);
    goome_draw_circle(c, p.x, p.y, F(1), col(a, 2));
}

static void d_ring(void *inst, gd_args *a, gd_var *ret)
{
    DRAW;
    gd_vec2 p = at(a, 0);
    goome_draw_ring(c, p.x, p.y, F(1), F(3), col(a, 2));
}

static void d_arc(void *inst, gd_args *a, gd_var *ret)
{
    DRAW;
    gd_vec2 p = at(a, 0);
    goome_draw_arc(c, p.x, p.y, F(1), F(5), F(2), F(3), col(a, 4));
}

static void d_line(void *inst, gd_args *a, gd_var *ret)
{
    DRAW;
    gd_vec2 p = at(a, 0), q = at(a, 1);
    goome_draw_line(c, p.x, p.y, q.x, q.y, F(3), col(a, 2));
}

static void d_polygon(void *inst, gd_args *a, gd_var *ret)
{
    DRAW;
    double *xy;
    int n = gd_vec2s(a->v[0], &xy);
    if (n >= 3) goome_draw_poly(c, xy, n, col(a, 1));
    free(xy);
}

static double text_px(goome_ui *u, double size) { return size > 0 ? size : goome_ui_theme(u)->font_size; }

static goome_font *font_or_theme(gd_ui *ui, goome_ui *u, const gd_var *v)
{
    goome_font *f = gd_is(v, T_NIL) ? NULL : font_of(ui, v);
    return f ? f : goome_ui_theme(u)->font;
}

static void d_text(void *inst, gd_args *a, gd_var *ret)
{
    DRAW;
    gd_vec2 p = at(a, 0);
    rf(ret, goome_text(c, font_or_theme(ui, u, a->v[4]), text_px(u, F(3)), p.x, p.y, S(1), col(a, 2)));
}

static void d_text_size(void *inst, gd_args *a, gd_var *ret)
{
    gd_ui *ui = inst;
    goome_ui *u = live(ui);
    if (!u) return;
    goome_font *f = font_or_theme(ui, u, a->v[2]);
    double size = text_px(u, F(1)), h = 0, w = goome_text_measure(f, size, S(0), &h);
    if (h <= 0) h = goome_text_line_height(f, size);
    gd.destroy(ret);
    gd_var_vec2(ret, (gd_vec2){(float)w, (float)h});
}

static void d_push_clip(void *inst, gd_args *a, gd_var *ret)
{
    DRAW;
    gd_rect2 r = box(a, 0);
    goome_clip_push(c, r.x, r.y, r.w, r.h);
}

static void d_pop_clip(void *inst, gd_args *a, gd_var *ret)
{
    DRAW;
    goome_clip_pop(c);
}

static void x_ease(void *inst, gd_args *a, gd_var *ret) { rf(ret, goome_ease(I(0), F(1))); }

static void x_anim(void *inst, gd_args *a, gd_var *ret)
{
    gd_ui *ui = inst;
    goome_ui *u = live(ui);
    rf(ret, goome_ui_anim(u, S(0), F(1), F(2), I(3)));
}

static void x_anim_color(void *inst, gd_args *a, gd_var *ret)
{
    gd_ui *ui = inst;
    goome_ui *u = live(ui);
    gd.destroy(ret);
    gd_var_color(ret, color_of(goome_ui_anim_color(u, S(0), argb_of(gd_as_color(a->v[1])), F(2), I(3))));
}

static void x_animating(void *inst, gd_args *a, gd_var *ret)
{
    gd_ui *ui = inst;
    rb(ret, ui->view && goome_ui_animating(ui->view->u));
}

static void x_text_dir(void *inst, gd_args *a, gd_var *ret)
{
    goome_text_dir(I(0));
    touch(inst);
}

static void x_font_fallback(void *inst, gd_args *a, gd_var *ret)
{
    gd_ui *ui = inst;
    goome_font *f = NULL, *fb = font_of(ui, a->v[1]);
    if (!gd_is(a->v[0], T_NIL)) f = font_of(ui, a->v[0]);
    else if (ui->view && ui->view->u) f = goome_ui_theme(ui->view->u)->font;
    if (f && fb) goome_font_fallback(f, fb);
    touch(ui);
    rb(ret, f && fb);
}

static void x_a11y(void *inst, gd_args *a, gd_var *ret)
{
    gd_ui *ui = inst;
    if (!ui->view) return;
    ui->view->a11y = B(0);
    if (ui->view->u) goome_ui_a11y(ui->view->u, B(0));
    touch(ui);
}

static void x_a11y_role(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_a11y_role(u, I(0), S(1));
}

static void x_a11y_value(void *inst, gd_args *a, gd_var *ret)
{
    UI;
    goome_ui_a11y_value(u, S(0));
}

static void put_int(gd_var *d, const char *key, int64_t x)
{
    gd_var v;
    gd_var_int(&v, x);
    gd_dict_put(d, key, &v);
}

static void put_str(gd_var *d, const char *key, const char *s)
{
    gd_var v;
    gd_var_cstr(&v, s);
    gd_dict_put(d, key, &v);
}

static void x_a11y_tree(void *inst, gd_args *a, gd_var *ret)
{
    gd_ui *ui = inst;
    gd.destroy(ret);
    gd_array_var(ret);
    goome_ui *u = ui->view ? ui->view->u : NULL;
    int n = goome_a11y_tree(u, NULL, 0);
    goome_a11y_node *nodes = n > 0 ? malloc(sizeof(*nodes) * (size_t)n) : NULL;
    if (!nodes) return;
    n = goome_a11y_tree(u, nodes, n);
    for (int i = 0; i < n; i++) {
        goome_a11y_node *k = &nodes[i];
        gd_var d, r;
        gd_dict_var(&d);
        put_int(&d, "role", k->role), put_int(&d, "parent", k->parent), put_int(&d, "states", k->states);
        gd_var_rect2(&r, (gd_rect2){(float)k->x, (float)k->y, (float)k->w, (float)k->h});
        gd_dict_put(&d, "rect", &r);
        put_str(&d, "label", k->label), put_str(&d, "value", k->value);
        put_int(&d, "id", k->id);
        put_num(&d, "num", k->num), put_num(&d, "min", k->min), put_num(&d, "max", k->max);
        put_int(&d, "caret", k->caret);
        gd_array_push(ret, &d);
    }
    free(nodes);
}

#define M0(n, f, r) {n, f, r, NULL, {{0}}, 0, 0, 0, {{{0}}}}
#define M(n, f, r, ...) {n, f, r, NULL, {__VA_ARGS__}, 0, 0, 0, {{{0}}}}
#define STR(n) {n, T_STRING, NULL, NULL}
#define STRD(n, d) {n, T_STRING, d, NULL}
#define INT(n) {n, T_INT, NULL, NULL}
#define INTD(n, d) {n, T_INT, d, NULL}
#define FLT(n) {n, T_FLOAT, NULL, NULL}
#define FLTD(n, d) {n, T_FLOAT, d, NULL}
#define BOOL(n) {n, T_BOOL, NULL, NULL}
#define BOOLD(n, d) {n, T_BOOL, d, NULL}
#define ARG(n, t) {n, t, NULL, NULL}
#define ARGD(n, t, d) {n, t, d, NULL}

static gd_method methods[] = {
    M("row", layout_row, T_VOID, INT("columns")),
    M0("same_line", layout_same_line, T_VOID),
    M("space", layout_space, T_VOID, FLT("pixels")),
    M0("separator", layout_separator, T_VOID),
    M("width", layout_width, T_VOID, FLT("width")),
    M("label", text_label, T_VOID, STR("text")),
    M("heading", text_heading, T_VOID, STR("text")),
    M("wrapped", text_wrapped, T_VOID, STR("text")),
    M("button", w_button, T_BOOL, STR("label")),
    M("checkbox", w_checkbox, T_BOOL, STR("label"), BOOL("value")),
    M("toggle", w_toggle, T_BOOL, STR("label"), BOOL("value")),
    M("radio", w_radio, T_INT, STR("label"), INT("value"), INT("option")),
    M("slider", w_slider, T_FLOAT, STR("label"), FLT("value"), FLTD("min", "0"), FLTD("max", "1"), STRD("format", "")),
    M("slider_int", w_slider_int, T_INT, STR("label"), INT("value"), INTD("min", "0"), INTD("max", "100")),
    M("input", w_input, T_STRING, STR("label"), STR("text"), INTD("max_length", "256")),
    M("input_ex", w_input_ex, T_STRING, STR("label"), STR("text"), INTD("max_length", "256"), INTD("flags", "0"), STRD("hint", "")),
    M("password", w_password, T_STRING, STR("label"), STR("text"), INTD("max_length", "256"), STRD("hint", "")),
    M0("entered", w_entered, T_BOOL),
    M("combo", w_combo, T_INT, STR("label"), INT("selected"), ARG("items", T_STRINGS)),
    M("tabs", w_tabs, T_INT, STR("id"), ARG("names", T_STRINGS), INT("selected")),
    M("selectable", w_selectable, T_BOOL, STR("label"), BOOLD("selected", "false")),
    M("collapsing", w_collapsing, T_BOOL, STR("label"), BOOLD("open", "false")),
    M("tree", w_tree, T_BOOL, STR("label"), BOOLD("open", "false")),
    M0("tree_end", w_tree_end, T_VOID),
    M("progress", w_progress, T_VOID, FLT("fraction"), STRD("text", "")),
    M("color", w_color, T_COLOR, STR("label"), ARG("color", T_COLOR)),
    M("tooltip", w_tooltip, T_VOID, STR("text")),
    M0("hovered", w_hovered, T_BOOL),
    M0("focused", w_focused, T_BOOL),
    M0("set_focus", w_set_focus, T_VOID),
    M("scroll_begin", w_scroll_begin, T_BOOL, STR("id"), FLTD("height", "0")),
    M0("scroll_end", w_scroll_end, T_VOID),
    M("push_id", w_push_id, T_VOID, STR("id")),
    M0("pop_id", w_pop_id, T_VOID),
    M("window", w_window, T_BOOL, STR("title"), ARGD("rect", T_RECT2, "40,40,300,0"), INTD("flags", "5")),
    M0("window_end", w_window_end, T_VOID),
    M("get_window_rect", w_get_window_rect, T_RECT2, STR("title")),
    M("set_window_rect", w_set_window_rect, T_VOID, STR("title"), ARG("rect", T_RECT2)),
    M("is_window_open", w_is_window_open, T_BOOL, STR("title")),
    M("set_window_open", w_set_window_open, T_VOID, STR("title"), BOOL("open")),
    M("app_begin", w_app_begin, T_BOOL, STRD("title", ""), INTD("flags", "0")),
    M0("app_end", w_app_end, T_VOID),
    M0("menubar_begin", w_menubar_begin, T_BOOL),
    M0("menubar_end", w_menubar_end, T_VOID),
    M("menu_begin", w_menu_begin, T_BOOL, STR("label")),
    M0("menu_end", w_menu_end, T_VOID),
    M("menu_item", w_menu_item, T_BOOL, STR("label"), STRD("shortcut", "")),
    M("menu_check", w_menu_check, T_BOOL, STR("label"), BOOL("checked"), STRD("shortcut", "")),
    M0("menu_separator", w_menu_separator, T_VOID),
    M("context_begin", w_context_begin, T_BOOL, STR("id")),
    M0("context_end", w_context_end, T_VOID),
    M("popup_open", w_popup_open, T_VOID, STR("id")),
    M("popup_begin", w_popup_begin, T_BOOL, STR("id")),
    M0("popup_end", w_popup_end, T_VOID),
    M0("popup_close", w_popup_close, T_VOID),
    M("modal_open", w_modal_open, T_VOID, STR("title")),
    M("is_modal_open", w_is_modal_open, T_BOOL, STR("title")),
    M("modal_begin", w_modal_begin, T_BOOL, STR("title")),
    M0("modal_end", w_modal_end, T_VOID),
    M("toast", w_toast, T_VOID, STR("text"), INTD("ms", "2500")),
    M("statusbar", w_statusbar, T_VOID, STR("text")),
    M("text_edit", w_text_edit, T_STRING, STR("id"), STR("text"), FLTD("height", "0"), INTD("flags", "0")),
    M("table_begin", w_table_begin, T_BOOL, STR("id"), INT("columns"), FLTD("height", "0"), INTD("flags", "0")),
    M("table_column", w_table_column, T_VOID, STR("name"), FLTD("width", "0")),
    M("table_sort", w_table_sort, T_VEC2I, ARG("sort", T_VEC2I)),
    M0("table_row", w_table_row, T_VOID),
    M0("table_next", w_table_next, T_VOID),
    M0("table_end", w_table_end, T_VOID),
    M("list_clip", w_list_clip, T_VEC2I, INT("count"), FLTD("row_height", "0")),
    M("drag_float", w_drag_float, T_FLOAT, STR("label"), FLT("value"), FLTD("speed", "0.01"), FLTD("min", "0"), FLTD("max", "0"), STRD("format", "")),
    M("drag_int", w_drag_int, T_INT, STR("label"), INT("value"), FLTD("speed", "0.1"), INTD("min", "0"), INTD("max", "0")),
    M("number", w_number, T_FLOAT, STR("label"), FLT("value"), FLTD("step", "1"), STRD("format", "")),
    M("plot_lines", w_plot_lines, T_VOID, STR("label"), ARG("values", T_FLOATS), FLTD("min", "0"), FLTD("max", "0"), FLTD("height", "60")),
    M("plot_bars", w_plot_bars, T_VOID, STR("label"), ARG("values", T_FLOATS), FLTD("min", "0"), FLTD("max", "0"), FLTD("height", "60")),
    M("split_begin", w_split_begin, T_BOOL, STR("id"), BOOLD("vertical", "false"), FLTD("fraction", "0.5")),
    M0("split_next", w_split_next, T_VOID),
    M0("split_end", w_split_end, T_VOID),
    M("spinner", w_spinner, T_VOID, FLTD("size", "0")),
    M("link", w_link, T_BOOL, STR("text")),
    M("badge", w_badge, T_VOID, STR("text"), ARG("color", T_COLOR)),
    M0("changed", w_changed, T_BOOL),
    M0("wants_mouse", w_wants_mouse, T_BOOL),
    M0("wants_keyboard", w_wants_keyboard, T_BOOL),
    M("theme_set", w_theme_set, T_VOID, STR("name"), ARG("value", T_NIL)),
    M("theme_get", w_theme_get, T_NIL, STR("name")),
    M0("theme_dark", w_theme_dark, T_VOID),
    M0("theme_light", w_theme_light, T_VOID),
    M("theme_make", w_theme_make, T_VOID, ARG("accent", T_COLOR), BOOLD("dark", "true"), FLTD("radius", "8")),
    M("theme_preset", w_theme_preset, T_BOOL, INT("preset")),
    M("scale", w_scale, T_VOID, FLT("by")),
    M("config_set", w_config_set, T_VOID, STR("name"), ARG("value", T_NIL)),
    M("config_get", w_config_get, T_NIL, STR("name")),
    M0("config_reset", w_config_reset, T_VOID),
    M("bind", w_bind, T_BOOL, INT("action"), STRD("keys", "")),
    M("theme_set_icon", w_theme_set_icon, T_VOID, INT("icon"), ARGD("image", T_NIL, "")),
    M("theme_has_icon", w_theme_has_icon, T_BOOL, INT("icon")),
    M("theme_set_skin", w_theme_set_skin, T_VOID, INT("kind"), ARGD("image", T_NIL, ""), FLTD("left", "0"), FLTD("top", "0"), FLTD("right", "0"), FLTD("bottom", "0")),
    M("pos", st_pos, T_VOID, FLT("x"), FLT("y")),
    M("size", st_size, T_VOID, FLT("width"), FLT("height")),
    M("push_color", st_push_color, T_VOID, INT("what"), ARG("color", T_COLOR)),
    M("push_num", st_push_num, T_VOID, INT("what"), FLT("value")),
    M("push_radii", st_push_radii, T_VOID, FLT("top_left"), FLT("top_right"), FLT("bottom_right"), FLT("bottom_left")),
    M("push_font", st_push_font, T_VOID, ARGD("font", T_NIL, "")),
    M("push_skin", st_push_skin, T_VOID, INT("kind"), ARGD("image", T_NIL, ""), FLTD("left", "0"), FLTD("top", "0"), FLTD("right", "0"), FLTD("bottom", "0")),
    M("push_icon", st_push_icon, T_VOID, INT("icon"), ARGD("image", T_NIL, "")),
    M("push_theme", st_push_theme, T_VOID, STRD("base", ""), ARGD("changes", T_DICT, "")),
    M("push_preset", st_push_preset, T_VOID, INT("preset"), ARGD("changes", T_DICT, "")),
    M("push_theme_make", st_push_theme_make, T_VOID, ARG("accent", T_COLOR), BOOLD("dark", "true"), FLTD("radius", "8"), ARGD("changes", T_DICT, "")),
    M("push_style", st_push_style, T_INT, ARG("style", T_DICT)),
    M("pop", st_pop, T_VOID, INTD("count", "1")),
    M("custom", w_custom, T_DICT, STR("id"), FLTD("width", "0"), FLTD("height", "0"), INTD("flags", "0")),
    M("store", w_store, T_DOUBLES, STR("id"), INT("count")),
    M("store_set", w_store_set, T_VOID, STR("id"), ARG("values", T_DOUBLES)),
    M("draw_rect", d_rect, T_VOID, ARG("rect", T_RECT2), ARG("color", T_COLOR), ARGD("radius", T_NIL, "")),
    M("draw_outline", d_outline, T_VOID, ARG("rect", T_RECT2), ARG("color", T_COLOR), FLTD("width", "1"), ARGD("radius", T_NIL, "")),
    M("draw_gradient", d_gradient, T_VOID, ARG("rect", T_RECT2), ARG("from", T_COLOR), ARG("to", T_COLOR), ARGD("radius", T_NIL, ""), BOOLD("sideways", "false")),
    M("draw_shadow", d_shadow, T_VOID, ARG("rect", T_RECT2), ARG("color", T_COLOR), FLTD("radius", "0"), FLTD("blur", "8")),
    M("draw_circle", d_circle, T_VOID, ARG("center", T_VEC2), FLT("radius"), ARG("color", T_COLOR)),
    M("draw_ring", d_ring, T_VOID, ARG("center", T_VEC2), FLT("radius"), ARG("color", T_COLOR), FLTD("width", "1")),
    M("draw_arc", d_arc, T_VOID, ARG("center", T_VEC2), FLT("radius"), FLT("from_angle"), FLT("to_angle"), ARG("color", T_COLOR), FLTD("width", "1")),
    M("draw_line", d_line, T_VOID, ARG("from", T_VEC2), ARG("to", T_VEC2), ARG("color", T_COLOR), FLTD("width", "1")),
    M("draw_polygon", d_polygon, T_VOID, ARG("points", T_VEC2S), ARG("color", T_COLOR)),
    M("draw_text", d_text, T_FLOAT, ARG("position", T_VEC2), STR("text"), ARG("color", T_COLOR), FLTD("size", "0"), ARGD("font", T_NIL, "")),
    M("text_size", d_text_size, T_VEC2, STR("text"), FLTD("size", "0"), ARGD("font", T_NIL, "")),
    M("ease", x_ease, T_FLOAT, INT("ease"), FLT("t")),
    M("anim", x_anim, T_FLOAT, STR("id"), FLT("target"), FLTD("ms", "200"), INTD("ease", "2")),
    M("anim_color", x_anim_color, T_COLOR, STR("id"), ARG("target", T_COLOR), FLTD("ms", "200"), INTD("ease", "2")),
    M0("animating", x_animating, T_BOOL),
    M("text_dir", x_text_dir, T_VOID, INT("dir")),
    M("font_fallback", x_font_fallback, T_BOOL, ARGD("font", T_NIL, ""), ARGD("fallback", T_NIL, "")),
    M("a11y", x_a11y, T_VOID, BOOL("on")),
    M("a11y_role", x_a11y_role, T_VOID, INT("role"), STRD("label", "")),
    M("a11y_value", x_a11y_value, T_VOID, STR("value")),
    M0("a11y_tree", x_a11y_tree, T_ARRAY),
    M("push_clip", d_push_clip, T_VOID, ARG("rect", T_RECT2)),
    M0("pop_clip", d_pop_clip, T_VOID),
    {NULL},
};

static const gd_const consts[] = {
    {GOOME_WIN_MOVE, "WIN_MOVE", "WindowFlags"},
    {GOOME_WIN_RESIZE, "WIN_RESIZE", "WindowFlags"},
    {GOOME_WIN_COLLAPSE, "WIN_COLLAPSE", "WindowFlags"},
    {GOOME_WIN_NO_TITLE, "WIN_NO_TITLE", "WindowFlags"},
    {GOOME_WIN_CLOSE, "WIN_CLOSE", "WindowFlags"},
    {GOOME_APP_TITLEBAR, "APP_TITLEBAR", "AppFlags"},
    {GOOME_APP_MENUBAR, "APP_MENUBAR", "AppFlags"},
    {GOOME_APP_STATUSBAR, "APP_STATUSBAR", "AppFlags"},
    {GOOME_APP_NO_PAD, "APP_NO_PAD", "AppFlags"},
    {GOOME_EDIT_READONLY, "EDIT_READONLY", "EditFlags"},
    {GOOME_EDIT_LINE_NUMBERS, "EDIT_LINE_NUMBERS", "EditFlags"},
    {GOOME_EDIT_WRAP, "EDIT_WRAP", "EditFlags"},
    {GOOME_EDIT_MONO, "EDIT_MONO", "EditFlags"},
    {GOOME_TABLE_SORTABLE, "TABLE_SORTABLE", "TableFlags"},
    {GOOME_TABLE_RESIZABLE, "TABLE_RESIZABLE", "TableFlags"},
    {GOOME_TABLE_STRIPES, "TABLE_STRIPES", "TableFlags"},
    {GOOME_TABLE_BORDERS, "TABLE_BORDERS", "TableFlags"},
    {GOOME_S_BG, "S_BG", "Style"},
    {GOOME_S_PANEL, "S_PANEL", "Style"},
    {GOOME_S_BORDER, "S_BORDER", "Style"},
    {GOOME_S_TEXT, "S_TEXT", "Style"},
    {GOOME_S_TEXT_DIM, "S_TEXT_DIM", "Style"},
    {GOOME_S_ACCENT, "S_ACCENT", "Style"},
    {GOOME_S_ACCENT_TEXT, "S_ACCENT_TEXT", "Style"},
    {GOOME_S_HOVER, "S_HOVER", "Style"},
    {GOOME_S_ACTIVE, "S_ACTIVE", "Style"},
    {GOOME_S_INPUT_BG, "S_INPUT_BG", "Style"},
    {GOOME_S_SHADOW, "S_SHADOW", "Style"},
    {GOOME_S_RADIUS, "S_RADIUS", "Style"},
    {GOOME_S_PAD, "S_PAD", "Style"},
    {GOOME_S_GAP, "S_GAP", "Style"},
    {GOOME_S_BORDER_WIDTH, "S_BORDER_WIDTH", "Style"},
    {GOOME_S_FONT_SIZE, "S_FONT_SIZE", "Style"},
    {GOOME_S_SHADOW_BLUR, "S_SHADOW_BLUR", "Style"},
    {GOOME_S_ANIM_MS, "S_ANIM_MS", "Style"},
    {GOOME_S_FOCUS, "S_FOCUS", "Style"},
    {GOOME_S_SELECT, "S_SELECT", "Style"},
    {GOOME_S_CARET, "S_CARET", "Style"},
    {GOOME_S_LINK, "S_LINK", "Style"},
    {GOOME_S_BADGE, "S_BADGE", "Style"},
    {GOOME_S_TOAST, "S_TOAST", "Style"},
    {GOOME_S_TOOLTIP, "S_TOOLTIP", "Style"},
    {GOOME_S_DANGER, "S_DANGER", "Style"},
    {GOOME_S_KNOB, "S_KNOB", "Style"},
    {GOOME_S_DIM, "S_DIM", "Style"},
    {GOOME_S_CHECK_SIZE, "S_CHECK_SIZE", "Style"},
    {GOOME_S_CHECK_RADIUS, "S_CHECK_RADIUS", "Style"},
    {GOOME_S_TOGGLE_W, "S_TOGGLE_W", "Style"},
    {GOOME_S_TOGGLE_H, "S_TOGGLE_H", "Style"},
    {GOOME_S_SLIDER_H, "S_SLIDER_H", "Style"},
    {GOOME_S_SLIDER_TRACK, "S_SLIDER_TRACK", "Style"},
    {GOOME_S_KNOB_SIZE, "S_KNOB_SIZE", "Style"},
    {GOOME_S_SCROLL_W, "S_SCROLL_W", "Style"},
    {GOOME_S_SCROLL_MIN, "S_SCROLL_MIN", "Style"},
    {GOOME_S_ITEM_H, "S_ITEM_H", "Style"},
    {GOOME_S_MENU_ITEM_H, "S_MENU_ITEM_H", "Style"},
    {GOOME_S_TAB_H, "S_TAB_H", "Style"},
    {GOOME_S_WIN_TITLE_H, "S_WIN_TITLE_H", "Style"},
    {GOOME_S_APP_TITLE_H, "S_APP_TITLE_H", "Style"},
    {GOOME_S_MENUBAR_H, "S_MENUBAR_H", "Style"},
    {GOOME_S_STATUSBAR_H, "S_STATUSBAR_H", "Style"},
    {GOOME_S_FOCUS_W, "S_FOCUS_W", "Style"},
    {GOOME_S_FOCUS_GAP, "S_FOCUS_GAP", "Style"},
    {GOOME_S_SEP_W, "S_SEP_W", "Style"},
    {GOOME_S_GRIP, "S_GRIP", "Style"},
    {GOOME_S_INDENT, "S_INDENT", "Style"},
    {GOOME_S_PLOT_W, "S_PLOT_W", "Style"},
    {GOOME_S_SPINNER, "S_SPINNER", "Style"},
    {GOOME_S_CARET_W, "S_CARET_W", "Style"},
    {GOOME_S_WIN_MIN_W, "S_WIN_MIN_W", "Style"},
    {GOOME_S_WIN_MIN_H, "S_WIN_MIN_H", "Style"},
    {GOOME_S_POP_GAP, "S_POP_GAP", "Style"},
    {GOOME_S_MENU_GAP, "S_MENU_GAP", "Style"},
    {GOOME_S_TIP_PAD_X, "S_TIP_PAD_X", "Style"},
    {GOOME_S_TIP_PAD_Y, "S_TIP_PAD_Y", "Style"},
    {GOOME_S_TIP_X, "S_TIP_X", "Style"},
    {GOOME_S_TIP_Y, "S_TIP_Y", "Style"},
    {GOOME_S_TIP_RADIUS, "S_TIP_RADIUS", "Style"},
    {GOOME_S_SHADOW_X, "S_SHADOW_X", "Style"},
    {GOOME_S_SHADOW_Y, "S_SHADOW_Y", "Style"},
    {GOOME_S_ALIGN, "S_ALIGN", "Style"},
    {GOOME_S_HEADING_SIZE, "S_HEADING_SIZE", "Style"},
    {GOOME_S_SMALL_SIZE, "S_SMALL_SIZE", "Style"},
    {GOOME_S_EASE, "S_EASE", "Style"},
    {GOOME_S_HOVER_MS, "S_HOVER_MS", "Style"},
    {GOOME_S_PRESS_MS, "S_PRESS_MS", "Style"},
    {GOOME_S_POPUP_MS, "S_POPUP_MS", "Style"},
    {GOOME_S_COLLAPSE_MS, "S_COLLAPSE_MS", "Style"},
    {GOOME_S_TOAST_FADE_MS, "S_TOAST_FADE_MS", "Style"},
    {GOOME_ALIGN_AUTO, "ALIGN_AUTO", "Align"},
    {GOOME_ALIGN_LEFT, "ALIGN_LEFT", "Align"},
    {GOOME_ALIGN_CENTER, "ALIGN_CENTER", "Align"},
    {GOOME_ALIGN_RIGHT, "ALIGN_RIGHT", "Align"},
    {GOOME_EASE_SMOOTH, "EASE_SMOOTH", "Ease"},
    {GOOME_EASE_LINEAR, "EASE_LINEAR", "Ease"},
    {GOOME_EASE_OUT, "EASE_OUT", "Ease"},
    {GOOME_EASE_IN_OUT, "EASE_IN_OUT", "Ease"},
    {GOOME_EASE_BACK, "EASE_BACK", "Ease"},
    {GOOME_EASE_CUSTOM, "EASE_CUSTOM", "Ease"},
    {GOOME_EASE_IN, "EASE_IN", "Ease"},
    {GOOME_EASE_SPRING, "EASE_SPRING", "Ease"},
    {GOOME_EASE_BOUNCE, "EASE_BOUNCE", "Ease"},
    {GOOME_DIR_AUTO, "DIR_AUTO", "TextDir"},
    {GOOME_DIR_LTR, "DIR_LTR", "TextDir"},
    {GOOME_DIR_RTL, "DIR_RTL", "TextDir"},
    {GOOME_ROLE_NONE, "ROLE_NONE", "Role"},
    {GOOME_ROLE_WINDOW, "ROLE_WINDOW", "Role"},
    {GOOME_ROLE_BUTTON, "ROLE_BUTTON", "Role"},
    {GOOME_ROLE_CHECKBOX, "ROLE_CHECKBOX", "Role"},
    {GOOME_ROLE_RADIO, "ROLE_RADIO", "Role"},
    {GOOME_ROLE_SLIDER, "ROLE_SLIDER", "Role"},
    {GOOME_ROLE_TEXT, "ROLE_TEXT", "Role"},
    {GOOME_ROLE_INPUT, "ROLE_INPUT", "Role"},
    {GOOME_ROLE_COMBO, "ROLE_COMBO", "Role"},
    {GOOME_ROLE_LIST, "ROLE_LIST", "Role"},
    {GOOME_ROLE_ITEM, "ROLE_ITEM", "Role"},
    {GOOME_ROLE_TAB, "ROLE_TAB", "Role"},
    {GOOME_ROLE_MENU, "ROLE_MENU", "Role"},
    {GOOME_ROLE_MENUITEM, "ROLE_MENUITEM", "Role"},
    {GOOME_ROLE_TABLE, "ROLE_TABLE", "Role"},
    {GOOME_ROLE_TREE, "ROLE_TREE", "Role"},
    {GOOME_ROLE_PROGRESS, "ROLE_PROGRESS", "Role"},
    {GOOME_ROLE_IMAGE, "ROLE_IMAGE", "Role"},
    {GOOME_ROLE_GROUP, "ROLE_GROUP", "Role"},
    {GOOME_ROLE_DIALOG, "ROLE_DIALOG", "Role"},
    {GOOME_A11Y_FOCUSED, "A11Y_FOCUSED", "A11yState"},
    {GOOME_A11Y_CHECKED, "A11Y_CHECKED", "A11yState"},
    {GOOME_A11Y_DISABLED, "A11Y_DISABLED", "A11yState"},
    {GOOME_A11Y_EXPANDED, "A11Y_EXPANDED", "A11yState"},
    {GOOME_A11Y_SELECTED, "A11Y_SELECTED", "A11yState"},
    {GOOME_PRESET_DARK, "PRESET_DARK", "Preset"},
    {GOOME_PRESET_LIGHT, "PRESET_LIGHT", "Preset"},
    {GOOME_PRESET_MIDNIGHT, "PRESET_MIDNIGHT", "Preset"},
    {GOOME_PRESET_PAPER, "PRESET_PAPER", "Preset"},
    {GOOME_PRESET_MINT, "PRESET_MINT", "Preset"},
    {GOOME_PRESET_HIGH_CONTRAST, "PRESET_HIGH_CONTRAST", "Preset"},
    {GOOME_PRESET_ROSE, "PRESET_ROSE", "Preset"},
    {GOOME_INPUT_PASSWORD, "INPUT_PASSWORD", "InputFlags"},
    {GOOME_INPUT_READONLY, "INPUT_READONLY", "InputFlags"},
    {GOOME_INPUT_NUMBER, "INPUT_NUMBER", "InputFlags"},
    {GOOME_INPUT_NO_SELECT_ON_FOCUS, "INPUT_NO_SELECT_ON_FOCUS", "InputFlags"},
    {GOOME_INPUT_ENTER_RETURNS, "INPUT_ENTER_RETURNS", "InputFlags"},
    {GOOME_ACT_COPY, "ACT_COPY", "Action"},
    {GOOME_ACT_CUT, "ACT_CUT", "Action"},
    {GOOME_ACT_PASTE, "ACT_PASTE", "Action"},
    {GOOME_ACT_UNDO, "ACT_UNDO", "Action"},
    {GOOME_ACT_REDO, "ACT_REDO", "Action"},
    {GOOME_ACT_SELECT_ALL, "ACT_SELECT_ALL", "Action"},
    {GOOME_ACT_WORD_LEFT, "ACT_WORD_LEFT", "Action"},
    {GOOME_ACT_WORD_RIGHT, "ACT_WORD_RIGHT", "Action"},
    {GOOME_ACT_DEL_WORD_LEFT, "ACT_DEL_WORD_LEFT", "Action"},
    {GOOME_ACT_DEL_WORD_RIGHT, "ACT_DEL_WORD_RIGHT", "Action"},
    {GOOME_ACT_DOC_START, "ACT_DOC_START", "Action"},
    {GOOME_ACT_DOC_END, "ACT_DOC_END", "Action"},
    {GOOME_CURSOR_ARROW, "CURSOR_ARROW", "Cursor"},
    {GOOME_CURSOR_IBEAM, "CURSOR_IBEAM", "Cursor"},
    {GOOME_CURSOR_HAND, "CURSOR_HAND", "Cursor"},
    {GOOME_CURSOR_MOVE, "CURSOR_MOVE", "Cursor"},
    {GOOME_CURSOR_RESIZE_EW, "CURSOR_RESIZE_EW", "Cursor"},
    {GOOME_CURSOR_RESIZE_NS, "CURSOR_RESIZE_NS", "Cursor"},
    {GOOME_CURSOR_RESIZE_NWSE, "CURSOR_RESIZE_NWSE", "Cursor"},
    {GOOME_CURSOR_RESIZE_NESW, "CURSOR_RESIZE_NESW", "Cursor"},
    {GOOME_CURSOR_WAIT, "CURSOR_WAIT", "Cursor"},
    {GOOME_CURSOR_NO, "CURSOR_NO", "Cursor"},
    {GOOME_CURSOR_HIDDEN, "CURSOR_HIDDEN", "Cursor"},
    {GOOME_CUR_BUTTON, "CUR_BUTTON", "CursorFor"},
    {GOOME_CUR_INPUT, "CUR_INPUT", "CursorFor"},
    {GOOME_CUR_LINK, "CUR_LINK", "CursorFor"},
    {GOOME_CUR_DRAG_VALUE, "CUR_DRAG_VALUE", "CursorFor"},
    {GOOME_CUR_SPLIT_X, "CUR_SPLIT_X", "CursorFor"},
    {GOOME_CUR_SPLIT_Y, "CUR_SPLIT_Y", "CursorFor"},
    {GOOME_CUR_GRIP, "CUR_GRIP", "CursorFor"},
    {GOOME_CUR_COLUMN, "CUR_COLUMN", "CursorFor"},
    {GOOME_CUR_DRAG_DROP, "CUR_DRAG_DROP", "CursorFor"},
    {GOOME_CUR_CUSTOM_X, "CUR_CUSTOM_X", "CursorFor"},
    {GOOME_CUR_CUSTOM_Y, "CUR_CUSTOM_Y", "CursorFor"},
    {GOOME_CUR_CUSTOM_XY, "CUR_CUSTOM_XY", "CursorFor"},
    {GOOME_SKIN_BUTTON, "SKIN_BUTTON", "Skin"},
    {GOOME_SKIN_PANEL, "SKIN_PANEL", "Skin"},
    {GOOME_SKIN_INPUT, "SKIN_INPUT", "Skin"},
    {GOOME_SKIN_CHECKBOX, "SKIN_CHECKBOX", "Skin"},
    {GOOME_SKIN_RADIO, "SKIN_RADIO", "Skin"},
    {GOOME_SKIN_TOGGLE, "SKIN_TOGGLE", "Skin"},
    {GOOME_SKIN_SLIDER_TRACK, "SKIN_SLIDER_TRACK", "Skin"},
    {GOOME_SKIN_SLIDER_FILL, "SKIN_SLIDER_FILL", "Skin"},
    {GOOME_SKIN_SCROLLBAR, "SKIN_SCROLLBAR", "Skin"},
    {GOOME_SKIN_TAB, "SKIN_TAB", "Skin"},
    {GOOME_SKIN_TITLEBAR, "SKIN_TITLEBAR", "Skin"},
    {GOOME_SKIN_TOOLTIP, "SKIN_TOOLTIP", "Skin"},
    {GOOME_SKIN_MENU, "SKIN_MENU", "Skin"},
    {GOOME_SKIN_POPUP, "SKIN_POPUP", "Skin"},
    {GOOME_ICON_ARROW_DOWN, "ICON_ARROW_DOWN", "Icon"},
    {GOOME_ICON_ARROW_RIGHT, "ICON_ARROW_RIGHT", "Icon"},
    {GOOME_ICON_ARROW_UP, "ICON_ARROW_UP", "Icon"},
    {GOOME_ICON_ARROW_LEFT, "ICON_ARROW_LEFT", "Icon"},
    {GOOME_ICON_CHECK, "ICON_CHECK", "Icon"},
    {GOOME_ICON_RADIO_DOT, "ICON_RADIO_DOT", "Icon"},
    {GOOME_ICON_CLOSE, "ICON_CLOSE", "Icon"},
    {GOOME_ICON_MINIMIZE, "ICON_MINIMIZE", "Icon"},
    {GOOME_ICON_MAXIMIZE, "ICON_MAXIMIZE", "Icon"},
    {GOOME_ICON_RESTORE, "ICON_RESTORE", "Icon"},
    {GOOME_ICON_RESIZE, "ICON_RESIZE", "Icon"},
    {GOOME_ICON_SORT_UP, "ICON_SORT_UP", "Icon"},
    {GOOME_ICON_SORT_DOWN, "ICON_SORT_DOWN", "Icon"},
    {GOOME_ICON_PLUS, "ICON_PLUS", "Icon"},
    {GOOME_ICON_MINUS, "ICON_MINUS", "Icon"},
    {GOOME_ICON_SLIDER_KNOB, "ICON_SLIDER_KNOB", "Icon"},
    {GOOME_ICON_TOGGLE_KNOB, "ICON_TOGGLE_KNOB", "Icon"},
    {GOOME_ICON_COLLAPSE, "ICON_COLLAPSE", "Icon"},
    {GOOME_ICON_EXPAND, "ICON_EXPAND", "Icon"},
    {GOOME_ICON_DRAG_HANDLE, "ICON_DRAG_HANDLE", "Icon"},
    {GOOME_ICON_TOAST, "ICON_TOAST", "Icon"},
    {GOOME_ICON_PICKER, "ICON_PICKER", "Icon"},
    {GOOME_ITEM_DRAG_X, "ITEM_DRAG_X", "ItemFlags"},
    {GOOME_ITEM_DRAG_Y, "ITEM_DRAG_Y", "ItemFlags"},
    {GOOME_ITEM_FOCUSABLE, "ITEM_FOCUSABLE", "ItemFlags"},
    {GOOME_ITEM_NO_HIT, "ITEM_NO_HIT", "ItemFlags"},
    {GOOME_ITEM_REPEAT, "ITEM_REPEAT", "ItemFlags"},
    {GOOME_K_BACKSPACE, "K_BACKSPACE", "Keys"},
    {GOOME_K_TAB, "K_TAB", "Keys"},
    {GOOME_K_ENTER, "K_ENTER", "Keys"},
    {GOOME_K_ESC, "K_ESC", "Keys"},
    {GOOME_K_SPACE, "K_SPACE", "Keys"},
    {GOOME_K_DELETE, "K_DELETE", "Keys"},
    {GOOME_K_LEFT, "K_LEFT", "Keys"},
    {GOOME_K_RIGHT, "K_RIGHT", "Keys"},
    {GOOME_K_UP, "K_UP", "Keys"},
    {GOOME_K_DOWN, "K_DOWN", "Keys"},
    {GOOME_K_HOME, "K_HOME", "Keys"},
    {GOOME_K_END, "K_END", "Keys"},
    {GOOME_K_PGUP, "K_PGUP", "Keys"},
    {GOOME_K_PGDN, "K_PGDN", "Keys"},
    {GOOME_SHIFT, "MOD_SHIFT", "Mods"},
    {GOOME_CTRL, "MOD_CTRL", "Mods"},
    {GOOME_ALT, "MOD_ALT", "Mods"},
    {GOOME_SUPER, "MOD_SUPER", "Mods"},
    {0, NULL, NULL},
};

static GDExtensionObjectPtr ui_create(void *ud, GDExtensionBool post)
{
    gd_ui *ui = ui_make(NULL);
    return ui ? ui->obj : NULL;
}

static void ui_free(void *ud, GDExtensionClassInstancePtr inst)
{
    gd_ui *ui = inst;
    if (ui->view) ui->view->ui = NULL;
    for (int i = 0; i < ui->nassets; i++) {
        if (ui->assets[i]->font) goome_font_free(ui->assets[i]->font);
        goome_bitmap_free(&ui->assets[i]->bm);
        gd.destroy(&ui->assets[i]->keep);
        free(ui->assets[i]);
    }
    free(ui->assets);
    free(ui);
}

gd_ui *ui_make(gd_view *v)
{
    gd_ui *ui = calloc(1, sizeof(*ui));
    if (!ui) return NULL;
    ui->view = v;
    ui->obj = gd_new("Object");
    gd.set_instance(ui->obj, SN("GoomeUI"), ui);
    return ui;
}

void ui_drop(gd_ui *ui)
{
    if (!ui) return;
    ui->view = NULL;
    gd.object_destroy(ui->obj);
}

void ui_register(void)
{
    GDExtensionClassCreationInfo4 info;
    memset(&info, 0, sizeof(info));
    info.is_exposed = 1;
    info.create_instance_func = ui_create;
    info.free_instance_func = ui_free;
    gd.register_class(gd.lib, SN("GoomeUI"), SN("Object"), &info);
    gd_register_methods("GoomeUI", methods);
    gd_register_consts("GoomeUI", consts);
}
