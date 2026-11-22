#include "wayland/listener/frame_and_buffer_listeners.hpp"
#include "rendering/renderer.hpp"
#include "util/bufferpair.hpp"

static void handle_frame(void *data, wl_callback *callback, uint32_t time);
static void handle_buffer_release(void *data, wl_buffer *buffer);

const wl_callback_listener frame_listener = {
    .done = handle_frame,
};
const wl_buffer_listener buffer_listener = {
    .release = handle_buffer_release,
};


void update_curve(DisplayState &state) {

    auto current = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        current - state.curve.last_update);

    if (elapsed < state.curve.time_step) {
        return;
    }

    const int candidate = (state.memory_buffers->activeIdx + 1) % 2;
    if (state.memory_buffers->busy[candidate]) {
        return; // dont make a new one, wait for the done event
    }
    state.memory_buffers->activeIdx = candidate;
    state.memory_buffers->busy[candidate] = true;

    state.curve.pixels = state.memory_buffers->pixels[candidate];
    state.curve.animated_index =
        state.curve.animated_index + state.curve.step_size;
    render_wallpaper(state.curve);
    state.curve.last_update = current; // Dont forget to update time
}

void request_frame(DisplayState &state) {
    // the last two zeroes are the x and y coordinates for the rectangle. They
    // are zero because we anchor our background at the origin at the top left
    // corner.
    if (!(state.pending_frame == nullptr)) {
        // There is still an active commit that didnt finish yet. So stop and do
        // nothing. If it finishes it will call the listener on the .done event
        // which calls this function, requesting the next frame
        return;
    }

    wl_surface_attach(
        state.surface,
        state.memory_buffers->wl_shm_buffers[state.memory_buffers->activeIdx],
        0, 0);
    wl_surface_damage(state.surface, 0, 0, state.surface_width,
                      state.surface_height);

    state.pending_frame = wl_surface_frame(state.surface);
    wl_callback_add_listener(state.pending_frame, &frame_listener, &state);

    wl_surface_commit(state.surface);
    wl_display_flush(state.wl_globals->display);
}

static void handle_frame(void *data, wl_callback *callback, uint32_t time) {
    DisplayState &st = *static_cast<DisplayState *>(data);
    if (st.surface == nullptr) {
        // Surface closed; the callback proxy was already destroyed in
        // handle_closed. Do not touch it again.
        return;
    }
    wl_callback_destroy(callback);
    st.pending_frame = nullptr;
    update_curve(st);
    request_frame(st);
}

static void handle_buffer_release(void *data, wl_buffer *buffer) {
    DisplayState &st = *static_cast<DisplayState *>(data);
    if (st.memory_buffers == nullptr) {
        return; // buffers torn down (surface closed or resized)
    }
    BufferPair &bp = *st.memory_buffers;

    for (int i = 0; i < 2; ++i) {
        if (bp.wl_shm_buffers[i] != buffer) {
            continue;
        }

        bp.busy[i] = false; // storage is writable again; NOT a draw trigger
        return;
    }
}
