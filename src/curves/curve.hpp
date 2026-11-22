#pragma once
#include <chrono>
#include <cstdint>
#include <vector>
enum class CurveType {
    Hilbert,
    Moore,
    Peano,
    ZOrder,
    Disk,
};

enum class AnimationType {
    None,
    Polished,
};

struct Curve {
    std::string monitor_name;
    std::string image_path; // TODO: in the config.cpp read and set this
    uint32_t *pixels = nullptr;
    std::vector<uint32_t> image_cache; 
    std::vector<uint32_t> x_points;
    std::vector<uint32_t> y_points;

    // Fractional position along the rasterized curve.
    double animated_index = 0.0;

    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t left_margin = 100;
    uint32_t right_margin = 100;
    uint32_t top_margin = 50;
    uint32_t bottom_margin = 50;
    uint32_t line_width = 10;
    uint32_t animation_length = 200;
    uint32_t animation_size = 35;
    uint32_t background_color = 0x00000000;
    uint32_t line_color = 0x00ff0000;
    bool curve_on = true;
    uint32_t animation_color = 0x00ffff00;
    uint32_t depth = 3;
    uint32_t step_size = 0;
    CurveType type = CurveType::Hilbert;
    AnimationType animation_type = AnimationType::None;
    std::chrono::steady_clock::time_point last_update;
    std::chrono::milliseconds time_step = std::chrono::milliseconds(1);
};

void render_curve(Curve &curve);
