#include "curves/curve.hpp"
#include "util/config.hpp"
#include "wayland/listener/registry_listener.hpp"
#include <cstdio>
#include <vector>
#include <wayland-client.h>

int main(int argc, char *argv[]) {
    std::vector<Curve> curve_configs;
    int config_result = configuration(curve_configs);
    if (config_result != 0) {
        return config_result;
    }
    WaylandGlobals globals;
    globals.curve_configs =
        curve_configs; // Store all configfile curves in the state

    globals.display = wl_display_connect(nullptr);
    if (globals.display == nullptr) {
        return -1; // failed to connect
    }
    std::fprintf(stderr, "Connected");

    globals.registry = wl_display_get_registry(globals.display);
    wl_registry_add_listener(globals.registry, &registry_listener, &globals);
    wl_display_roundtrip(globals.display);

    // Event loop
    while (wl_display_dispatch(globals.display) != -1) {
        /* This space deliberately left blank */
    }

    wl_display_disconnect(globals.display);
    return 0;
}
