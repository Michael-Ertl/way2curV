#pragma once

#include <cstdint>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h> // wl_buffer

struct DisplayState; // Forward declaration

struct BufferPair {
    int activeIdx = 0; // Indicates which canvas is currently written/active
    bool busy[2] = {false, false}; // Indicates which buffers are still in use
                                   // by the compositor
    uint32_t *pixels[2] = {nullptr, nullptr}; // CPU-side pixel views into the
                                              // shm pool
    wl_buffer *wl_shm_buffers[2] = {nullptr, nullptr}; // Wayland buffer handles

    uint8_t *pool_data = nullptr;
    uint32_t buffer_width = 0;
    uint32_t buffer_height = 0;
    std::size_t pool_size = 0; // bytes; needed for munmap

    BufferPair(DisplayState &display_state, uint32_t requested_width,
                       uint32_t requested_height);
    ~BufferPair();
};
