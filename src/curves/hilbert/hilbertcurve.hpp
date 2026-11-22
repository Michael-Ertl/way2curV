#pragma once

#include "curves/curve.hpp"


void left(std::vector<uint32_t> *x_points, std::vector<uint32_t> *y_points,
          uint32_t depth, uint32_t x_s, uint32_t x_e, uint32_t y_s,
          uint32_t y_e);
void right(std::vector<uint32_t> *x_points, std::vector<uint32_t> *y_points,
           uint32_t depth, uint32_t x_s, uint32_t x_e, uint32_t y_s,
           uint32_t y_e);
void up(std::vector<uint32_t> *x_points, std::vector<uint32_t> *y_points,
        uint32_t depth, uint32_t x_s, uint32_t x_e, uint32_t y_s, uint32_t y_e);
void down(std::vector<uint32_t> *x_points, std::vector<uint32_t> *y_points,
          uint32_t depth, uint32_t x_s, uint32_t x_e, uint32_t y_s,
          uint32_t y_e);


void generate_hilbert_points(Curve &curve);
