#include "wayland/listener/layershell_listener.hpp"
#include "curves/curve.hpp"
#include "rendering/renderer.hpp"
#include "util/bufferpair.hpp"
#include "wayland/listener/frame_and_buffer_listeners.hpp"
#include "wayland/state.hpp"
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <wayland-client.h>
// The generated C protocol uses a C++ keyword as a parameter name.
#define namespace namespace_
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#undef namespace

static void handle_configure(void *data, struct zwlr_layer_surface_v1 *layer,
                             uint32_t serial, uint32_t width, uint32_t height);
static void handle_closed(void *data, struct zwlr_layer_surface_v1 *layer);

const struct zwlr_layer_surface_v1_listener layershell_listener = {
    .configure = handle_configure,
    .closed = handle_closed,
};

static void handle_configure(void *data, struct zwlr_layer_surface_v1 *layer,
                             uint32_t serial, uint32_t width, uint32_t height) {
    DisplayState &st = *static_cast<DisplayState *>(data);
    // ack must precede the commit in request_frame() that attaches the
    // new-size buffer
    zwlr_layer_surface_v1_ack_configure(layer, serial);
    if (width == 0 || height == 0) {
        // Cannot allocate buffers with zero dimensions, but the ack still
        // requires a commit.
        wl_surface_commit(st.surface);
        return;
    }

    // If eithers the buffers dont exist, or the display dimensions changed (new
    // settings and such) we need to redo our buffersetup and start a new chain
    // of events.
    if (!st.memory_buffers || st.memory_buffers->buffer_width != width ||
        st.memory_buffers->buffer_height != height) {
        // On resizing, we might need to destroy a currenltty active callback to
        // avoid a crash
        if (st.pending_frame) {
            wl_callback_destroy(st.pending_frame);
            st.pending_frame = nullptr;
        }

        BufferPair *temporary_buffer_pair;
        try {
            temporary_buffer_pair = new BufferPair(st, width, height);
        } catch (const std::exception &error) {
            fprintf(stderr, "Buffer creation failed: %s\n", error.what());
            return;
        }
        delete st.memory_buffers;
        st.memory_buffers = temporary_buffer_pair;
        st.surface_height = height;
        st.surface_width = width;
        st.curve.width = width;
        st.curve.height = height;
        // update_curve(st); //update is wrong here. We need a "firstrender" as
        // the update method does too much and handles business logic like etime
        // etc.. its almost similar but MUST ignore time constraints. Most
        // importantly, it must create the pixelmap, set the buffer maps
        // correctly
        st.memory_buffers->busy[0] = true;
        st.curve.pixels = st.memory_buffers->pixels[0];

        // We need to clear the cache after reconfiguring the display
        st.curve.x_points.clear();
        st.curve.y_points.clear();
        st.curve.image_cache.clear();

        render_wallpaper(st.curve);
        // As we just created these buffers, a request will be sent in the if
        // stmt below.
        assert(st.memory_buffers &&
               "memory_buffers must exist after successful creation at "
               "configuration of a monitor.");
    }

    // std::cout << st.surface_width << "|" << st.surface_height
    //           << "<-->width|height\n";
    // std::cout << st.curve.width << "|" << st.curve.height
    //           << "<-->width|height\n";

    if (st.memory_buffers) {
        request_frame(st);
    }
}

static void handle_closed(void *data, struct zwlr_layer_surface_v1 *layer) {
    DisplayState &st = *static_cast<DisplayState *>(data);
    if (st.pending_frame) {
        wl_callback_destroy(st.pending_frame);
        st.pending_frame = nullptr;
    }
    if (st.layer_surface) {
        zwlr_layer_surface_v1_destroy(st.layer_surface);
        st.layer_surface = nullptr;
    }
    delete st.memory_buffers;
    st.memory_buffers = nullptr;
    if (st.surface) {
        wl_surface_destroy(st.surface);
        st.surface = nullptr;
    }
}
