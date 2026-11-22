#include "wayland/listener/output_listener.hpp"

#include "wayland/state.hpp"
#include <cstdint>
#include <iostream>
#include <sys/types.h>
#include <wayland-client.h>

static void apply_curve_config(WaylandGlobals &g, uint32_t id) {
    // Attach the correct curve or a default
    bool found = 0;
    Curve default_c = {};
    DisplayState& ds = g.displays.at(id);
    for (auto &curve_config : g.curve_configs) {
        if (curve_config.monitor_name == g.output_names[id]) {
            // Now these displays match
            ds.curve = curve_config;
            found = true;
        }
        if (curve_config.monitor_name == "default") {
            default_c = curve_config;
        }
    }
    if (!found) {
        ds.curve = default_c;
    }
}

static void handle_output_geometry(void *, wl_output *, int32_t, int32_t,
                                   int32_t, int32_t, int32_t, const char *,
                                   const char *, int32_t) {}
static void handle_output_mode(void *, wl_output *, uint32_t, int32_t, int32_t,
                               int32_t) {}
static void handle_output_done(void *, wl_output *) {}
static void handle_output_scale(void *, wl_output *, int32_t) {}
static void handle_output_name(void *data, wl_output *output,
                               const char *name) {
    WaylandGlobals &g = *static_cast<WaylandGlobals *>(data);
    for (const auto &[id, candidate] : g.monitors) {
        if (candidate == output) {
            g.output_names[id] = name;
            apply_curve_config(g, id);
            return;
        }
    }
}
static void handle_output_description(void *, wl_output *, const char *) {}

// Holds a bunch of stubs i dont need. extract the monitor name only
const wl_output_listener output_listener = {
    .geometry = handle_output_geometry,
    .mode = handle_output_mode,
    .done = handle_output_done,
    .scale = handle_output_scale,
    .name = handle_output_name,
    .description = handle_output_description,
};
