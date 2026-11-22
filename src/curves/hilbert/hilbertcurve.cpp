#include "hilbertcurve.hpp"
#include "util/utilities.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

void left(std::vector<uint32_t> *x_points, std::vector<uint32_t> *y_points,
          uint32_t depth, uint32_t x_s, uint32_t x_e, uint32_t y_s,
          uint32_t y_e) {
    if (depth > 0) {
        const uint32_t x_mid = x_s + (x_e - x_s) / 2;
        const uint32_t y_mid = y_s + (y_e - y_s) / 2;
        down(x_points, y_points, depth - 1, x_s, x_mid, y_mid, y_e);
        left(x_points, y_points, depth - 1, x_mid, x_e, y_mid, y_e);
        left(x_points, y_points, depth - 1, x_mid, x_e, y_s, y_mid);
        up(x_points, y_points, depth - 1, x_s, x_mid, y_s, y_mid);
        return;
    }

    const uint32_t x_l = x_s + (x_e - x_s) / 4;
    const uint32_t x_r = x_s + 3 * (x_e - x_s) / 4;
    const uint32_t y_u = y_s + (y_e - y_s) / 4;
    const uint32_t y_d = y_s + 3 * (y_e - y_s) / 4;

    x_points->insert(x_points->end(), {x_l, x_r, x_r, x_l});
    y_points->insert(y_points->end(), {y_d, y_d, y_u, y_u});
}

void right(std::vector<uint32_t> *x_points, std::vector<uint32_t> *y_points,
           uint32_t depth, uint32_t x_s, uint32_t x_e, uint32_t y_s,
           uint32_t y_e) {
    if (depth > 0) {
        const uint32_t x_mid = x_s + (x_e - x_s) / 2;
        const uint32_t y_mid = y_s + (y_e - y_s) / 2;
        up(x_points, y_points, depth - 1, x_mid, x_e, y_s, y_mid);
        right(x_points, y_points, depth - 1, x_s, x_mid, y_s, y_mid);
        right(x_points, y_points, depth - 1, x_s, x_mid, y_mid, y_e);
        down(x_points, y_points, depth - 1, x_mid, x_e, y_mid, y_e);
        return;
    }

    const uint32_t x_l = x_s + (x_e - x_s) / 4;
    const uint32_t x_r = x_s + 3 * (x_e - x_s) / 4;
    const uint32_t y_u = y_s + (y_e - y_s) / 4;
    const uint32_t y_d = y_s + 3 * (y_e - y_s) / 4;

    x_points->insert(x_points->end(), {x_r, x_l, x_l, x_r});
    y_points->insert(y_points->end(), {y_u, y_u, y_d, y_d});
}

void up(std::vector<uint32_t> *x_points, std::vector<uint32_t> *y_points,
        uint32_t depth, uint32_t x_s, uint32_t x_e, uint32_t y_s,
        uint32_t y_e) {
    if (depth > 0) {
        const uint32_t x_mid = x_s + (x_e - x_s) / 2;
        const uint32_t y_mid = y_s + (y_e - y_s) / 2;
        right(x_points, y_points, depth - 1, x_mid, x_e, y_s, y_mid);
        up(x_points, y_points, depth - 1, x_mid, x_e, y_mid, y_e);
        up(x_points, y_points, depth - 1, x_s, x_mid, y_mid, y_e);
        left(x_points, y_points, depth - 1, x_s, x_mid, y_s, y_mid);
        return;
    }

    const uint32_t x_l = x_s + (x_e - x_s) / 4;
    const uint32_t x_r = x_s + 3 * (x_e - x_s) / 4;
    const uint32_t y_u = y_s + (y_e - y_s) / 4;
    const uint32_t y_d = y_s + 3 * (y_e - y_s) / 4;

    x_points->insert(x_points->end(), {x_r, x_r, x_l, x_l});
    y_points->insert(y_points->end(), {y_u, y_d, y_d, y_u});
}

void down(std::vector<uint32_t> *x_points, std::vector<uint32_t> *y_points,
          uint32_t depth, uint32_t x_s, uint32_t x_e, uint32_t y_s,
          uint32_t y_e) {
    if (depth > 0) {
        const uint32_t x_mid = x_s + (x_e - x_s) / 2;
        const uint32_t y_mid = y_s + (y_e - y_s) / 2;
        left(x_points, y_points, depth - 1, x_s, x_mid, y_mid, y_e);
        down(x_points, y_points, depth - 1, x_s, x_mid, y_s, y_mid);
        down(x_points, y_points, depth - 1, x_mid, x_e, y_s, y_mid);
        right(x_points, y_points, depth - 1, x_mid, x_e, y_mid, y_e);
        return;
    }
    std::vector<uint32_t> cornerpoints;

    const uint32_t x_l = x_s + (x_e - x_s) / 4;
    const uint32_t x_r = x_s + 3 * (x_e - x_s) / 4;
    const uint32_t y_u = y_s + (y_e - y_s) / 4;
    const uint32_t y_d = y_s + 3 * (y_e - y_s) / 4;

    x_points->insert(x_points->end(), {x_l, x_l, x_r, x_r});
    y_points->insert(y_points->end(), {y_d, y_u, y_u, y_d});
}

void generate_hilbert_points(Curve &curve) {
    curve.x_points.clear();
    curve.y_points.clear();

    CurveBounds bounds = curve_bounds(curve);
    if (bounds.x_start >= bounds.x_end || bounds.y_start >= bounds.y_end) {
        return;
    }

    std::vector<uint32_t> corner_x_points;
    std::vector<uint32_t> corner_y_points;
    down(&corner_x_points, &corner_y_points, curve.depth, bounds.x_start,
         bounds.x_end, bounds.y_start, bounds.y_end);

    // Draw the lines
    for (std::size_t i = 0; i + 1 < corner_x_points.size(); ++i) {
        append_line_points(curve,
                           {corner_x_points[i], corner_y_points[i],
                            corner_x_points[i + 1], corner_y_points[i + 1]});
    }
}
