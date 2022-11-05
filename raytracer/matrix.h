#pragma once

#include "../raytracer-geom/vector.h"
#include "../raytracer-geom/ray.h"
#include "camera_options.h"

#include <array>

class Matrix {
public:
    Matrix(Vector a, Vector b, Vector c) {
        data_[0] = a;
        data_[1] = b;
        data_[2] = c;
    }

    Matrix() {
    }

    inline Matrix operator*(const Matrix& other) {
        Matrix res;
        for (int i = 0; i != 3; ++i) {
            for (int j = 0; j != 3; ++j) {
                for (int k = 0; k != 3; ++k) {
                    res[i][j] += data_[i][k] * other[k][j];
                }
            }
        }
        return res;
    }

    inline Matrix& operator*=(const Matrix& other) {
        *this = operator*(other);
        return *this;
    }

    inline Vector& operator[](size_t i) {
        return data_[i];
    }

    inline const Vector& operator[](size_t i) const {
        return data_[i];
    }

    inline Vector operator*(const Vector& v) const {
        return {DotProduct(data_[0], v), DotProduct(data_[1], v), DotProduct(data_[2], v)};
    }

private:
    std::array<Vector, 3> data_;
};

inline Matrix CreateTransitionMatrix(const CameraOptions& co) {
    Vector dir = Normalized(Vector(co.look_to) - Vector(co.look_from));
    float alpha_sin = dir[1];
    float alpha_cos = std::sqrt(1 - alpha_sin * alpha_sin);
    Matrix vertical_transform({1, 0, 0}, {0, alpha_cos, -alpha_sin}, {0, alpha_sin, alpha_cos});
    dir[1] = 0;
    dir.Normalize();
    if (Length(dir) < 1e-6) {
        return vertical_transform;
    }
    dir = -dir;
    Matrix rotate({dir[2], 0, dir[0]}, {0, 1, 0}, {-dir[0], 0, dir[2]});
    return rotate * vertical_transform;
}

class RayTransformer {
public:
    RayTransformer(const CameraOptions& co)
        : co_(co), m_(CreateTransitionMatrix(co)), def_(co_.screen_height / 2) {
    }

    Ray operator()(size_t i, size_t j) const {
        double x = i - static_cast<double>(co_.screen_width - 1) / 2;
        double y = j - static_cast<double>(co_.screen_height - 1) / 2;
        x *= std::tan(co_.fov / 2) / def_;
        y *= std::tan(co_.fov / 2) / def_;
        return Ray(co_.look_from, Normalized(m_ * Vector({x, -y, -1})));
    }

private:
    CameraOptions co_;
    Matrix m_;
    size_t def_;
};
