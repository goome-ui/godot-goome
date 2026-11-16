#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "view.h"

enum {
    N_ENTER_TREE = 10,
    N_EXIT_TREE = 11,
    N_READY = 13,
    N_PROCESS = 17,
    N_DRAW = 30,
    N_RESIZED = 40,
    N_MOUSE_EXIT = 42,
    N_WINDOW_FOCUS_IN = 1004,
    N_WINDOW_FOCUS_OUT = 1005,
    N_IME_UPDATE = 2013,
};

enum {
    K_ESCAPE = 4194305,
    K_TAB,
    K_BACKTAB,
    K_BACKSPACE,
    K_ENTER,
    K_KP_ENTER,
    K_INSERT,
    K_DELETE,
    K_HOME = 4194317,
    K_END,
    K_LEFT,
    K_UP,
    K_RIGHT,
    K_DOWN,
    K_PAGEUP,
    K_PAGEDOWN,
    K_F1 = 4194332,
    K_F12 = 4194343,
    K_SPECIAL = 4194304,
};

static uint32_t slot_hash(int kind, const char *id, uint32_t seed)
{
    uint32_t h = (seed ? seed : 2166136261u) ^ (uint32_t)kind;
    h *= 16777619u;
    for (; *id; id++) h = (h ^ (uint8_t)*id) * 16777619u;
    return h;
}

gd_slot *view_slot(gd_view *v, int kind, const char *id, uint32_t seed, int *fresh)
{
    uint32_t key = slot_hash(kind, id ? id : "", seed);
    for (int i = 0; i < v->nslots; i++)
        if (v->slots[i].key == key && v->slots[i].kind == kind) {
            *fresh = 0;
            v->slots[i].seen = v->frames;
            return &v->slots[i];
        }
    if (v->nslots == v->capslots) {
        int cap = v->capslots ? v->capslots * 2 : 16;
        gd_slot *s = realloc(v->slots, (size_t)cap * sizeof(*s));
        if (!s) return NULL;
        v->slots = s, v->capslots = cap;
    }
    gd_slot *s = &v->slots[v->nslots++];
    memset(s, 0, sizeof(*s));
    s->key = key, s->kind = kind, s->seen = v->frames;
    *fresh = 1;
    return s;
}

static void push(gd_view *v, int type, double x, double y, int button, int dy, int key, int mods, const char *text)
{
    if (!v->g) return;
    goome_event e;
    memset(&e, 0, sizeof(e));
    e.type = type, e.x = x, e.y = y, e.button = button, e.dy = dy, e.key = key, e.mods = mods;
    if (text) {
        size_t n = strlen(text);
        if (n > sizeof(e.text) - 1) n = sizeof(e.text) - 1;
        memcpy(e.text, text, n);
    }
    goome_push(v->g, &e);
    v->dirty = 1;
}

static gd_vec2 my_size(gd_view *v)
{
    gd_var r;
    gd_call(&v->me, "get_size", &r, 0);
    gd_vec2 s = gd_as_vec2(&r);
    gd.destroy(&r);
    return s;
}

static int round_px(float x) { return x < 1 ? 1 : (int)(x + 0.5f); }

static double length(double a, double b)
{
    double s = a * a + b * b, r = s > 1 ? s : 1;
    if (s <= 0) return 0;
    for (int i = 0; i < 30; i++) r = (r + s / r) / 2;
    return r;
}

static double my_scale(gd_view *v)
{
    if (v->pixel_scale > 0) return v->pixel_scale;
    gd_var r;
    gd_call(&v->me, "get_screen_transform", &r, 0);
    gd_xform2 t = gd_as_xform2(&r);
    gd.destroy(&r);
    double s = length(t.m[0], t.m[1]);
    if (!(s > 0)) s = 1;
    return s < 0.25 ? 0.25 : s > 8 ? 8 : s;
}

static void set_int(gd_view *v, const char *method, int64_t x)
{
    gd_var a;
    gd_var_int(&a, x);
    gd_call(&v->me, method, NULL, 1, &a);
    gd.destroy(&a);
}

static void open_goome(gd_view *v)
{
    gd_vec2 sz = my_size(v);
    v->w = round_px(sz.x);
    v->h = round_px(sz.y);
    v->scale = my_scale(v);
    int err = 0;
    goome_opts o;
    memset(&o, 0, sizeof(o));
    o.title = "godot";
    o.mode = GOOME_MODE_OFFSCREEN;
    o.width = v->w, o.height = v->h, o.scale = v->scale;
    v->g = goome_open(&o, &err);
    if (!v->g) {
        gd_err("goome: cant open offscreen goome");
        return;
    }
    v->u = goome_ui_new(v->g);
    if (!v->u) {
        goome_close(v->g);
        v->g = NULL;
        gd_err("goome: cant make the ui");
        return;
    }
    if (v->light_theme) goome_ui_theme_light(goome_ui_theme(v->u));
    if (v->fixed_step) gd_goome_fixed_time(v->u, 1);
    if (v->a11y >= 0) goome_ui_a11y(v->u, v->a11y);
    v->dirty = 1;
}

static void display_server(gd_var *out);

static int64_t window_id(gd_view *v)
{
    gd_var w, r;
    gd_call(&v->me, "get_window", &w, 0);
    int64_t id = 0;
    if (gd_as_obj(&w)) {
        gd_call(&w, "get_window_id", &r, 0);
        id = gd_as_int(&r);
        gd.destroy(&r);
    }
    gd.destroy(&w);
    return id;
}

static void ime_set(gd_view *v, int on)
{
    gd_var ds, a, id;
    display_server(&ds);
    gd_var_bool(&a, on);
    gd_var_int(&id, window_id(v));
    gd_call(&ds, "window_set_ime_active", NULL, 2, &a, &id);
    gd.destroy(&a);
    gd.destroy(&id);
    gd.destroy(&ds);
    v->ime_active = on;
    v->ime_at = (goome_rect){-1, -1, -1, -1};
}

static void ime_place(gd_view *v, goome_rect r)
{
    gd_var t, ds, p, id;
    gd_call(&v->me, "get_global_transform_with_canvas", &t, 0);
    gd_xform2 m = gd_as_xform2(&t);
    gd.destroy(&t);
    double x = r.x, y = r.y + r.h;
    display_server(&ds);
    gd_var_vec2i(&p, (int)(m.m[0] * x + m.m[2] * y + m.m[4]), (int)(m.m[1] * x + m.m[3] * y + m.m[5]));
    gd_var_int(&id, window_id(v));
    gd_call(&ds, "window_set_ime_position", NULL, 2, &p, &id);
    gd.destroy(&p);
    gd.destroy(&id);
    gd.destroy(&ds);
    v->ime_at = r;
}

static void ime_sync(gd_view *v)
{
    goome_rect r;
    int on = v->g && v->take_keyboard && gd_goome_ime(v->g, &r);
    if (on != v->ime_active) ime_set(v, on);
    if (on && memcmp(&r, &v->ime_at, sizeof(r))) ime_place(v, r);
}

static int utf8_at(const char *s, int64_t chars)
{
    int i = 0;
    for (; s[i] && chars > 0; chars--) {
        i++;
        while (((unsigned char)s[i] & 0xC0) == 0x80) i++;
    }
    return i;
}

static void ime_update(gd_view *v)
{
    if (!v->g || !v->ime_active) return;
    gd_var ds, t, sel;
    display_server(&ds);
    gd_call(&ds, "ime_get_text", &t, 0);
    gd_call(&ds, "ime_get_selection", &sel, 0);
    gd.destroy(&ds);
    char *s = gd_as_cstr(&t);
    gd_vec2i at = gd_as_vec2i(&sel);
    gd.destroy(&t);
    gd.destroy(&sel);
    if (!s) return;
    gd_goome_ime_push(v->g, GOOME_PREEDIT, s, utf8_at(s, at.x));
    free(s);
    v->dirty = 1;
}

static void ime_signal(gd_view *v, const goome_event *ev)
{
    gd_var sig, t, c;
    gd_var_sn(&sig, ev->type == GOOME_TEXT ? "ime_text" : "ime_preedit");
    const char *s = goome_event_text(v->g);
    int64_t chars = 0;
    for (int i = 0; s[i] && i < ev->id; i++) chars += ((unsigned char)s[i] & 0xC0) != 0x80;
    gd_var_cstr(&t, s);
    gd_var_int(&c, chars);
    gd_call(&v->me, "emit_signal", NULL, ev->type == GOOME_TEXT ? 2 : 3, &sig, &t, &c);
    gd.destroy(&sig);
    gd.destroy(&t);
    gd.destroy(&c);
}

static void close_goome(gd_view *v)
{
    if (v->ime_active) ime_set(v, 0);
    if (v->u) goome_ui_free(v->u);
    if (v->g) goome_close(v->g);
    v->u = NULL, v->g = NULL;
    v->texw = v->texh = 0;
}

static void image_data(gd_var *img, int w, int h, gd_var *bytes)
{
    gd_var aw, ah, mip, fmt;
    gd_var_int(&aw, w);
    gd_var_int(&ah, h);
    gd_var_bool(&mip, 0);
    gd_var_int(&fmt, 5);
    gd_call(img, "set_data", NULL, 5, &aw, &ah, &mip, &fmt, bytes);
    gd.destroy(&aw);
    gd.destroy(&ah);
    gd.destroy(&mip);
    gd.destroy(&fmt);
}

static void upload(gd_view *v)
{
    goome_rect r;
    goome_image f;
    if (goome_frame(v->g, &f) != GOOME_OK) return;
    int whole = f.w != v->texw || f.h != v->texh || gd_is(&v->img, T_NIL) || gd_is(&v->tex, T_NIL);
    if (!goome_frame_dirty(v->g, &r) && !whole) return;
    if (whole) r = (goome_rect){0, 0, f.w, f.h};
    if (r.w <= 0 || r.h <= 0) return;
    gd_var bytes;
    uint8_t *px;
    gd_bytes_var(&bytes, (int64_t)r.w * r.h * 4, &px);
    if (!px || goome_frame_copy(v->g, px, r.w * 4, GOOME_FMT_RGBA_PREMUL, &r) != GOOME_OK) {
        gd.destroy(&bytes);
        return;
    }
    if (gd_is(&v->img, T_NIL)) gd_new_var(&v->img, "Image");
    if (whole) {
        image_data(&v->img, r.w, r.h, &bytes);
        if (gd_is(&v->tex, T_NIL)) gd_new_var(&v->tex, "ImageTexture");
        gd_call(&v->tex, "set_image", NULL, 1, &v->img);
        v->texw = f.w, v->texh = f.h;
    } else {
        if (gd_is(&v->part, T_NIL)) gd_new_var(&v->part, "Image");
        image_data(&v->part, r.w, r.h, &bytes);
        gd_var src, dst;
        gd_var_rect2i(&src, 0, 0, r.w, r.h);
        gd_var_vec2i(&dst, r.x, r.y);
        gd_call(&v->img, "blit_rect", NULL, 3, &v->part, &src, &dst);
        gd.destroy(&src);
        gd.destroy(&dst);
        gd_call(&v->tex, "update", NULL, 1, &v->img);
    }
    gd.destroy(&bytes);
    gd_call(&v->me, "queue_redraw", NULL, 0);
}

static int has_method(gd_view *v, const char *name)
{
    gd_var n, r;
    gd_var_sn(&n, name);
    gd_call(&v->me, "has_method", &r, 1, &n);
    int ok = gd_as_bool(&r);
    gd.destroy(&n);
    gd.destroy(&r);
    return ok;
}

static void render(gd_view *v)
{
    v->dirty = 0;
    if (!goome_ui_begin(v->u, 0)) return;
    v->frames++;
    v->in_frame = 1;
    if (v->ui) v->ui->nids = 0, v->ui->changed = 0, v->ui->canvas = NULL;
    if (has_method(v, "_goome_frame")) gd_call(&v->me, "_goome_frame", NULL, 1, &v->uivar);
    gd_var sig;
    gd_var_sn(&sig, "goome_frame");
    gd_call(&v->me, "emit_signal", NULL, 2, &sig, &v->uivar);
    gd.destroy(&sig);
    v->in_frame = 0;
    goome_ui_end(v->u);
    upload(v);
}

static int cursor_of(int shape)
{
    switch (shape) {
    case GOOME_CURSOR_IBEAM: return 1;
    case GOOME_CURSOR_HAND: return 2;
    case GOOME_CURSOR_MOVE: return 13;
    case GOOME_CURSOR_RESIZE_EW: return 10;
    case GOOME_CURSOR_RESIZE_NS: return 9;
    case GOOME_CURSOR_RESIZE_NWSE: return 12;
    case GOOME_CURSOR_RESIZE_NESW: return 11;
    case GOOME_CURSOR_WAIT: return 5;
    case GOOME_CURSOR_NO: return 8;
    }
    return 0;
}

static void display_server(gd_var *out) { gd_var_obj(out, gd.singleton(SN("DisplayServer"))); }

static void paste_in(gd_view *v)
{
    gd_var ds, r;
    display_server(&ds);
    gd_call(&ds, "clipboard_get", &r, 0);
    char *s = gd_as_cstr(&r);
    gd.destroy(&r);
    gd.destroy(&ds);
    if (!s) return;
    if (!v->clip || strcmp(s, v->clip)) {
        goome_clipboard_set(v->g, s);
        free(v->clip);
        v->clip = s;
    } else
        free(s);
}

static void copy_out(gd_view *v)
{
    int n = goome_clipboard_get(v->g, NULL, 0);
    if (n < 0) return;
    char *s = malloc((size_t)n + 1);
    if (!s) return;
    goome_clipboard_get(v->g, s, n + 1);
    if (v->clip && !strcmp(s, v->clip)) {
        free(s);
        return;
    }
    free(v->clip);
    v->clip = s;
    gd_var ds, a;
    display_server(&ds);
    gd_var_cstr(&a, s);
    gd_call(&ds, "clipboard_set", NULL, 1, &a);
    gd.destroy(&a);
    gd.destroy(&ds);
}

static void tick(gd_view *v)
{
    if (!v->g) open_goome(v);
    if (!v->g) return;
    gd_vec2 sz = my_size(v);
    int w = round_px(sz.x), h = round_px(sz.y);
    double s = my_scale(v);
    if (w != v->w || h != v->h || (s - v->scale > 0.001 || v->scale - s > 0.001)) {
        v->w = w, v->h = h, v->scale = s;
        goome_resize(v->g, w, h, s);
        v->dirty = 1;
    }
    goome_event ev;
    while (goome_poll(v->g, &ev, 0) > 0) {
        if (ev.type == GOOME_TEXT || ev.type == GOOME_PREEDIT) ime_signal(v, &ev);
        goome_ui_feed(v->u, &ev);
        v->dirty = 1;
    }
    if (v->dirty || v->redraw_every_frame || goome_ui_animating(v->u)) render(v);
    ime_sync(v);
    v->wants = goome_ui_wants(v->u);
    goome_event far;
    memset(&far, 0, sizeof(far));
    far.type = GOOME_MOTION, far.x = far.y = -100000;
    goome_ui_feed(v->u, &far);
    v->sticky = (goome_ui_wants(v->u) & GOOME_WANT_MOUSE) != 0;
    if (v->inside) {
        far.x = v->mx, far.y = v->my;
        goome_ui_feed(v->u, &far);
    }
    int shape = cursor_of(goome_cursor_shape(v->g));
    if (shape != v->shape) {
        v->shape = shape;
        set_int(v, "set_default_cursor_shape", shape);
    }
    if (v->check_clip && v->sync_clipboard) {
        v->check_clip = 0;
        copy_out(v);
    }
}

static int mods_of(const gd_var *ev)
{
    gd_var r;
    int m = 0;
    static const char *const names[4] = {"is_shift_pressed", "is_ctrl_pressed", "is_alt_pressed", "is_meta_pressed"};
    static const int bits[4] = {GOOME_SHIFT, GOOME_CTRL, GOOME_ALT, GOOME_SUPER};
    for (int i = 0; i < 4; i++) {
        gd_call((gd_var *)ev, names[i], &r, 0);
        if (gd_as_bool(&r)) m |= bits[i];
        gd.destroy(&r);
    }
    return m;
}

static int is_a(gd_var *ev, const char *cls)
{
    gd_var n, r;
    gd_var_cstr(&n, cls);
    gd_call(ev, "is_class", &r, 1, &n);
    int ok = gd_as_bool(&r);
    gd.destroy(&n);
    gd.destroy(&r);
    return ok;
}

static int64_t get_int(gd_var *ev, const char *m)
{
    gd_var r;
    gd_call(ev, m, &r, 0);
    int64_t x = gd_as_int(&r);
    gd.destroy(&r);
    return x;
}

static gd_vec2 get_pos(gd_var *ev)
{
    gd_var r;
    gd_call(ev, "get_position", &r, 0);
    gd_vec2 p = gd_as_vec2(&r);
    gd.destroy(&r);
    return p;
}

static int key_of(int64_t k)
{
    switch (k) {
    case K_ESCAPE: return GOOME_K_ESC;
    case K_TAB:
    case K_BACKTAB: return GOOME_K_TAB;
    case K_BACKSPACE: return GOOME_K_BACKSPACE;
    case K_ENTER:
    case K_KP_ENTER: return GOOME_K_ENTER;
    case K_INSERT: return GOOME_K_INSERT;
    case K_DELETE: return GOOME_K_DELETE;
    case K_HOME: return GOOME_K_HOME;
    case K_END: return GOOME_K_END;
    case K_LEFT: return GOOME_K_LEFT;
    case K_UP: return GOOME_K_UP;
    case K_RIGHT: return GOOME_K_RIGHT;
    case K_DOWN: return GOOME_K_DOWN;
    case K_PAGEUP: return GOOME_K_PGUP;
    case K_PAGEDOWN: return GOOME_K_PGDN;
    }
    if (k >= K_F1 && k <= K_F12) return GOOME_K_F1 + (int)(k - K_F1);
    if (k >= 'A' && k <= 'Z') return (int)k + 32;
    if (k >= 32 && k < 127) return (int)k;
    return 0;
}

static int utf8_of(uint32_t c, char *out)
{
    if (c < 0x80) return out[0] = (char)c, out[1] = 0, 1;
    if (c < 0x800) return out[0] = (char)(0xC0 | c >> 6), out[1] = (char)(0x80 | (c & 63)), out[2] = 0, 2;
    if (c < 0x10000) return out[0] = (char)(0xE0 | c >> 12), out[1] = (char)(0x80 | ((c >> 6) & 63)), out[2] = (char)(0x80 | (c & 63)), out[3] = 0, 3;
    if (c > 0x10FFFF) return out[0] = 0, 0;
    out[0] = (char)(0xF0 | c >> 18), out[1] = (char)(0x80 | ((c >> 12) & 63)), out[2] = (char)(0x80 | ((c >> 6) & 63)), out[3] = (char)(0x80 | (c & 63)), out[4] = 0;
    return 4;
}

static void accept(gd_view *v) { gd_call(&v->me, "accept_event", NULL, 0); }

static int mouse_ours(gd_view *v, double x, double y)
{
    if (!v->g) return 0;
    if (v->buttons) return v->pressed_on_ui;
    return goome_hit(v->g, x, y) || v->sticky;
}

static void gui_input(gd_view *v, gd_var *ev)
{
    if (!v->g) return;
    if (is_a(ev, "InputEventMouseButton")) {
        gd_vec2 p = get_pos(ev);
        int b = (int)get_int(ev, "get_button_index");
        gd_var r;
        gd_call(ev, "is_pressed", &r, 0);
        int down = gd_as_bool(&r);
        gd.destroy(&r);
        int mods = mods_of(ev), ours;
        v->mx = p.x, v->my = p.y, v->inside = 1;
        if (b == 4 || b == 5 || b == 6 || b == 7) {
            ours = goome_hit(v->g, p.x, p.y) || v->sticky;
            if (down && ours && (b == 4 || b == 5)) push(v, GOOME_SCROLL, p.x, p.y, 0, b == 4 ? -1 : 1, 0, mods, NULL);
            if (ours) accept(v);
            return;
        }
        int gb = b == 1 ? 1 : b == 2 ? 3 : b == 3 ? 2 : 0;
        if (!gb) return;
        int bit = 1 << (gb - 1);
        if (down) {
            if (!v->buttons) v->pressed_on_ui = goome_hit(v->g, p.x, p.y) || v->sticky;
            v->buttons |= bit;
            push(v, GOOME_DOWN, p.x, p.y, gb, 0, 0, mods, NULL);
            ours = v->pressed_on_ui;
            if (ours && gb == 1) gd_call(&v->me, "grab_focus", NULL, 0);
        } else {
            ours = v->buttons ? v->pressed_on_ui : 0;
            v->buttons &= ~bit;
            push(v, GOOME_UP, p.x, p.y, gb, 0, 0, mods, NULL);
            v->check_clip = 1;
        }
        if (ours) accept(v);
        return;
    }
    if (is_a(ev, "InputEventMouseMotion")) {
        gd_vec2 p = get_pos(ev);
        v->mx = p.x, v->my = p.y, v->inside = 1;
        push(v, GOOME_MOTION, p.x, p.y, 0, 0, 0, 0, NULL);
        if (mouse_ours(v, p.x, p.y)) accept(v);
        return;
    }
    if (is_a(ev, "InputEventKey")) {
        if (!v->take_keyboard || !(v->wants & GOOME_WANT_KEYBOARD)) return;
        accept(v);
        gd_var r;
        gd_call(ev, "is_pressed", &r, 0);
        int down = gd_as_bool(&r);
        gd.destroy(&r);
        if (!down) return;
        int mods = mods_of(ev);
        int64_t kc = get_int(ev, "get_keycode");
        if (!kc) kc = get_int(ev, "get_physical_keycode");
        uint32_t uc = (uint32_t)get_int(ev, "get_unicode");
        if (!kc && uc >= 32 && uc != 127) {
            char t[8];
            utf8_of(uc, t);
            gd_goome_ime_push(v->g, GOOME_TEXT, t, 0);
            v->dirty = 1;
            return;
        }
        int key = key_of(kc);
        char text[8] = {0};
        int combo = (mods & (GOOME_CTRL | GOOME_ALT | GOOME_SUPER)) != 0;
        if (!combo && uc >= 32 && uc != 127) utf8_of(uc, text);
        if (!key && !text[0]) return;
        if (kc >= 'A' && kc <= 'Z' && uc >= 'A' && uc <= 'Z') mods |= GOOME_SHIFT;
        if (combo && v->sync_clipboard) {
            if (key == 'v') paste_in(v);
            if (key == 'c' || key == 'x') v->check_clip = 1;
        }
        push(v, GOOME_KEY, v->mx, v->my, 0, 0, key, mods, text[0] ? text : NULL);
    }
}

static GDExtensionBool has_point(gd_view *v, gd_vec2 p);

static void any_input(gd_view *v, gd_var *ev)
{
    if (!v->g || !(v->wants & GOOME_WANT_KEYBOARD) || !is_a(ev, "InputEventMouseButton")) return;
    gd_var r;
    gd_call(ev, "is_pressed", &r, 0);
    int down = gd_as_bool(&r);
    gd.destroy(&r);
    if (!down) return;
    gd_call(&v->me, "get_local_mouse_position", &r, 0);
    gd_vec2 p = gd_as_vec2(&r);
    gd.destroy(&r);
    if (has_point(v, p)) return;
    push(v, GOOME_DOWN, p.x, p.y, 1, 0, 0, 0, NULL);
    push(v, GOOME_UP, p.x, p.y, 1, 0, 0, 0, NULL);
    v->wants &= ~GOOME_WANT_KEYBOARD;
    gd_call(&v->me, "release_focus", NULL, 0);
}

static GDExtensionBool has_point(gd_view *v, gd_vec2 p)
{
    if (!v->g) return 0;
    if (v->buttons) return v->pressed_on_ui;
    return goome_hit(v->g, p.x, p.y) || v->sticky;
}

static void draw(gd_view *v)
{
    if (gd_is(&v->tex, T_NIL)) return;
    gd_vec2 sz = my_size(v);
    gd_var rect, tile;
    gd_var_rect2(&rect, (gd_rect2){0, 0, sz.x, sz.y});
    gd_var_bool(&tile, 0);
    gd_call(&v->me, "draw_texture_rect", NULL, 3, &v->tex, &rect, &tile);
    gd.destroy(&rect);
    gd.destroy(&tile);
}

static void ready(gd_view *v)
{
    gd_var on;
    gd_var_bool(&on, 1);
    gd_call(&v->me, "set_process", NULL, 1, &on);
    gd_call(&v->me, "set_process_input", NULL, 1, &on);
    gd.destroy(&on);
    set_int(v, "set_focus_mode", 1);
    gd_var cur;
    gd_call(&v->me, "get_material", &cur, 0);
    if (gd_is(&cur, T_NIL)) {
        if (gd_is(&v->mat, T_NIL)) {
            gd_new_var(&v->mat, "CanvasItemMaterial");
            gd_var mode;
            gd_var_int(&mode, 4);
            gd_call(&v->mat, "set_blend_mode", NULL, 1, &mode);
            gd.destroy(&mode);
        }
        gd_call(&v->me, "set_material", NULL, 1, &v->mat);
    }
    gd.destroy(&cur);
}

static void notify(GDExtensionClassInstancePtr inst, int32_t what, GDExtensionBool reversed)
{
    gd_view *v = inst;
    switch (what) {
    case N_READY: ready(v); break;
    case N_PROCESS: tick(v); break;
    case N_DRAW: draw(v); break;
    case N_EXIT_TREE: close_goome(v); break;
    case N_RESIZED: v->dirty = 1; break;
    case N_MOUSE_EXIT:
        v->inside = 0;
        if (!v->buttons) push(v, GOOME_MOTION, -100000, -100000, 0, 0, 0, 0, NULL);
        break;
    case N_IME_UPDATE: ime_update(v); break;
    case N_WINDOW_FOCUS_IN:
    case N_WINDOW_FOCUS_OUT:
        if (v->g) {
            goome_event e;
            memset(&e, 0, sizeof(e));
            e.type = GOOME_FOCUS, e.id = what == N_WINDOW_FOCUS_IN;
            goome_push(v->g, &e);
            v->dirty = 1;
        }
        break;
    }
}

static void call_gui_input(GDExtensionClassInstancePtr inst, const GDExtensionConstTypePtr *args, GDExtensionTypePtr ret)
{
    gd_var ev;
    gd_var_obj(&ev, gd.ref_get(args[0]));
    if (!gd_is(&ev, T_NIL)) gui_input(inst, &ev);
    gd.destroy(&ev);
}

static void call_input(GDExtensionClassInstancePtr inst, const GDExtensionConstTypePtr *args, GDExtensionTypePtr ret)
{
    gd_var ev;
    gd_var_obj(&ev, gd.ref_get(args[0]));
    if (!gd_is(&ev, T_NIL)) any_input(inst, &ev);
    gd.destroy(&ev);
}

static void call_has_point(GDExtensionClassInstancePtr inst, const GDExtensionConstTypePtr *args, GDExtensionTypePtr ret)
{
    *(GDExtensionBool *)ret = has_point(inst, *(const gd_vec2 *)args[0]);
}

static GDExtensionClassCallVirtual get_virtual(void *ud, GDExtensionConstStringNamePtr name, uint32_t hash)
{
    if (!memcmp(name, SN("_gui_input"), sizeof(gd_sn))) return call_gui_input;
    if (!memcmp(name, SN("_has_point"), sizeof(gd_sn))) return call_has_point;
    if (!memcmp(name, SN("_input"), sizeof(gd_sn))) return call_input;
    return NULL;
}

static GDExtensionObjectPtr view_create(void *ud, GDExtensionBool post)
{
    gd_view *v = calloc(1, sizeof(*v));
    if (!v) return NULL;
    v->obj = gd_new("Control");
    gd.set_instance(v->obj, SN("GoomeView"), v);
    gd_var_obj(&v->me, v->obj);
    gd.nil(&v->img);
    gd.nil(&v->part);
    gd.nil(&v->tex);
    gd.nil(&v->mat);
    v->redraw_every_frame = 1, v->take_keyboard = 1, v->sync_clipboard = 1;
    v->shape = -1, v->a11y = -1;
    v->ui = ui_make(v);
    if (v->ui) gd_var_obj(&v->uivar, v->ui->obj);
    else gd.nil(&v->uivar);
    return v->obj;
}

static void view_free(void *ud, GDExtensionClassInstancePtr inst)
{
    gd_view *v = inst;
    v->ime_active = 0;
    close_goome(v);
    gd.destroy(&v->img);
    gd.destroy(&v->part);
    gd.destroy(&v->tex);
    gd.destroy(&v->mat);
    gd.destroy(&v->uivar);
    gd.destroy(&v->me);
    if (v->ui) ui_drop(v->ui);
    for (int i = 0; i < v->nslots; i++) free(v->slots[i].buf);
    free(v->slots);
    free(v->clip);
    free(v);
}

#define V gd_view *v = inst

static void rb(gd_var *r, int x)
{
    gd.destroy(r);
    gd_var_bool(r, x);
}

static void m_redraw(void *inst, gd_args *a, gd_var *ret)
{
    V;
    v->dirty = 1;
}

static void m_get_ui(void *inst, gd_args *a, gd_var *ret)
{
    V;
    gd.destroy(ret);
    gd.copy(ret, &v->uivar);
}

static void m_get_texture(void *inst, gd_args *a, gd_var *ret)
{
    V;
    gd.destroy(ret);
    gd.copy(ret, &v->tex);
}

static void m_get_image(void *inst, gd_args *a, gd_var *ret)
{
    V;
    gd.destroy(ret);
    gd.copy(ret, &v->img);
}

static void m_wants_mouse(void *inst, gd_args *a, gd_var *ret)
{
    V;
    rb(ret, (v->wants & GOOME_WANT_MOUSE) != 0);
}

static void m_wants_keyboard(void *inst, gd_args *a, gd_var *ret)
{
    V;
    rb(ret, v->take_keyboard && (v->wants & GOOME_WANT_KEYBOARD) != 0);
}

static void m_hit(void *inst, gd_args *a, gd_var *ret)
{
    V;
    gd_vec2 p = gd_as_vec2(a->v[0]);
    rb(ret, v->g && goome_hit(v->g, p.x, p.y));
}

static void m_push_text(void *inst, gd_args *a, gd_var *ret)
{
    V;
    for (const char *s = a->s[0]; *s;) {
        int n = 1;
        unsigned char c = (unsigned char)*s;
        if (c >= 0xF0) n = 4;
        else if (c >= 0xE0) n = 3;
        else if (c >= 0xC0) n = 2;
        char t[8] = {0};
        for (int i = 0; i < n && s[i]; i++) t[i] = s[i];
        push(v, GOOME_KEY, v->mx, v->my, 0, 0, c < 128 ? key_of(c) : 0, c >= 'A' && c <= 'Z' ? GOOME_SHIFT : 0, t);
        s += strlen(t);
    }
}

static void m_push_key(void *inst, gd_args *a, gd_var *ret)
{
    V;
    int key = (int)gd_as_int(a->v[0]), mods = (int)gd_as_int(a->v[1]);
    char t[2] = {0, 0};
    if (key >= 32 && key < 127 && !(mods & (GOOME_CTRL | GOOME_ALT | GOOME_SUPER))) t[0] = (char)key;
    push(v, GOOME_KEY, v->mx, v->my, 0, 0, key, mods, t[0] ? t : NULL);
}

static void m_push_ime_text(void *inst, gd_args *a, gd_var *ret)
{
    V;
    if (v->g && a->s[0][0]) gd_goome_ime_push(v->g, GOOME_TEXT, a->s[0], 0);
    v->dirty = 1;
}

static void m_push_preedit(void *inst, gd_args *a, gd_var *ret)
{
    V;
    int64_t c = gd_as_int(a->v[1]);
    if (v->g) gd_goome_ime_push(v->g, GOOME_PREEDIT, a->s[0], c < 0 ? (int)strlen(a->s[0]) : utf8_at(a->s[0], c));
    v->dirty = 1;
}

static void m_is_ime_active(void *inst, gd_args *a, gd_var *ret)
{
    V;
    rb(ret, v->g && gd_goome_ime(v->g, NULL));
}

static void m_get_ime_caret(void *inst, gd_args *a, gd_var *ret)
{
    V;
    goome_rect r = {0, 0, 0, 0};
    if (v->g) gd_goome_ime(v->g, &r);
    gd.destroy(ret);
    gd_var_rect2(ret, (gd_rect2){(float)r.x, (float)r.y, (float)r.w, (float)r.h});
}

static void m_a11y_action(void *inst, gd_args *a, gd_var *ret)
{
    V;
    if (!v->g) return;
    goome_event e;
    memset(&e, 0, sizeof(e));
    e.type = GOOME_CONTROL;
    int64_t id = gd_as_int(a->v[1]);
    snprintf(e.msg, sizeof(e.msg), "a11y %s %u%s%s", a->s[0], (unsigned)id, a->s[2][0] ? " " : "", a->s[2]);
    goome_push(v->g, &e);
    v->dirty = 1;
}

static void set_fixed_step(void *inst, gd_args *a, gd_var *ret)
{
    V;
    v->fixed_step = gd_as_bool(a->v[0]);
    if (v->u) gd_goome_fixed_time(v->u, v->fixed_step);
}

static void get_fixed_step(void *inst, gd_args *a, gd_var *ret)
{
    V;
    rb(ret, v->fixed_step);
}

#define FLAG(name, field)                                                \
    static void set_##name(void *inst, gd_args *a, gd_var *ret)         \
    {                                                                    \
        V;                                                               \
        v->field = gd_as_bool(a->v[0]);                                  \
        v->dirty = 1;                                                    \
    }                                                                    \
    static void get_##name(void *inst, gd_args *a, gd_var *ret)         \
    {                                                                    \
        V;                                                               \
        rb(ret, v->field);                                               \
    }

FLAG(redraw_every_frame, redraw_every_frame)
FLAG(take_keyboard, take_keyboard)
FLAG(sync_clipboard, sync_clipboard)

static void set_light_theme(void *inst, gd_args *a, gd_var *ret)
{
    V;
    v->light_theme = gd_as_bool(a->v[0]);
    if (v->u) {
        if (v->light_theme) goome_ui_theme_light(goome_ui_theme(v->u));
        else goome_ui_theme_dark(goome_ui_theme(v->u));
    }
    v->dirty = 1;
}

static void get_light_theme(void *inst, gd_args *a, gd_var *ret)
{
    V;
    rb(ret, v->light_theme);
}

static void set_pixel_scale(void *inst, gd_args *a, gd_var *ret)
{
    V;
    double s = gd_as_float(a->v[0]);
    v->pixel_scale = s < 0 ? 0 : s > 8 ? 8 : s;
}

static void get_pixel_scale(void *inst, gd_args *a, gd_var *ret)
{
    V;
    gd.destroy(ret);
    gd_var_float(ret, v->pixel_scale);
}

#define VM0(n, f, r) {n, f, r, NULL, {{0}}, 0, 0, 0, {{{0}}}}
#define VM(n, f, r, ...) {n, f, r, NULL, {__VA_ARGS__}, 0, 0, 0, {{{0}}}}
#define VMO(n, f, cls, refc) {n, f, T_OBJECT, cls, {{0}}, refc, 0, 0, {{{0}}}}

static gd_method methods[] = {
    VM0("redraw", m_redraw, T_VOID),
    VMO("get_ui", m_get_ui, "GoomeUI", 0),
    VMO("get_texture", m_get_texture, "ImageTexture", 1),
    VMO("get_image", m_get_image, "Image", 1),
    VM0("wants_mouse", m_wants_mouse, T_BOOL),
    VM0("wants_keyboard", m_wants_keyboard, T_BOOL),
    VM("hit", m_hit, T_BOOL, {"position", T_VEC2, NULL, NULL}),
    VM("push_text", m_push_text, T_VOID, {"text", T_STRING, NULL, NULL}),
    VM("push_key", m_push_key, T_VOID, {"key", T_INT, NULL, NULL}, {"mods", T_INT, "0", NULL}),
    VM("push_ime_text", m_push_ime_text, T_VOID, {"text", T_STRING, NULL, NULL}),
    VM("push_preedit", m_push_preedit, T_VOID, {"text", T_STRING, NULL, NULL}, {"caret", T_INT, "-1", NULL}),
    VM0("is_ime_active", m_is_ime_active, T_BOOL),
    VM0("get_ime_caret", m_get_ime_caret, T_RECT2),
    VM("a11y_action", m_a11y_action, T_VOID, {"action", T_STRING, NULL, NULL}, {"id", T_INT, NULL, NULL}, {"text", T_STRING, "", NULL}),
    VM("set_fixed_step", set_fixed_step, T_VOID, {"on", T_BOOL, NULL, NULL}),
    VM0("get_fixed_step", get_fixed_step, T_BOOL),
    VM("set_redraw_every_frame", set_redraw_every_frame, T_VOID, {"on", T_BOOL, NULL, NULL}),
    VM0("get_redraw_every_frame", get_redraw_every_frame, T_BOOL),
    VM("set_take_keyboard", set_take_keyboard, T_VOID, {"on", T_BOOL, NULL, NULL}),
    VM0("get_take_keyboard", get_take_keyboard, T_BOOL),
    VM("set_sync_clipboard", set_sync_clipboard, T_VOID, {"on", T_BOOL, NULL, NULL}),
    VM0("get_sync_clipboard", get_sync_clipboard, T_BOOL),
    VM("set_light_theme", set_light_theme, T_VOID, {"on", T_BOOL, NULL, NULL}),
    VM0("get_light_theme", get_light_theme, T_BOOL),
    VM("set_pixel_scale", set_pixel_scale, T_VOID, {"scale", T_FLOAT, NULL, NULL}),
    VM0("get_pixel_scale", get_pixel_scale, T_FLOAT),
    {NULL},
};

void view_register(void)
{
    GDExtensionClassCreationInfo4 info;
    memset(&info, 0, sizeof(info));
    info.is_exposed = 1;
    info.notification_func = notify;
    info.create_instance_func = view_create;
    info.free_instance_func = view_free;
    info.get_virtual_func = get_virtual;
    gd.register_class(gd.lib, SN("GoomeView"), SN("Control"), &info);
    gd_register_methods("GoomeView", methods);
    gd_register_prop("GoomeView", "redraw_every_frame", T_BOOL, "set_redraw_every_frame", "get_redraw_every_frame");
    gd_register_prop("GoomeView", "take_keyboard", T_BOOL, "set_take_keyboard", "get_take_keyboard");
    gd_register_prop("GoomeView", "sync_clipboard", T_BOOL, "set_sync_clipboard", "get_sync_clipboard");
    gd_register_prop("GoomeView", "light_theme", T_BOOL, "set_light_theme", "get_light_theme");
    gd_register_prop("GoomeView", "pixel_scale", T_FLOAT, "set_pixel_scale", "get_pixel_scale");
    gd_register_prop("GoomeView", "fixed_step", T_BOOL, "set_fixed_step", "get_fixed_step");
    gd_register_signal("GoomeView", "goome_frame", "ui", "GoomeUI");
    static const char *const text_args[] = {"text"}, *const pre_args[] = {"text", "caret"};
    static const int text_types[] = {T_STRING}, pre_types[] = {T_STRING, T_INT};
    gd_register_signal_args("GoomeView", "ime_text", 1, text_args, text_types);
    gd_register_signal_args("GoomeView", "ime_preedit", 2, pre_args, pre_types);
}
