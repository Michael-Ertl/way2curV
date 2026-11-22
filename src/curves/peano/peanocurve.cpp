#include "peanocurve.hpp"

#include "util/utilities.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

struct Bounds {
    uint32_t x_start;
    uint32_t x_end;
    uint32_t y_start;
    uint32_t y_end;
};

void x(std::vector<uint32_t> *x_points, std::vector<uint32_t> *y_points,
       uint32_t depth, uint32_t x_start, uint32_t x_end, uint32_t y_start,
       uint32_t y_end, uint32_t direction);
void y(std::vector<uint32_t> *x_points, std::vector<uint32_t> *y_points,
       uint32_t depth, uint32_t x_start, uint32_t x_end, uint32_t y_start,
       uint32_t y_end, uint32_t direction);

Bounds child_bounds(uint32_t x_start, uint32_t x_end, uint32_t y_start,
                    uint32_t y_end, uint32_t column, uint32_t row,
                    uint32_t direction) {
    uint32_t transformed_column = column;
    uint32_t transformed_row = row;
    switch (direction % 4) {
    case 1:
        transformed_column = 2 - row;
        transformed_row = column;
        break;
    case 2:
        transformed_column = 2 - column;
        transformed_row = 2 - row;
        break;
    case 3:
        transformed_column = row;
        transformed_row = 2 - column;
        break;
    default:
        break;
    }

    const uint32_t x_first = x_start + static_cast<uint64_t>(x_end - x_start) *
                                           transformed_column / 3;
    const uint32_t x_last = x_start + static_cast<uint64_t>(x_end - x_start) *
                                          (transformed_column + 1) / 3;
    const uint32_t y_first =
        y_start + static_cast<uint64_t>(y_end - y_start) * transformed_row / 3;
    const uint32_t y_last = y_start + static_cast<uint64_t>(y_end - y_start) *
                                          (transformed_row + 1) / 3;
    return {x_first, x_last, y_first, y_last};
}

void append_middle(std::vector<uint32_t> *x_points,
                   std::vector<uint32_t> *y_points, uint32_t x_start,
                   uint32_t x_end, uint32_t y_start, uint32_t y_end) {
    x_points->push_back(x_start + (x_end - x_start) / 2);
    y_points->push_back(y_start + (y_end - y_start) / 2);
}

// X -> X Y X / Y X Y / X Y X, traversed from bottom to top.
void x(std::vector<uint32_t> *x_points, std::vector<uint32_t> *y_points,
       uint32_t depth, uint32_t x_start, uint32_t x_end, uint32_t y_start,
       uint32_t y_end, uint32_t direction) {
    if (depth > 0) {
        const uint32_t next_depth = depth - 1;
        const Bounds b0 =
            child_bounds(x_start, x_end, y_start, y_end, 0, 2, direction);
        const Bounds b1 =
            child_bounds(x_start, x_end, y_start, y_end, 1, 2, direction);
        const Bounds b2 =
            child_bounds(x_start, x_end, y_start, y_end, 2, 2, direction);
        const Bounds b3 =
            child_bounds(x_start, x_end, y_start, y_end, 2, 1, direction);
        const Bounds b4 =
            child_bounds(x_start, x_end, y_start, y_end, 1, 1, direction);
        const Bounds b5 =
            child_bounds(x_start, x_end, y_start, y_end, 0, 1, direction);
        const Bounds b6 =
            child_bounds(x_start, x_end, y_start, y_end, 0, 0, direction);
        const Bounds b7 =
            child_bounds(x_start, x_end, y_start, y_end, 1, 0, direction);
        const Bounds b8 =
            child_bounds(x_start, x_end, y_start, y_end, 2, 0, direction);
        x(x_points, y_points, next_depth, b0.x_start, b0.x_end, b0.y_start,
          b0.y_end, direction);
        y(x_points, y_points, next_depth, b1.x_start, b1.x_end, b1.y_start,
          b1.y_end, direction);
        x(x_points, y_points, next_depth, b2.x_start, b2.x_end, b2.y_start,
          b2.y_end, direction);
        y(x_points, y_points, next_depth, b3.x_start, b3.x_end, b3.y_start,
          b3.y_end, (direction + 2) % 4);
        x(x_points, y_points, next_depth, b4.x_start, b4.x_end, b4.y_start,
          b4.y_end, (direction + 2) % 4);
        y(x_points, y_points, next_depth, b5.x_start, b5.x_end, b5.y_start,
          b5.y_end, (direction + 2) % 4);
        x(x_points, y_points, next_depth, b6.x_start, b6.x_end, b6.y_start,
          b6.y_end, direction);
        y(x_points, y_points, next_depth, b7.x_start, b7.x_end, b7.y_start,
          b7.y_end, direction);
        x(x_points, y_points, next_depth, b8.x_start, b8.x_end, b8.y_start,
          b8.y_end, direction);
        return;
    }

    append_middle(x_points, y_points, x_start, x_end, y_start, y_end);
}

// Y mirrors X vertically and traverses from top to bottom.
void y(std::vector<uint32_t> *x_points, std::vector<uint32_t> *y_points,
       uint32_t depth, uint32_t x_start, uint32_t x_end, uint32_t y_start,
       uint32_t y_end, uint32_t direction) {
    if (depth > 0) {
        const uint32_t next_depth = depth - 1;
        const Bounds b0 =
            child_bounds(x_start, x_end, y_start, y_end, 0, 0, direction);
        const Bounds b1 =
            child_bounds(x_start, x_end, y_start, y_end, 1, 0, direction);
        const Bounds b2 =
            child_bounds(x_start, x_end, y_start, y_end, 2, 0, direction);
        const Bounds b3 =
            child_bounds(x_start, x_end, y_start, y_end, 2, 1, direction);
        const Bounds b4 =
            child_bounds(x_start, x_end, y_start, y_end, 1, 1, direction);
        const Bounds b5 =
            child_bounds(x_start, x_end, y_start, y_end, 0, 1, direction);
        const Bounds b6 =
            child_bounds(x_start, x_end, y_start, y_end, 0, 2, direction);
        const Bounds b7 =
            child_bounds(x_start, x_end, y_start, y_end, 1, 2, direction);
        const Bounds b8 =
            child_bounds(x_start, x_end, y_start, y_end, 2, 2, direction);
        y(x_points, y_points, next_depth, b0.x_start, b0.x_end, b0.y_start,
          b0.y_end, direction);
        x(x_points, y_points, next_depth, b1.x_start, b1.x_end, b1.y_start,
          b1.y_end, direction);
        y(x_points, y_points, next_depth, b2.x_start, b2.x_end, b2.y_start,
          b2.y_end, direction);
        x(x_points, y_points, next_depth, b3.x_start, b3.x_end, b3.y_start,
          b3.y_end, (direction + 2) % 4);
        y(x_points, y_points, next_depth, b4.x_start, b4.x_end, b4.y_start,
          b4.y_end, (direction + 2) % 4);
        x(x_points, y_points, next_depth, b5.x_start, b5.x_end, b5.y_start,
          b5.y_end, (direction + 2) % 4);
        y(x_points, y_points, next_depth, b6.x_start, b6.x_end, b6.y_start,
          b6.y_end, direction);
        x(x_points, y_points, next_depth, b7.x_start, b7.x_end, b7.y_start,
          b7.y_end, direction);
        y(x_points, y_points, next_depth, b8.x_start, b8.x_end, b8.y_start,
          b8.y_end, direction);
        return;
    }

    append_middle(x_points, y_points, x_start, x_end, y_start, y_end);
}

void generate_peano_points(Curve &curve) {
    curve.x_points.clear();
    curve.y_points.clear();

    const CurveBounds bounds = curve_bounds(curve);
    if (bounds.x_start >= bounds.x_end || bounds.y_start >= bounds.y_end) {
        return;
    }

    std::vector<uint32_t> control_x_points;
    std::vector<uint32_t> control_y_points;
    x(&control_x_points, &control_y_points, curve.depth, bounds.x_start,
      bounds.x_end, bounds.y_start, bounds.y_end, 0);

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
