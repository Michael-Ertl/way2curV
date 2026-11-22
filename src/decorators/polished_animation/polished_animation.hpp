#pragma once

#include "decorators/decorator.hpp"

class PolishedDecorator final : public Decorator {
  public:
    void place_on_background(Curve &curve) const override;
    void place_on_curve(Curve &curve) const override;
    void decorate(Curve &curve) const override;
};
