#include "gd.h"

#if defined(_WIN32)
#define GOOME_GD_EXPORT __declspec(dllexport)
#else
#define GOOME_GD_EXPORT __attribute__((visibility("default")))
#endif

static void init(void *ud, GDExtensionInitializationLevel level)
{
    if (level != GDEXTENSION_INITIALIZATION_SCENE) return;
    ui_register();
    view_register();
}

static void deinit(void *ud, GDExtensionInitializationLevel level)
{
    if (level != GDEXTENSION_INITIALIZATION_SCENE) return;
    gd.unregister_class(gd.lib, SN("GoomeView"));
    gd.unregister_class(gd.lib, SN("GoomeUI"));
}

GOOME_GD_EXPORT GDExtensionBool goome_gd_init(GDExtensionInterfaceGetProcAddress get, GDExtensionClassLibraryPtr lib, GDExtensionInitialization *r)
{
    if (!gd_load(get, lib)) return 0;
    r->initialize = init;
    r->deinitialize = deinit;
    r->userdata = NULL;
    r->minimum_initialization_level = GDEXTENSION_INITIALIZATION_SCENE;
    return 1;
}
