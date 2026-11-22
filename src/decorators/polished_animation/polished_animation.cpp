#include "decorators/polished_animation/polished_animation.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

static std::uint32_t blend_color(std::uint32_t start, std::uint32_t end,
                                 double progress) {
    std::uint32_t color = 0;
    for (std::uint32_t shift : {0U, 8U, 16U}) {
        const double start_channel = (start >> shift) & 0xff;
        const double end_channel = (end >> shift) & 0xff;
        const auto channel = static_cast<std::uint32_t>(
            start_channel + (end_channel - start_channel) * progress + 0.5);
        color |= channel << shift;
    }
    return color;
}

void PolishedDecorator::place_on_background(Curve &) const {}

void PolishedDecorator::place_on_curve(Curve &) const {}

void PolishedDecorator::decorate(Curve &curve) const {
    const std::size_t point_count =
        std::min(curve.x_points.size(), curve.y_points.size());
    if (curve.pixels == nullptr || curve.width == 0 || curve.height == 0 ||
        point_count < 2 || curve.animation_length == 0) {
        return;
    }

    const double cycle_length = static_cast<double>(point_count);
    double position = std::fmod(curve.animated_index, cycle_length);
    if (position < 0.0) {
        position += cycle_length;
    }
    const double half_length = curve.animation_length / 2.0;
    const double span = std::min(half_length, cycle_length / 2.0);
    const int64_t first_point =
        static_cast<int64_t>(std::ceil(position - span));
    const int64_t last_point =
        static_cast<int64_t>(std::floor(position + span));

    struct HighlightPoint {
        std::size_t index;
        double intensity;
    };

    std::vector<HighlightPoint> highlight_points;
    highlight_points.reserve(
        static_cast<std::size_t>(last_point - first_point + 1));
    for (int64_t i = first_point; i <= last_point; ++i) {
        int64_t wrapped_index = i % static_cast<int64_t>(point_count);
        if (wrapped_index < 0) {
            wrapped_index += static_cast<int64_t>(point_count);
        }
        const double distance = std::abs(static_cast<double>(i) - position);
        const double progress = std::max(0.0, 1.0 - distance / half_length);
        const double intensity = progress * progress * (3.0 - 2.0 * progress);
        highlight_points.push_back(
            {static_cast<std::size_t>(wrapped_index), intensity});
    }

    std::sort(highlight_points.begin(), highlight_points.end(),
              [](const HighlightPoint &a, const HighlightPoint &b) {
                  return a.intensity < b.intensity;
              });

    const int64_t radius = curve.line_width / 2;
    for (const HighlightPoint &point : highlight_points) {
        const uint32_t color = blend_color(
            curve.line_color, curve.animation_color, point.intensity);
        const int64_t start_x =
            static_cast<int64_t>(curve.x_points[point.index]) - radius;
        const int64_t start_y =
            static_cast<int64_t>(curve.y_points[point.index]) - radius;
        const int64_t end_x = start_x + curve.line_width;
        const int64_t end_y = start_y + curve.line_width;

        for (int64_t y = start_y; y < end_y; ++y) {
            if (y < 0 || y >= static_cast<int64_t>(curve.height)) {
                continue;
            }

            for (int64_t x = start_x; x < end_x; ++x) {
                if (x < 0 || x >= static_cast<int64_t>(curve.width)) {
                    continue;
                }

                curve.pixels[static_cast<uint32_t>(y) * curve.width +
                             static_cast<uint32_t>(x)] = color;
            }
        }
    }
}
