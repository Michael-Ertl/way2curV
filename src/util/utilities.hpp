#pragma once

#include "curves/curve.hpp"


constexpr const char* way2curV = "way2curV"; 


struct LineSegment {
    uint32_t start_x;
    uint32_t start_y;
    uint32_t end_x;
    uint32_t end_y;
};

struct CurveBounds {
    uint32_t x_start;
    uint32_t x_end;
    uint32_t y_start;
    uint32_t y_end;
};

CurveBounds curve_bounds(const Curve &curve);
void append_line_points(Curve &curve, LineSegment line);
void draw_background(Curve &curve);
void draw_curve(Curve &curve);
