#include "diskcurve.hpp"

#include "curves/moore/moorecurve.hpp"
#include "util/utilities.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>

// Fills the disk inscribed in the curve bounds: the Moore curve is generated
// in the bounding rectangle, each point is normalized onto the unit square
// (per-axis), then rescaled along its ray from the center so the square
// boundary lands exactly on the circle. The composed rectangle -> disk map is
// a homeomorphism, so the curve still covers the whole disk in the continuous
// limit, and the Moore loop stays closed on the rim.
void generate_disk_points(Curve &curve) {
    generate_moore_points(curve);

    const CurveBounds bounds = curve_bounds(curve);
    const double half_width = (bounds.x_end - bounds.x_start) / 2.0;
    const double half_height = (bounds.y_end - bounds.y_start) / 2.0;
    if (half_width <= 0.0 || half_height <= 0.0) {
        return;
    }

    const double center_x = (bounds.x_start + bounds.x_end) / 2.0;
    const double center_y = (bounds.y_start + bounds.y_end) / 2.0;
    const double radius = std::min(half_width, half_height);

    for (std::size_t i = 0; i < curve.x_points.size(); ++i) {
        const double u =
            (static_cast<double>(curve.x_points[i]) - center_x) / half_width;
        const double v =
            (static_cast<double>(curve.y_points[i]) - center_y) / half_height;
        const double r = std::sqrt(u * u + v * v);
        if (r == 0.0) {
            continue; // The center maps to itself.
        }

        const double scale = std::max(std::abs(u), std::abs(v)) / r;
        const double x = center_x + u * radius * scale;
        const double y = center_y + v * radius * scale;
        curve.x_points[i] = static_cast<uint32_t>(std::lround(x));
        curve.y_points[i] = static_cast<uint32_t>(std::lround(y));
    }
}
