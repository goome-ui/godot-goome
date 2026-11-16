#ifndef GOOME_GD_H
#define GOOME_GD_H

#include <stddef.h>
#include <stdint.h>
#include "gdextension_interface.h"

typedef struct {
    uint64_t b[3];
} gd_var;

typedef struct {
    void *p;
} gd_sn;

typedef struct {
    void *p;
} gd_str;

typedef struct {
    void *p[2];
} gd_packed;

typedef struct {
    float x, y;
} gd_vec2;

typedef struct {
    float x, y, w, h;
} gd_rect2;

typedef struct {
    int32_t x, y;
} gd_vec2i;

typedef struct {
    int32_t x, y, w, h;
} gd_rect2i;

typedef struct {
    float x, y, z, w;
} gd_vec4;

typedef struct {
    float r, g, b, a;
} gd_color;

typedef struct {
    float m[6];
} gd_xform2;

enum {
    T_VOID = -1,
    T_NIL = GDEXTENSION_VARIANT_TYPE_NIL,
    T_BOOL = GDEXTENSION_VARIANT_TYPE_BOOL,
    T_INT = GDEXTENSION_VARIANT_TYPE_INT,
    T_FLOAT = GDEXTENSION_VARIANT_TYPE_FLOAT,
    T_STRING = GDEXTENSION_VARIANT_TYPE_STRING,
    T_VEC2 = GDEXTENSION_VARIANT_TYPE_VECTOR2,
    T_VEC2I = GDEXTENSION_VARIANT_TYPE_VECTOR2I,
    T_RECT2 = GDEXTENSION_VARIANT_TYPE_RECT2,
    T_RECT2I = GDEXTENSION_VARIANT_TYPE_RECT2I,
    T_XFORM2 = GDEXTENSION_VARIANT_TYPE_TRANSFORM2D,
    T_VEC4 = GDEXTENSION_VARIANT_TYPE_VECTOR4,
    T_COLOR = GDEXTENSION_VARIANT_TYPE_COLOR,
    T_SN = GDEXTENSION_VARIANT_TYPE_STRING_NAME,
    T_OBJECT = GDEXTENSION_VARIANT_TYPE_OBJECT,
    T_DICT = GDEXTENSION_VARIANT_TYPE_DICTIONARY,
    T_ARRAY = GDEXTENSION_VARIANT_TYPE_ARRAY,
    T_BYTES = GDEXTENSION_VARIANT_TYPE_PACKED_BYTE_ARRAY,
    T_FLOATS = GDEXTENSION_VARIANT_TYPE_PACKED_FLOAT32_ARRAY,
    T_DOUBLES = GDEXTENSION_VARIANT_TYPE_PACKED_FLOAT64_ARRAY,
    T_STRINGS = GDEXTENSION_VARIANT_TYPE_PACKED_STRING_ARRAY,
    T_VEC2S = GDEXTENSION_VARIANT_TYPE_PACKED_VECTOR2_ARRAY,
    T_MAX = GDEXTENSION_VARIANT_TYPE_VARIANT_MAX,
};

typedef struct {
    GDExtensionClassLibraryPtr lib;
    GDExtensionInterfaceGetProcAddress get;
    GDExtensionInterfaceVariantCall call;
    GDExtensionInterfaceVariantNewCopy copy;
    GDExtensionInterfaceVariantNewNil nil;
    GDExtensionInterfaceVariantDestroy destroy;
    GDExtensionInterfaceVariantGetType type;
    GDExtensionInterfaceVariantConstruct construct;
    GDExtensionInterfaceVariantGetIndexed get_indexed;
    GDExtensionInterfaceVariantSetIndexed set_indexed;
    GDExtensionInterfaceVariantSet set;
    GDExtensionInterfaceVariantGetPtrConstructor ptr_ctor;
    GDExtensionInterfaceVariantGetPtrBuiltinMethod ptr_method;
    GDExtensionInterfaceStringNewWithUtf8CharsAndLen2 str_new;
    GDExtensionInterfaceStringToUtf8Chars str_utf8;
    GDExtensionInterfaceStringNameNewWithLatin1Chars sn_new;
    GDExtensionInterfacePackedByteArrayOperatorIndex bytes_at;
    GDExtensionInterfaceGlobalGetSingleton singleton;
    GDExtensionInterfaceClassdbConstructObject2 construct_object;
    GDExtensionInterfaceObjectSetInstance set_instance;
    GDExtensionInterfaceObjectDestroy object_destroy;
    GDExtensionInterfaceRefGetObject ref_get;
    GDExtensionInterfaceRefSetObject ref_set;
    GDExtensionInterfaceClassdbRegisterExtensionClass4 register_class;
    GDExtensionInterfaceClassdbRegisterExtensionClassMethod register_method;
    GDExtensionInterfaceClassdbRegisterExtensionClassIntegerConstant register_const;
    GDExtensionInterfaceClassdbRegisterExtensionClassProperty register_prop;
    GDExtensionInterfaceClassdbRegisterExtensionClassSignal register_signal;
    GDExtensionInterfaceClassdbUnregisterExtensionClass unregister_class;
    GDExtensionInterfacePrintError print_error;
    GDExtensionInterfacePrintWarning print_warning;
    GDExtensionVariantFromTypeConstructorFunc from[T_MAX];
    GDExtensionTypeFromVariantConstructorFunc to[T_MAX];
    GDExtensionPtrDestructor dtor[T_MAX];
    GDExtensionPtrBuiltInMethod bytes_resize;
} gd_api;

extern gd_api gd;

int gd_load(GDExtensionInterfaceGetProcAddress get, GDExtensionClassLibraryPtr lib);
const gd_sn *SN(const char *lit);
void gd_err(const char *msg);

void gd_var_nil(gd_var *v);
void gd_var_bool(gd_var *v, int b);
void gd_var_int(gd_var *v, int64_t i);
void gd_var_float(gd_var *v, double f);
void gd_var_cstr(gd_var *v, const char *s);
void gd_var_sn(gd_var *v, const char *lit);
void gd_var_obj(gd_var *v, GDExtensionObjectPtr o);
void gd_var_vec2(gd_var *v, gd_vec2 x);
void gd_var_vec2i(gd_var *v, int x, int y);
void gd_var_rect2(gd_var *v, gd_rect2 r);
void gd_var_rect2i(gd_var *v, int x, int y, int w, int h);
void gd_var_color(gd_var *v, gd_color c);
void gd_var_free(gd_var *v);
void gd_var_set(gd_var *dst, gd_var *src);

int gd_is(const gd_var *v, int type);
int gd_convert(const gd_var *in, int type, gd_var *out);
int64_t gd_as_int(const gd_var *v);
double gd_as_float(const gd_var *v);
int gd_as_bool(const gd_var *v);
gd_vec2 gd_as_vec2(const gd_var *v);
gd_vec2i gd_as_vec2i(const gd_var *v);
gd_rect2 gd_as_rect2(const gd_var *v);
gd_vec4 gd_as_vec4(const gd_var *v);
gd_color gd_as_color(const gd_var *v);
gd_xform2 gd_as_xform2(const gd_var *v);
GDExtensionObjectPtr gd_as_obj(const gd_var *v);
char *gd_as_cstr(const gd_var *v);
int gd_strs(const gd_var *v, char ***out);
void gd_strs_free(char **s, int n);
int gd_floats(const gd_var *v, float **out);
int gd_doubles(const gd_var *v, double **out);
int gd_vec2s(const gd_var *v, double **xy);
void gd_doubles_var(gd_var *v, const double *x, int n);
void gd_dict_var(gd_var *v);
void gd_dict_put(gd_var *d, const char *key, gd_var *val);
void gd_array_var(gd_var *v);
void gd_array_push(gd_var *a, gd_var *val);

int gd_call(gd_var *self, const char *method, gd_var *ret, int n, ...);
GDExtensionObjectPtr gd_new(const char *cls);
void gd_new_var(gd_var *v, const char *cls);
void gd_bytes_var(gd_var *v, int64_t n, uint8_t **ptr);

uint32_t argb_of(gd_color c);
gd_color color_of(uint32_t argb);

typedef struct {
    const gd_var *v[8];
    char *s[8];
    int n;
} gd_args;

typedef void (*gd_impl)(void *inst, gd_args *a, gd_var *ret);

typedef struct {
    const char *name;
    int type;
    const char *def;
    const char *cls;
} gd_arg;

typedef struct {
    const char *name;
    gd_impl fn;
    int ret;
    const char *ret_cls;
    gd_arg args[8];
    int refc;
    int nargs, nreq;
    gd_var defs[8];
} gd_method;

typedef struct {
    int64_t value;
    const char *name;
    const char *group;
} gd_const;

void gd_register_methods(const char *cls, gd_method *m);
void gd_register_consts(const char *cls, const gd_const *c);
void gd_register_prop(const char *cls, const char *name, int type, const char *setter, const char *getter);
void gd_register_signal(const char *cls, const char *name, const char *arg, const char *arg_cls);
void gd_register_signal_args(const char *cls, const char *name, int n, const char *const *args, const int *types);

void ui_register(void);
void view_register(void);

#endif
