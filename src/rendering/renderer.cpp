#include "rendering/renderer.hpp"
#include "curves/curve.hpp"
#include "util/utilities.hpp"
#include <cstring>
#define STB_IMAGE_IMPLEMENTATION
#include "rendering/vendor/stb_image.h"

void render_image(Curve &c) {
    int w, h, n;
    unsigned char *data = stbi_load(c.image_path.c_str(), &w, &h, &n, 4);
    if (!data || c.pixels == nullptr || c.width == 0 || c.height == 0)
        return;
    for (uint32_t y = 0; y < c.height; ++y)
        for (uint32_t x = 0; x < c.width; ++x) {
            const int sx = x * w / c.width,
                      sy = y * h / c.height; // nearest-neighbor stretch
            const unsigned char *p = data + 4 * (sy * w + sx);
            c.pixels[y * c.width + x] = 0xFF000000u | (uint32_t(p[0]) << 16) |
                                        (uint32_t(p[1]) << 8) | p[2];
        }
    stbi_image_free(data);
}

void render_wallpaper(Curve &curve) {
    const std::size_t pixel_count = std::size_t(curve.width) * curve.height;
    if (!curve.image_path.empty()) {
        if (curve.image_cache.size() != pixel_count) {
            render_image(curve);
            curve.image_cache.resize(pixel_count);
            std::memcpy(curve.image_cache.data(), curve.pixels,
                        pixel_count * sizeof(uint32_t));
        } else {
            std::memcpy(curve.pixels, curve.image_cache.data(),
                        pixel_count * sizeof(uint32_t));
        }
    } else {
        draw_background(curve);
    }
    render_curve(curve);
}