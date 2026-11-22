#pragma once

#include "curves/curve.hpp"

class Decorator {
  public:
    virtual ~Decorator() = default;
    virtual void place_on_background(Curve &curve) const = 0;
    virtual void place_on_curve(Curve &curve) const = 0;
    virtual void decorate(Curve &curve) const = 0;
};

void place_on_background(Curve &curve);
void place_on_curve(Curve &curve);
void decorate(Curve &curve);
