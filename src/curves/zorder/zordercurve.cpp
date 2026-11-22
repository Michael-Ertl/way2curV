#include "zordercurve.hpp"

#include "util/utilities.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

void z_order(std::vector<uint32_t> *x_points, std::vector<uint32_t> *y_points,
             uint32_t depth, uint32_t x_start, uint32_t x_end, uint32_t y_start,
             uint32_t y_end) {
    if (depth > 0) {
        const uint32_t x_mid = x_start + (x_end - x_start) / 2;
        const uint32_t y_mid = y_start + (y_end - y_start) / 2;
        z_order(x_points, y_points, depth - 1, x_start, x_mid, y_start, y_mid);
        z_order(x_points, y_points, depth - 1, x_mid, x_end, y_start, y_mid);
        z_order(x_points, y_points, depth - 1, x_start, x_mid, y_mid, y_end);
        z_order(x_points, y_points, depth - 1, x_mid, x_end, y_mid, y_end);
        return;
    }

    x_points->push_back(x_start + (x_end - x_start) / 2);
    y_points->push_back(y_start + (y_end - y_start) / 2);
}

void generate_z_order_points(Curve &curve) {
    curve.x_points.clear();
    curve.y_points.clear();

    const CurveBounds bounds = curve_bounds(curve);
    if (bounds.x_start >= bounds.x_end || bounds.y_start >= bounds.y_end) {
        return;
    }

    std::vector<uint32_t> control_x_points;
    std::vector<uint32_t> control_y_points;
    z_order(&control_x_points, &control_y_points, curve.depth, bounds.x_start,
            bounds.x_end, bounds.y_start, bounds.y_end);

    if (control_x_points.size() == 1) {
        curve.x_points = control_x_points;
        curve.y_points = control_y_points;
        return;
    }

    for (std::size_t i = 0; i + 1 < control_x_points.size(); ++i) {
        append_line_points(curve,
                           {control_x_points[i], control_y_points[i],
                            control_x_points[i + 1], control_y_points[i + 1]});
    }
}
