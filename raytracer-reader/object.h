#pragma once

#include "../raytracer-geom/triangle.h"
#include "material.h"
#include "../raytracer-geom/sphere.h"

struct Object {
    const Material* material = nullptr;
    Triangle polygon;
    std::array<Vector, 3> normals;

    const Vector* GetNormal(size_t index) const {
        return &normals[index];
    }
};

struct SphereObject {
    const Material* material = nullptr;
    Sphere sphere;
};
