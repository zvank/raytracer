#pragma once

#include "vector.h"

#include <array>

class Triangle {
public:
    Triangle(std::initializer_list<Vector> list) {
        auto it = list.begin();
        vertices_[0] = *it;
        vertices_[1] = *++it;
        vertices_[2] = *++it;
    }

    double Area() const {
        double a = Length((vertices_[0] - vertices_[1]));
        double b = Length((vertices_[1] - vertices_[2]));
        double c = Length((vertices_[2] - vertices_[0]));
        double half_p = (a + b + c) / 2;
        return std::sqrt(half_p * (half_p - a) * (half_p - b) * (half_p - c));
    }

    const Vector& GetVertex(size_t ind) const {
        return vertices_[ind];
    }

private:
    std::array<Vector, 3> vertices_;
};
