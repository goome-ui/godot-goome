#define GOOME_IMPLEMENTATION
#include "goome.h"

int gd_goome_ime(goome *g, goome_rect *caret)
{
    if (!g) return 0;
    if (caret) *caret = g->ime_caret;
    return g->ime_on;
}

int gd_goome_ime_push(goome *g, int type, const char *s, int cursor) { return g && s ? gm_ime_push(g, type, s, -1, cursor) : GOOME_EARG; }

void gd_goome_fixed_time(goome_ui *u, int on)
{
    if (u) u->fixed_time = on;
}
