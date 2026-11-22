#include "utilities.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <stdlib.h>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

CurveBounds curve_bounds(const Curve &curve) {
    const uint32_t left_margin = std::min(curve.left_margin, curve.width);
    const uint32_t right_margin = std::min(curve.right_margin, curve.width);
    const uint32_t top_margin = std::min(curve.top_margin, curve.height);
    const uint32_t bottom_margin = std::min(curve.bottom_margin, curve.height);
    return {
        left_margin,
        curve.width - right_margin,
        top_margin,
        curve.height - bottom_margin,
    };
}

void append_line_points(Curve &curve, LineSegment line) {
    int32_t x = static_cast<int32_t>(line.start_x);
    int32_t y = static_cast<int32_t>(line.start_y);
    const int32_t end_x = static_cast<int32_t>(line.end_x);
    const int32_t end_y = static_cast<int32_t>(line.end_y);
    const int32_t dx = end_x >= x ? end_x - x : x - end_x;
    const int32_t dy = end_y >= y ? end_y - y : y - end_y;
    const int32_t step_x = x < end_x ? 1 : -1;
    const int32_t step_y = y < end_y ? 1 : -1;
    int32_t error = dx - dy;

    while (true) {
        if (y >= 0 && y < static_cast<int32_t>(curve.height) && x >= 0 &&
            x < static_cast<int32_t>(curve.width)) {
            const uint32_t point_x = static_cast<uint32_t>(x);
            const uint32_t point_y = static_cast<uint32_t>(y);
            if (curve.x_points.empty() || curve.x_points.back() != point_x ||
                curve.y_points.back() != point_y) {
                curve.x_points.push_back(point_x);
                curve.y_points.push_back(point_y);
            }
        }

        if (x == end_x && y == end_y) {
            break;
        }

        const int32_t doubled_error = error * 2;
        if (doubled_error > -dy) {
            error -= dy;
            x += step_x;
        }
        if (doubled_error < dx) {
            error += dx;
            y += step_y;
        }
    }
}

void draw_background(Curve &curve) {
    for (uint32_t y = 0; y < curve.height; ++y) {
        for (uint32_t x = 0; x < curve.width; ++x) {
            curve.pixels[y * curve.width + x] = curve.background_color;
        }
    }
}

void draw_curve(Curve &curve) {
    const std::size_t point_count =
        std::min(curve.x_points.size(), curve.y_points.size());
    if (curve.pixels == nullptr || curve.width == 0 || curve.height == 0 ||
        point_count == 0) {
        return;
    }

    const int64_t radius = curve.line_width / 2;
    const std::size_t stride = static_cast<std::size_t>(curve.width) + 1;
    std::vector<int32_t> coverage(stride *
                                  (static_cast<std::size_t>(curve.height) + 1));

    for (std::size_t i = 0; i < point_count; ++i) {
        const int64_t start_x =
            std::clamp(static_cast<int64_t>(curve.x_points[i]) - radius,
                       int64_t{0}, static_cast<int64_t>(curve.width));
        const int64_t start_y =
            std::clamp(static_cast<int64_t>(curve.y_points[i]) - radius,
                       int64_t{0}, static_cast<int64_t>(curve.height));
        const int64_t end_x = std::clamp(
            static_cast<int64_t>(curve.x_points[i]) - radius + curve.line_width,
            int64_t{0}, static_cast<int64_t>(curve.width));
        const int64_t end_y = std::clamp(
            static_cast<int64_t>(curve.y_points[i]) - radius + curve.line_width,
            int64_t{0}, static_cast<int64_t>(curve.height));
        if (start_x == end_x || start_y == end_y) {
            continue;
        }

        const std::size_t top_left =
            static_cast<std::size_t>(start_y) * stride + start_x;
        const std::size_t top_right =
            static_cast<std::size_t>(start_y) * stride + end_x;
        const std::size_t bottom_left =
            static_cast<std::size_t>(end_y) * stride + start_x;
        const std::size_t bottom_right =
            static_cast<std::size_t>(end_y) * stride + end_x;
        ++coverage[top_left];
        --coverage[top_right];
        --coverage[bottom_left];
        ++coverage[bottom_right];
    }

    for (std::size_t y = 0; y < curve.height; ++y) {
        int32_t row_coverage = 0;
        for (std::size_t x = 0; x < curve.width; ++x) {
            const std::size_t index = y * stride + x;
            row_coverage += coverage[index];
            coverage[index] =
                row_coverage + (y == 0 ? 0 : coverage[index - stride]);
            if (coverage[index] > 0) {
                curve.pixels[y * curve.width + x] = curve.line_color;
            }
        }
    }
}

