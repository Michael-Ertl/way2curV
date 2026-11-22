#include "util/bufferpair.hpp"

#include "wayland/listener/frame_and_buffer_listeners.hpp"// buffer_listener
#include "wayland/shm_utils.hpp"
#include "wayland/state.hpp"

#include <cstddef>
#include <cstdio>
#include <format>
#include <limits>
#include <stdexcept>
#include <sys/mman.h>
#include <unistd.h>

constexpr std::size_t wayland_max =
    static_cast<std::size_t>(std::numeric_limits<int32_t>::max());

BufferPair::BufferPair(DisplayState &display_state, uint32_t requested_width,
                       uint32_t requested_height) {
    const size_t width = requested_width;
    const size_t height = requested_height;
    constexpr std::size_t bytes_per_pixel = 4;
    constexpr std::size_t buffer_count = 2;

    if (width == 0 || height == 0 || width > wayland_max / bytes_per_pixel ||
        height > (wayland_max / buffer_count) / (width * bytes_per_pixel)) {
        throw std::runtime_error(std::format(
            "Cannot create buffers with dimensions {} x {}", width, height));
    }

    const std::size_t stride = width * bytes_per_pixel;
    const std::size_t buffer_size = height * stride;
    const std::size_t shm_pool_size = buffer_size * buffer_count;

    int fd = allocate_shm_file(shm_pool_size);
    if (fd < 0) {
        throw std::runtime_error(
            std::format("Could not allocate shared-memory file\n"));
    }

    // set the members
    auto pool_data_l = static_cast<uint8_t *>(mmap(
        nullptr, shm_pool_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    if (pool_data_l == MAP_FAILED) {
        close(fd);
        throw std::runtime_error(
            std::format("mmap failed. Buffer setup failed."));
    }
    wl_shm_pool *pool_l =
        wl_shm_create_pool(display_state.shm, fd, shm_pool_size);
    close(fd);
    if (!pool_l) {
        munmap(pool_data_l, shm_pool_size);
        throw std::runtime_error(
            std::format("Could not create the shm pool!\n"));
    }

    // Now ensure the pixelmatrices and the wl_buffer objects are made twice

    uint32_t *pixels_l[2] = {nullptr, nullptr};
    wl_buffer *wl_shm_buffers_l[2] = {nullptr, nullptr};

    auto cleanup_locals = [&] {
        for (int j = 0; j < 2; ++j) {
            if (wl_shm_buffers_l[j]) {
                wl_buffer_destroy(wl_shm_buffers_l[j]);
            }
        }
        wl_shm_pool_destroy(pool_l);
        munmap(pool_data_l, shm_pool_size);
    }; // this function destructs the buffers
    
    for (int i = 0; i < 2; ++i) {
        const std::size_t offset =
            height * stride * static_cast<std::size_t>(i);

        pixels_l[i] = reinterpret_cast<uint32_t *>(pool_data_l + offset);
        wl_shm_buffers_l[i] = wl_shm_pool_create_buffer(
            pool_l, offset, width, height, stride, WL_SHM_FORMAT_XRGB8888);
        if (!wl_shm_buffers_l[i]) {
            cleanup_locals();
            throw std::runtime_error(
                std::format("Could not create shared-memory buffer\n"));
        }

        // Now attach the listener. Destruct if this doesnt work
        if (wl_buffer_add_listener(wl_shm_buffers_l[i], &buffer_listener,
                                   &display_state) != 0) {
            cleanup_locals();
            throw std::runtime_error(
                "Could not add shared-memory buffer listener");
        }
    }
    wl_shm_pool_destroy(pool_l);

    this->pool_size = shm_pool_size;
    this->buffer_width = width;
    this->buffer_height = height;
    this->pool_data = pool_data_l;
    for (int i = 0; i < 2; ++i) {
        pixels[i] = pixels_l[i];
        wl_shm_buffers[i] = wl_shm_buffers_l[i];
    }
}

BufferPair::~BufferPair() {
    for (int i = 0; i < 2; ++i) {
        if (this->wl_shm_buffers[i]) {
            wl_buffer_destroy(this->wl_shm_buffers[i]);
            this->wl_shm_buffers[i] = nullptr;
        }
    }

    if (this->pool_data) {
        munmap(this->pool_data, this->pool_size);
        this->pool_data = nullptr;
    }
    this->pool_size = 0;
    this->buffer_width = 0;
    this->buffer_height = 0;
}