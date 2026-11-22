#include "decorator.hpp"
#include "decorators/polished_animation/polished_animation.hpp"

static const Decorator *decorator_for(AnimationType type) {
    static const PolishedDecorator animation;

    switch (type) {
    case AnimationType::Polished:
        return &animation;
    default:
        return nullptr;
    }
}

void place_on_background(Curve &curve) {
    if (const Decorator *decorator = decorator_for(curve.animation_type)) {
        decorator->place_on_background(curve);
    }
}

void place_on_curve(Curve &curve) {
    if (const Decorator *decorator = decorator_for(curve.animation_type)) {
        decorator->place_on_curve(curve);
    }
}

void decorate(Curve &curve) {
    if (const Decorator *decorator = decorator_for(curve.animation_type)) {
        decorator->decorate(curve);
    }
}
