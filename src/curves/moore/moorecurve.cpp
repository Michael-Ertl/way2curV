#include "moorecurve.hpp"
#include "util/utilities.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

void left_m(std::vector<uint32_t> *x_points, std::vector<uint32_t> *y_points,
            uint32_t depth, uint32_t x_s, uint32_t x_e, uint32_t y_s,
            uint32_t y_e);
void right_m(std::vector<uint32_t> *x_points, std::vector<uint32_t> *y_points,
             uint32_t depth, uint32_t x_s, uint32_t x_e, uint32_t y_s,
             uint32_t y_e);
void up_m(std::vector<uint32_t> *x_points, std::vector<uint32_t> *y_points,
          uint32_t depth, uint32_t x_s, uint32_t x_e, uint32_t y_s,
          uint32_t y_e);
void down_m(std::vector<uint32_t> *x_points, std::vector<uint32_t> *y_points,
            uint32_t depth, uint32_t x_s, uint32_t x_e, uint32_t y_s,
            uint32_t y_e);

void left_m(std::vector<uint32_t> *x_points, std::vector<uint32_t> *y_points,
            uint32_t depth, uint32_t x_s, uint32_t x_e, uint32_t y_s,
            uint32_t y_e) {
    if (depth > 0) {
        const uint32_t x_mid = x_s + (x_e - x_s) / 2;
        const uint32_t y_mid = y_s + (y_e - y_s) / 2;
        down_m(x_points, y_points, depth - 1, x_s, x_mid, y_mid, y_e);
        left_m(x_points, y_points, depth - 1, x_mid, x_e, y_mid, y_e);
        left_m(x_points, y_points, depth - 1, x_mid, x_e, y_s, y_mid);
        up_m(x_points, y_points, depth - 1, x_s, x_mid, y_s, y_mid);
        return;
    }

    const uint32_t x_l = x_s + (x_e - x_s) / 4;
    const uint32_t x_r = x_s + 3 * (x_e - x_s) / 4;
    const uint32_t y_u = y_s + (y_e - y_s) / 4;
    const uint32_t y_d = y_s + 3 * (y_e - y_s) / 4;

    x_points->insert(x_points->end(), {x_l, x_r, x_r, x_l});
    y_points->insert(y_points->end(), {y_d, y_d, y_u, y_u});
}

void right_m(std::vector<uint32_t> *x_points, std::vector<uint32_t> *y_points,
             uint32_t depth, uint32_t x_s, uint32_t x_e, uint32_t y_s,
             uint32_t y_e) {
    if (depth > 0) {
        const uint32_t x_mid = x_s + (x_e - x_s) / 2;
        const uint32_t y_mid = y_s + (y_e - y_s) / 2;
        up_m(x_points, y_points, depth - 1, x_mid, x_e, y_s, y_mid);
        right_m(x_points, y_points, depth - 1, x_s, x_mid, y_s, y_mid);
        right_m(x_points, y_points, depth - 1, x_s, x_mid, y_mid, y_e);
        down_m(x_points, y_points, depth - 1, x_mid, x_e, y_mid, y_e);
        return;
    }

    const uint32_t x_l = x_s + (x_e - x_s) / 4;
    const uint32_t x_r = x_s + 3 * (x_e - x_s) / 4;
    const uint32_t y_u = y_s + (y_e - y_s) / 4;
    const uint32_t y_d = y_s + 3 * (y_e - y_s) / 4;

    x_points->insert(x_points->end(), {x_r, x_l, x_l, x_r});
    y_points->insert(y_points->end(), {y_u, y_u, y_d, y_d});
}

void up_m(std::vector<uint32_t> *x_points, std::vector<uint32_t> *y_points,
          uint32_t depth, uint32_t x_s, uint32_t x_e, uint32_t y_s,
          uint32_t y_e) {
    if (depth > 0) {
        const uint32_t x_mid = x_s + (x_e - x_s) / 2;
        const uint32_t y_mid = y_s + (y_e - y_s) / 2;
        right_m(x_points, y_points, depth - 1, x_mid, x_e, y_s, y_mid);
        up_m(x_points, y_points, depth - 1, x_mid, x_e, y_mid, y_e);
        up_m(x_points, y_points, depth - 1, x_s, x_mid, y_mid, y_e);
        left_m(x_points, y_points, depth - 1, x_s, x_mid, y_s, y_mid);
        return;
    }

    const uint32_t x_l = x_s + (x_e - x_s) / 4;
    const uint32_t x_r = x_s + 3 * (x_e - x_s) / 4;
    const uint32_t y_u = y_s + (y_e - y_s) / 4;
    const uint32_t y_d = y_s + 3 * (y_e - y_s) / 4;

    x_points->insert(x_points->end(), {x_r, x_r, x_l, x_l});
    y_points->insert(y_points->end(), {y_u, y_d, y_d, y_u});
}

void down_m(std::vector<uint32_t> *x_points, std::vector<uint32_t> *y_points,
            uint32_t depth, uint32_t x_s, uint32_t x_e, uint32_t y_s,
            uint32_t y_e) {
    if (depth > 0) {
        const uint32_t x_mid = x_s + (x_e - x_s) / 2;
        const uint32_t y_mid = y_s + (y_e - y_s) / 2;
        left_m(x_points, y_points, depth - 1, x_s, x_mid, y_mid, y_e);
        down_m(x_points, y_points, depth - 1, x_s, x_mid, y_s, y_mid);
        down_m(x_points, y_points, depth - 1, x_mid, x_e, y_s, y_mid);
        right_m(x_points, y_points, depth - 1, x_mid, x_e, y_mid, y_e);
        return;
    }
    const uint32_t x_l = x_s + (x_e - x_s) / 4;
    const uint32_t x_r = x_s + 3 * (x_e - x_s) / 4;
    const uint32_t y_u = y_s + (y_e - y_s) / 4;
    const uint32_t y_d = y_s + 3 * (y_e - y_s) / 4;

    x_points->insert(x_points->end(), {x_l, x_l, x_r, x_r});
    y_points->insert(y_points->end(), {y_d, y_u, y_u, y_d});
}

void generate_moore_points(Curve &curve) {
    curve.x_points.clear();
    curve.y_points.clear();

    CurveBounds bounds = curve_bounds(curve);
    if (bounds.x_start >= bounds.x_end || bounds.y_start >= bounds.y_end) {
        return;
    }

    std::vector<uint32_t> corner_x_points;
    std::vector<uint32_t> corner_y_points;
    if (curve.depth == 0) {
        down_m(&corner_x_points, &corner_y_points, 0, bounds.x_start,
               bounds.x_end, bounds.y_start, bounds.y_end);
    } else {
        const uint32_t x_mid =
            bounds.x_start + (bounds.x_end - bounds.x_start) / 2;
        const uint32_t y_mid =
            bounds.y_start + (bounds.y_end - bounds.y_start) / 2;
        right_m(&corner_x_points, &corner_y_points, curve.depth - 1,
                bounds.x_start, x_mid, bounds.y_start, y_mid);
        right_m(&corner_x_points, &corner_y_points, curve.depth - 1,
                bounds.x_start, x_mid, y_mid, bounds.y_end);
        left_m(&corner_x_points, &corner_y_points, curve.depth - 1, x_mid,
               bounds.x_end, y_mid, bounds.y_end);
        left_m(&corner_x_points, &corner_y_points, curve.depth - 1, x_mid,
               bounds.x_end, bounds.y_start, y_mid);
    }

    // Draw the lines
    for (std::size_t i = 0; i + 1 < corner_x_points.size(); ++i) {
        append_line_points(curve,
                           {corner_x_points[i], corner_y_points[i],
                            corner_x_points[i + 1], corner_y_points[i + 1]});
    }

    // Close the curve
    append_line_points(curve,
                       {corner_x_points.back(), corner_y_points.back(),
                        corner_x_points.front(), corner_y_points.front()});
}
