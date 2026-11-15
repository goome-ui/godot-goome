#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gd.h"

gd_api gd;

#define LOAD(field, name)                                         \
    do {                                                          \
        *(void **)&gd.field = (void *)get(name);                  \
        if (!gd.field) {                                          \
            fprintf(stderr, "goome: this godot has no %s\n", name); \
            return 0;                                             \
        }                                                         \
    } while (0)

typedef void (*gd_any_fn)(void);

int gd_load(GDExtensionInterfaceGetProcAddress get, GDExtensionClassLibraryPtr lib)
{
    gd.lib = lib;
    gd.get = get;
    LOAD(call, "variant_call");
    LOAD(copy, "variant_new_copy");
    LOAD(nil, "variant_new_nil");
    LOAD(destroy, "variant_destroy");
    LOAD(type, "variant_get_type");
    LOAD(construct, "variant_construct");
    LOAD(get_indexed, "variant_get_indexed");
    LOAD(set_indexed, "variant_set_indexed");
    LOAD(set, "variant_set");
    LOAD(ptr_ctor, "variant_get_ptr_constructor");
    LOAD(ptr_method, "variant_get_ptr_builtin_method");
    LOAD(str_new, "string_new_with_utf8_chars_and_len2");
    LOAD(str_utf8, "string_to_utf8_chars");
    LOAD(sn_new, "string_name_new_with_latin1_chars");
    LOAD(bytes_at, "packed_byte_array_operator_index");
    LOAD(singleton, "global_get_singleton");
    LOAD(construct_object, "classdb_construct_object2");
    LOAD(set_instance, "object_set_instance");
    LOAD(object_destroy, "object_destroy");
    LOAD(ref_get, "ref_get_object");
    LOAD(ref_set, "ref_set_object");
    LOAD(register_class, "classdb_register_extension_class4");
    LOAD(register_method, "classdb_register_extension_class_method");
    LOAD(register_const, "classdb_register_extension_class_integer_constant");
    LOAD(register_prop, "classdb_register_extension_class_property");
    LOAD(register_signal, "classdb_register_extension_class_signal");
    LOAD(unregister_class, "classdb_unregister_extension_class");
    LOAD(print_error, "print_error");
    LOAD(print_warning, "print_warning");
    GDExtensionInterfaceGetVariantFromTypeConstructor from;
    GDExtensionInterfaceGetVariantToTypeConstructor to;
    GDExtensionInterfaceVariantGetPtrDestructor dtor;
    *(void **)&from = (void *)get("get_variant_from_type_constructor");
    *(void **)&to = (void *)get("get_variant_to_type_constructor");
    *(void **)&dtor = (void *)get("variant_get_ptr_destructor");
    if (!from || !to || !dtor) return 0;
    for (int t = 1; t < T_MAX; t++) {
        gd.from[t] = from((GDExtensionVariantType)t);
        gd.to[t] = to((GDExtensionVariantType)t);
        gd.dtor[t] = dtor((GDExtensionVariantType)t);
    }
    gd.bytes_resize = gd.ptr_method(GDEXTENSION_VARIANT_TYPE_PACKED_BYTE_ARRAY, SN("resize"), 848867239);
    if (!gd.bytes_resize) {
        fprintf(stderr, "goome: PackedByteArray.resize changed in this godot\n");
        return 0;
    }
    return 1;
}

typedef struct {
    const char *lit;
    gd_sn sn;
} sn_slot;

static sn_slot sn_tab[1024];

const gd_sn *SN(const char *lit)
{
    size_t h = ((uintptr_t)lit >> 3) & 1023;
    for (int i = 0; i < 1024; i++, h = (h + 1) & 1023) {
        if (sn_tab[h].lit == lit) return &sn_tab[h].sn;
        if (!sn_tab[h].lit) {
            sn_tab[h].lit = lit;
            gd.sn_new(&sn_tab[h].sn, lit, 1);
            return &sn_tab[h].sn;
        }
    }
    static gd_sn spare;
    gd.sn_new(&spare, lit, 0);
    return &spare;
}

void gd_err(const char *msg) { gd.print_error(msg, "goome", __FILE__, __LINE__, 0); }

void gd_var_nil(gd_var *v) { gd.nil(v); }

void gd_var_bool(gd_var *v, int b)
{
    GDExtensionBool x = b ? 1 : 0;
    gd.from[T_BOOL](v, &x);
}

void gd_var_int(gd_var *v, int64_t i) { gd.from[T_INT](v, &i); }

void gd_var_float(gd_var *v, double f) { gd.from[T_FLOAT](v, &f); }

void gd_var_cstr(gd_var *v, const char *s)
{
    gd_str str;
    gd.str_new(&str, s ? s : "", s ? (GDExtensionInt)strlen(s) : 0);
    gd.from[T_STRING](v, &str);
    gd.dtor[T_STRING](&str);
}

void gd_var_sn(gd_var *v, const char *lit) { gd.from[T_SN](v, (void *)SN(lit)); }

void gd_var_obj(gd_var *v, GDExtensionObjectPtr o)
{
    if (!o) {
        gd.nil(v);
        return;
    }
    gd.from[T_OBJECT](v, &o);
}

void gd_var_vec2(gd_var *v, gd_vec2 x) { gd.from[T_VEC2](v, &x); }

void gd_var_vec2i(gd_var *v, int x, int y)
{
    gd_vec2i p = {x, y};
    gd.from[T_VEC2I](v, &p);
}

void gd_var_rect2(gd_var *v, gd_rect2 r) { gd.from[T_RECT2](v, &r); }

void gd_var_rect2i(gd_var *v, int x, int y, int w, int h)
{
    gd_rect2i r = {x, y, w, h};
    gd.from[T_RECT2I](v, &r);
}

void gd_var_color(gd_var *v, gd_color c) { gd.from[T_COLOR](v, &c); }

void gd_var_free(gd_var *v)
{
    gd.destroy(v);
    gd.nil(v);
}

void gd_var_set(gd_var *dst, gd_var *src)
{
    gd.destroy(dst);
    memcpy(dst, src, sizeof(*dst));
    gd.nil(src);
}

int gd_is(const gd_var *v, int type) { return (int)gd.type(v) == type; }

int gd_convert(const gd_var *in, int type, gd_var *out)
{
    if ((int)gd.type(in) == type) {
        gd.copy(out, in);
        return 1;
    }
    GDExtensionCallError ce = {0};
    GDExtensionConstVariantPtr args[1] = {in};
    gd.construct((GDExtensionVariantType)type, out, args, 1, &ce);
    if (ce.error != GDEXTENSION_CALL_OK) {
        gd.construct((GDExtensionVariantType)type, out, NULL, 0, &ce);
        return 0;
    }
    return 1;
}

#define AS(T, ctype, zero)                 \
    ctype x = zero;                        \
    gd_var t;                              \
    gd_convert(v, T, &t);                  \
    gd.to[T](&x, &t);                      \
    gd.destroy(&t);                        \
    return x

int64_t gd_as_int(const gd_var *v) { AS(T_INT, int64_t, 0); }

double gd_as_float(const gd_var *v) { AS(T_FLOAT, double, 0); }

gd_vec2 gd_as_vec2(const gd_var *v) { AS(T_VEC2, gd_vec2, ((gd_vec2){0, 0})); }

gd_vec2i gd_as_vec2i(const gd_var *v) { AS(T_VEC2I, gd_vec2i, ((gd_vec2i){0, 0})); }

gd_rect2 gd_as_rect2(const gd_var *v) { AS(T_RECT2, gd_rect2, ((gd_rect2){0, 0, 0, 0})); }

gd_vec4 gd_as_vec4(const gd_var *v) { AS(T_VEC4, gd_vec4, ((gd_vec4){0, 0, 0, 0})); }
gd_color gd_as_color(const gd_var *v) { AS(T_COLOR, gd_color, ((gd_color){0, 0, 0, 0})); }

gd_xform2 gd_as_xform2(const gd_var *v) { AS(T_XFORM2, gd_xform2, ((gd_xform2){{1, 0, 0, 1, 0, 0}})); }

int gd_as_bool(const gd_var *v)
{
    GDExtensionBool x = 0;
    gd_var t;
    gd_convert(v, T_BOOL, &t);
    gd.to[T_BOOL](&x, &t);
    gd.destroy(&t);
    return x != 0;
}

GDExtensionObjectPtr gd_as_obj(const gd_var *v)
{
    if (!gd_is(v, T_OBJECT)) return NULL;
    GDExtensionObjectPtr o = NULL;
    gd.to[T_OBJECT](&o, (GDExtensionVariantPtr)v);
    return o;
}

char *gd_as_cstr(const gd_var *v)
{
    gd_var t;
    gd_convert(v, T_STRING, &t);
    gd_str s;
    gd.to[T_STRING](&s, &t);
    gd.destroy(&t);
    GDExtensionInt n = gd.str_utf8(&s, NULL, 0);
    char *out = malloc((size_t)n + 1);
    if (out) {
        gd.str_utf8(&s, out, n);
        out[n] = 0;
    }
    gd.dtor[T_STRING](&s);
    return out;
}

int gd_strs(const gd_var *v, char ***out)
{
    gd_var t, sz;
    gd_convert(v, T_STRINGS, &t);
    gd_call(&t, "size", &sz, 0);
    int n = (int)gd_as_int(&sz);
    gd.destroy(&sz);
    char **s = calloc(n > 0 ? (size_t)n : 1, sizeof(char *));
    for (int i = 0; s && i < n; i++) {
        gd_var e;
        GDExtensionBool ok = 0, oob = 0;
        gd.get_indexed(&t, i, &e, &ok, &oob);
        s[i] = ok ? gd_as_cstr(&e) : NULL;
        if (!s[i]) s[i] = calloc(1, 1);
        gd.destroy(&e);
    }
    gd.destroy(&t);
    *out = s;
    return s ? n : 0;
}

void gd_strs_free(char **s, int n)
{
    if (!s) return;
    for (int i = 0; i < n; i++) free(s[i]);
    free(s);
}

int gd_floats(const gd_var *v, float **out)
{
    gd_var t, sz;
    gd_convert(v, T_FLOATS, &t);
    gd_call(&t, "size", &sz, 0);
    int n = (int)gd_as_int(&sz);
    gd.destroy(&sz);
    float *f = calloc(n > 0 ? (size_t)n : 1, sizeof(float));
    for (int i = 0; f && i < n; i++) {
        gd_var e;
        GDExtensionBool ok = 0, oob = 0;
        gd.get_indexed(&t, i, &e, &ok, &oob);
        f[i] = ok ? (float)gd_as_float(&e) : 0;
        gd.destroy(&e);
    }
    gd.destroy(&t);
    *out = f;
    return f ? n : 0;
}

int gd_doubles(const gd_var *v, double **out)
{
    gd_var t, sz;
    gd_convert(v, T_DOUBLES, &t);
    gd_call(&t, "size", &sz, 0);
    int n = (int)gd_as_int(&sz);
    gd.destroy(&sz);
    double *d = calloc(n > 0 ? (size_t)n : 1, sizeof(double));
    for (int i = 0; d && i < n; i++) {
        gd_var e;
        GDExtensionBool ok = 0, oob = 0;
        gd.get_indexed(&t, i, &e, &ok, &oob);
        d[i] = ok ? gd_as_float(&e) : 0;
        gd.destroy(&e);
    }
    gd.destroy(&t);
    *out = d;
    return d ? n : 0;
}

int gd_vec2s(const gd_var *v, double **xy)
{
    gd_var t, sz;
    gd_convert(v, T_VEC2S, &t);
    gd_call(&t, "size", &sz, 0);
    int n = (int)gd_as_int(&sz);
    gd.destroy(&sz);
    double *d = calloc(n > 0 ? (size_t)n * 2 : 1, sizeof(double));
    for (int i = 0; d && i < n; i++) {
        gd_var e;
        GDExtensionBool ok = 0, oob = 0;
        gd.get_indexed(&t, i, &e, &ok, &oob);
        gd_vec2 p = ok ? gd_as_vec2(&e) : (gd_vec2){0, 0};
        d[i * 2] = p.x, d[i * 2 + 1] = p.y;
        gd.destroy(&e);
    }
    gd.destroy(&t);
    *xy = d;
    return d ? n : 0;
}

void gd_doubles_var(gd_var *v, const double *x, int n)
{
    GDExtensionCallError ce = {0};
    gd.construct((GDExtensionVariantType)T_DOUBLES, v, NULL, 0, &ce);
    gd_var sz;
    gd_var_int(&sz, n);
    gd_call(v, "resize", NULL, 1, &sz);
    gd.destroy(&sz);
    for (int i = 0; i < n; i++) {
        gd_var e;
        GDExtensionBool ok = 0, oob = 0;
        gd_var_float(&e, x[i]);
        gd.set_indexed(v, i, &e, &ok, &oob);
        gd.destroy(&e);
    }
}

void gd_dict_var(gd_var *v)
{
    GDExtensionCallError ce = {0};
    gd.construct((GDExtensionVariantType)T_DICT, v, NULL, 0, &ce);
}

void gd_dict_put(gd_var *d, const char *key, gd_var *val)
{
    gd_var k;
    GDExtensionBool ok = 0;
    gd_var_cstr(&k, key);
    gd.set(d, &k, val, &ok);
    gd.destroy(&k);
    gd.destroy(val);
}

int gd_call(gd_var *self, const char *method, gd_var *ret, int n, ...)
{
    GDExtensionConstVariantPtr args[8];
    va_list ap;
    va_start(ap, n);
    for (int i = 0; i < n && i < 8; i++) args[i] = va_arg(ap, gd_var *);
    va_end(ap);
    gd_var r;
    GDExtensionCallError ce = {0};
    gd.call(self, SN(method), args, n, &r, &ce);
    if (ret) memcpy(ret, &r, sizeof(r));
    else gd.destroy(&r);
    if (ce.error != GDEXTENSION_CALL_OK) {
        char msg[160];
        snprintf(msg, sizeof(msg), "goome: calling %s failed (error %d, argument %d)", method, (int)ce.error, (int)ce.argument);
        gd_err(msg);
        return 0;
    }
    return 1;
}

GDExtensionObjectPtr gd_new(const char *cls) { return gd.construct_object(SN(cls)); }

void gd_new_var(gd_var *v, const char *cls) { gd_var_obj(v, gd_new(cls)); }

void gd_bytes_var(gd_var *v, int64_t n, uint8_t **ptr)
{
    gd_packed raw;
    gd.ptr_ctor(GDEXTENSION_VARIANT_TYPE_PACKED_BYTE_ARRAY, 0)(&raw, NULL);
    int64_t got = 0;
    GDExtensionConstTypePtr args[1] = {&n};
    gd.bytes_resize(&raw, args, &got, 1);
    *ptr = n > 0 ? gd.bytes_at(&raw, 0) : NULL;
    gd.from[T_BYTES](v, &raw);
    gd.dtor[T_BYTES](&raw);
}

uint32_t argb_of(gd_color c)
{
    float f[4] = {c.a, c.r, c.g, c.b};
    uint32_t out = 0;
    for (int i = 0; i < 4; i++) {
        float x = f[i] < 0 ? 0 : f[i] > 1 ? 1 : f[i];
        out = (out << 8) | (uint32_t)(x * 255 + 0.5f);
    }
    return out;
}

gd_color color_of(uint32_t argb)
{
    return (gd_color){((argb >> 16) & 255) / 255.0f, ((argb >> 8) & 255) / 255.0f, (argb & 255) / 255.0f, (argb >> 24) / 255.0f};
}

static void default_of(gd_var *v, const gd_arg *a)
{
    const char *d = a->def;
    switch (a->type) {
    case T_BOOL: gd_var_bool(v, !strcmp(d, "true")); break;
    case T_INT: gd_var_int(v, strtoll(d, NULL, 0)); break;
    case T_FLOAT: gd_var_float(v, strtod(d, NULL)); break;
    case T_STRING: gd_var_cstr(v, d); break;
    case T_RECT2:
    case T_COLOR:
    case T_VEC2:
    case T_VEC2I: {
        float f[4] = {0, 0, 0, 0};
        char *e = (char *)d;
        for (int i = 0; i < 4 && *e; i++) {
            f[i] = strtof(e, &e);
            while (*e == ',' || *e == ' ') e++;
        }
        if (a->type == T_RECT2) gd_var_rect2(v, (gd_rect2){f[0], f[1], f[2], f[3]});
        else if (a->type == T_COLOR) gd_var_color(v, (gd_color){f[0], f[1], f[2], f[3]});
        else if (a->type == T_VEC2) gd_var_vec2(v, (gd_vec2){f[0], f[1]});
        else gd_var_vec2i(v, (int)f[0], (int)f[1]);
        break;
    }
    default: {
        GDExtensionCallError ce = {0};
        gd.construct((GDExtensionVariantType)a->type, v, NULL, 0, &ce);
    }
    }
}

static void zero_of(gd_var *v, int type)
{
    GDExtensionCallError ce = {0};
    if (type == T_VOID || type == T_NIL || type == T_OBJECT) gd.nil(v);
    else gd.construct((GDExtensionVariantType)type, v, NULL, 0, &ce);
}

static void run(gd_method *m, void *inst, gd_var *vals, gd_var *out)
{
    gd_args a;
    memset(&a, 0, sizeof(a));
    a.n = m->nargs;
    for (int i = 0; i < m->nargs; i++) {
        a.v[i] = &vals[i];
        if (m->args[i].type == T_STRING) a.s[i] = gd_as_cstr(&vals[i]);
    }
    zero_of(out, m->ret);
    m->fn(inst, &a, out);
    for (int i = 0; i < m->nargs; i++) free(a.s[i]);
}

static void method_call(void *ud, GDExtensionClassInstancePtr inst, const GDExtensionConstVariantPtr *args, GDExtensionInt argc, GDExtensionVariantPtr ret, GDExtensionCallError *err)
{
    gd_method *m = ud;
    if (argc > m->nargs) {
        err->error = GDEXTENSION_CALL_ERROR_TOO_MANY_ARGUMENTS;
        err->expected = m->nargs;
        return;
    }
    if (argc < m->nreq) {
        err->error = GDEXTENSION_CALL_ERROR_TOO_FEW_ARGUMENTS;
        err->expected = m->nreq;
        return;
    }
    gd_var vals[8], out;
    int i;
    for (i = 0; i < argc; i++) {
        int want = m->args[i].type;
        if (want == T_NIL) {
            gd.copy(&vals[i], args[i]);
            continue;
        }
        if (!gd_convert(args[i], want, &vals[i])) {
            for (int k = 0; k <= i; k++) gd.destroy(&vals[k]);
            err->error = GDEXTENSION_CALL_ERROR_INVALID_ARGUMENT;
            err->argument = i;
            err->expected = want;
            return;
        }
    }
    for (; i < m->nargs; i++) gd.copy(&vals[i], &m->defs[i]);
    run(m, inst, vals, &out);
    for (i = 0; i < m->nargs; i++) gd.destroy(&vals[i]);
    if (m->ret != T_VOID) {
        gd.destroy(ret);
        memcpy(ret, &out, sizeof(out));
    } else
        gd.destroy(&out);
}

static void method_ptrcall(void *ud, GDExtensionClassInstancePtr inst, const GDExtensionConstTypePtr *args, GDExtensionTypePtr ret)
{
    gd_method *m = ud;
    gd_var vals[8], out;
    for (int i = 0; i < m->nargs; i++) {
        int t = m->args[i].type;
        if (t == T_NIL) gd.copy(&vals[i], args[i]);
        else if (t == T_OBJECT) {
            GDExtensionObjectPtr o = *(GDExtensionObjectPtr const *)args[i];
            gd_var_obj(&vals[i], o);
        } else
            gd.from[t](&vals[i], (GDExtensionTypePtr)args[i]);
    }
    run(m, inst, vals, &out);
    for (int i = 0; i < m->nargs; i++) gd.destroy(&vals[i]);
    if (m->ret == T_VOID) {
        gd.destroy(&out);
        return;
    }
    if (m->ret == T_NIL) {
        gd.destroy(ret);
        memcpy(ret, &out, sizeof(out));
        return;
    }
    if (m->ret == T_OBJECT) {
        GDExtensionObjectPtr o = gd_as_obj(&out);
        if (m->refc) gd.ref_set(ret, o);
        else *(GDExtensionObjectPtr *)ret = o;
    } else if (m->ret == T_STRING || m->ret == T_DICT || m->ret == T_DOUBLES) {
        gd.dtor[m->ret](ret);
        gd.to[m->ret](ret, &out);
    } else
        gd.to[m->ret](ret, &out);
    gd.destroy(&out);
}

static GDExtensionPropertyInfo prop(int type, const char *name, const char *cls)
{
    static gd_str empty;
    static int made;
    if (!made) {
        gd.str_new(&empty, "", 0);
        made = 1;
    }
    GDExtensionPropertyInfo p;
    memset(&p, 0, sizeof(p));
    p.type = (GDExtensionVariantType)type;
    p.name = (GDExtensionStringNamePtr)SN(name ? name : "");
    p.class_name = (GDExtensionStringNamePtr)SN(cls ? cls : "");
    p.hint_string = &empty;
    p.usage = 6;
    return p;
}

void gd_register_methods(const char *cls, gd_method *list)
{
    for (gd_method *m = list; m->name; m++) {
        GDExtensionPropertyInfo args[8], rv;
        GDExtensionClassMethodArgumentMetadata meta[8];
        GDExtensionVariantPtr defs[8];
        m->nargs = 0;
        m->nreq = 0;
        while (m->nargs < 8 && m->args[m->nargs].name) m->nargs++;
        for (int i = 0; i < m->nargs; i++) {
            args[i] = prop(m->args[i].type, m->args[i].name, m->args[i].cls);
            if (m->args[i].type == T_NIL) args[i].usage = 6 | 131072;
            meta[i] = GDEXTENSION_METHOD_ARGUMENT_METADATA_NONE;
            if (m->args[i].def) default_of(&m->defs[i], &m->args[i]);
            else m->nreq = i + 1;
        }
        int nd = 0;
        for (int i = m->nreq; i < m->nargs; i++) defs[nd++] = &m->defs[i];
        rv = prop(m->ret == T_VOID ? T_NIL : m->ret, "", m->ret_cls);
        if (m->ret == T_NIL) rv.usage = 6 | 131072;
        GDExtensionClassMethodInfo info;
        memset(&info, 0, sizeof(info));
        info.name = (GDExtensionStringNamePtr)SN(m->name);
        info.method_userdata = m;
        info.call_func = method_call;
        info.ptrcall_func = method_ptrcall;
        info.method_flags = GDEXTENSION_METHOD_FLAGS_DEFAULT;
        info.has_return_value = m->ret != T_VOID;
        info.return_value_info = &rv;
        info.return_value_metadata = GDEXTENSION_METHOD_ARGUMENT_METADATA_NONE;
        info.argument_count = (uint32_t)m->nargs;
        info.arguments_info = args;
        info.arguments_metadata = meta;
        info.default_argument_count = (uint32_t)nd;
        info.default_arguments = defs;
        gd.register_method(gd.lib, SN(cls), &info);
    }
}

void gd_register_consts(const char *cls, const gd_const *c)
{
    for (; c->name; c++) gd.register_const(gd.lib, SN(cls), SN(c->group), SN(c->name), c->value, 1);
}

void gd_register_prop(const char *cls, const char *name, int type, const char *setter, const char *getter)
{
    GDExtensionPropertyInfo p = prop(type, name, NULL);
    gd.register_prop(gd.lib, SN(cls), &p, SN(setter), SN(getter));
}

void gd_register_signal(const char *cls, const char *name, const char *arg, const char *arg_cls)
{
    GDExtensionPropertyInfo p = prop(T_OBJECT, arg, arg_cls);
    gd.register_signal(gd.lib, SN(cls), SN(name), &p, 1);
}
