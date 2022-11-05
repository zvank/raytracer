#pragma once

#include "../raytracer-geom/vector.h"
#include "../raytracer-geom/triangle.h"

struct Light {
    Vector position;
    Vector intensity;

    Light(const Vector& position, const Vector& intensity)
        : position(position), intensity(intensity) {
    }
};
