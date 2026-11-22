#pragma once

#include "wayland/state.hpp"
#include <wayland-client.h>

void update_curve(DisplayState &state);
void request_frame(DisplayState &state);

extern const wl_callback_listener frame_listener;
extern const wl_buffer_listener buffer_listener;
