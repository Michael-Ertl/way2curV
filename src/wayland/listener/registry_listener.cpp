#include "wayland/listener/registry_listener.hpp"

#include "util/bufferpair.hpp"
#include "util/utilities.hpp"
#include "wayland/listener/layershell_listener.hpp"
#include "wayland/listener/output_listener.hpp"
#include <cstdint>
#include <cstring>
#include <wayland-client.h>
// The generated C protocol uses a C++ keyword as a parameter name.
#define namespace namespace_
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#undef namespace

static void registry_handle_global(void *data, struct wl_registry *registry,
                                   uint32_t name, const char *interface,
                                   uint32_t version) {

    // Compositor advertises a global.
    WaylandGlobals &globals = *static_cast<WaylandGlobals *>(data);
    if (std::strcmp(interface, wl_compositor_interface.name) == 0) {
        globals.compositor = static_cast<wl_compositor *>(
            wl_registry_bind(registry, name, &wl_compositor_interface, 4));
    }

    if (std::strcmp(interface, wl_shm_interface.name) == 0) {
        globals.shm = static_cast<wl_shm *>(
            wl_registry_bind(registry, name, &wl_shm_interface, 1));
    }

    if (std::strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
        globals.layer_shell =
            static_cast<zwlr_layer_shell_v1 *>(wl_registry_bind(
                registry, name, &zwlr_layer_shell_v1_interface, 1));
    }

    if (std::strcmp(interface, wl_output_interface.name) == 0) {
        wl_output *output = static_cast<wl_output *>(wl_registry_bind(
            registry, name, &wl_output_interface, std::min(version, 4u)));
        wl_output_add_listener(output, &output_listener,
                               &globals); // before the name event is dispatched
        globals.monitors.emplace(name, output);
    }

    // One critical bug previously: The order of advertisement for wayland
    // globals is NOT fixed by the protocol. Ergo, some of the things like
    // wl_shm which is used in the wl_output branch may NOT exist at that time
    // yet. Hence we need to avoid creating these display objects immediatly
    // (even though it works on probably most implementations, and instead
    // create the displays AFTER all necessary globals are active

    // IMPORTANT: The function below also registers to the layershell_listener
    // dynamically!
    // sync_displays(globals); // This function creates for any newly registered
                            // wl_outout a new object.

    // every time a global is advertised we need to recheck if we have all available gloabls to make the displays. If already present skip. Due to the fact that I am not sure about the order the globals are advertised in.
    if (globals.compositor && globals.shm && globals.layer_shell) {

        for (auto &[name, output] : globals.monitors) {
            if (!globals.displays.contains(name)) { // idempotent
                auto [it, inserted] = globals.displays.emplace(
                    name, DisplayState{}); // map node = stable address
                DisplayState &ds = it->second;
                ds.wl_globals = &globals;
                ds.output_name = name;
                ds.shm = globals.shm;
                // Notice: The curve config is omitted here. This will be set as
                // soon as the name event is happening.
                ds.surface = wl_compositor_create_surface(globals.compositor);
                ds.layer_surface = zwlr_layer_shell_v1_get_layer_surface(
                    globals.layer_shell, ds.surface, output,
                    ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND, way2curV);
                zwlr_layer_surface_v1_add_listener(ds.layer_surface,
                                                   &layershell_listener, &ds);
                zwlr_layer_surface_v1_set_anchor(
                    ds.layer_surface, ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
                                          ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |
                                          ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
                                          ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
                zwlr_layer_surface_v1_set_size(ds.layer_surface, 0, 0);
                zwlr_layer_surface_v1_set_exclusive_zone(
                    ds.layer_surface,
                    -1); // Apparrently necessary in order to enforce the
                         // wallpaper to not be affected by exclusive zones
                         // specified by other surfaces
                wl_surface_commit(ds.surface);
                wl_display_flush(globals.display);
            }
        }
    }; // can't build surfaces yet; retry next event
}

static void registry_handle_global_remove(void *data,
                                          struct wl_registry *registry,
                                          uint32_t name) {

    // We have to remove the thing form our monitors list as its unplugged or so
    WaylandGlobals &st = *static_cast<WaylandGlobals *>(data);
    auto monitor_it = st.monitors.find(name);
    if (monitor_it == st.monitors.end()) {
        return;
    };
    wl_output *output = monitor_it->second;
    st.monitors.erase(monitor_it);

    st.output_names.erase(name);
    wl_output_destroy(output);

    // Now we also have to detsroy the display object
    auto it = st.displays.find(name);
    if (it == st.displays.end()) {
        return;
    }

    DisplayState &ds = it->second;

    if (ds.pending_frame) {
        wl_callback_destroy(ds.pending_frame);
        ds.pending_frame = nullptr;
    }
    if (ds.layer_surface) {
        zwlr_layer_surface_v1_destroy(ds.layer_surface);
        ds.layer_surface = nullptr;
    }
    delete ds.memory_buffers;
    ds.memory_buffers = nullptr;
    if (ds.surface) {
        wl_surface_destroy(ds.surface);
        ds.surface = nullptr;
    }

    st.displays.erase(it);
}

const struct wl_registry_listener registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove,
};
