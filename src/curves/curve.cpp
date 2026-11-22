#include "curves/curve.hpp"

#include "curves/disk/diskcurve.hpp"
#include "curves/hilbert/hilbertcurve.hpp"
#include "curves/moore/moorecurve.hpp"
#include "curves/peano/peanocurve.hpp"
#include "curves/zorder/zordercurve.hpp"
#include "decorators/decorator.hpp"
#include "util/utilities.hpp"

void render_curve(Curve &curve) {

    if (curve.x_points.empty() && curve.y_points.empty()) {
        switch (curve.type) {
        case CurveType::Hilbert:
            generate_hilbert_points(curve);
            break;
        case CurveType::Moore:
            generate_moore_points(curve);
            break;
        case CurveType::Peano:
            generate_peano_points(curve);
            break;
        case CurveType::ZOrder:
            generate_z_order_points(curve);
            break;
        case CurveType::Disk:
            generate_disk_points(curve);
            break;
        }
    }
    place_on_background(curve);
    if (curve.curve_on) {
        draw_curve(curve);
    }
    place_on_curve(curve);

    decorate(curve);
}
