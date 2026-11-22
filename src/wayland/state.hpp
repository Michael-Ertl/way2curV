#pragma once

#include "curves/curve.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <fcntl.h>
#include <map>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

struct DisplayState; // Forward declaration
struct BufferPair;   // Forward declaration

struct WaylandGlobals {

    wl_display *display = nullptr;
    wl_registry *registry = nullptr;
    wl_compositor *compositor = nullptr;
    wl_shm *shm = nullptr;

    struct zwlr_layer_shell_v1 *layer_shell = nullptr;
    struct zwlr_layer_surface_v1 *layer_surface = nullptr;

    std::map<uint32_t, std::string> output_names; // registry id -> "DP-1"

    std::map<uint32_t, wl_output *>
        monitors; // Stores all discovered wl_outputs. This is
                  // updated/maintained by the listener function.
    std::map<uint32_t, DisplayState>
        displays; // This holds per monitor Displaystates. Gets made out of the
                  // monitors map.
    std::vector<Curve> curve_configs;
};





struct DisplayState {
    WaylandGlobals *wl_globals = nullptr;
    uint32_t surface_width = 1000;
    uint32_t surface_height = 1000;
    wl_callback *pending_frame = nullptr;
    zwlr_layer_surface_v1 *layer_surface = nullptr;
    wl_surface *surface = nullptr;
    wl_shm *shm = nullptr;
    Curve curve{};

    BufferPair *memory_buffers = nullptr;

    uint32_t output_name = 0;   // In wayland monitors have a number as a name
    std::string connector_name; // The name of the actual display like 'DP-1'
};


