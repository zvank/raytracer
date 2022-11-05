#pragma once

#include <array>
#include <cmath>
#include <iostream>
#include <initializer_list>
#include <algorithm>

class Vector {
public:
    Vector() : data_{0, 0, 0} {
    }

    Vector(std::initializer_list<double> list) {
        auto it = list.begin();
        data_[0] = *it;
        data_[1] = *++it;
        data_[2] = *++it;
    }

    Vector(std::array<double, 3> data) : data_(data) {
    }

    double& operator[](size_t ind) {
        return data_[ind];
    }

    double operator[](size_t ind) const {
        return data_[ind];
    }

    void Normalize() {
        double hypot = std::__hypot3(data_[0], data_[1], data_[2]);
        if (hypot != 0) {
            data_[0] /= hypot;
            data_[1] /= hypot;
            data_[2] /= hypot;
        }
    }

private:
    std::array<double, 3> data_;
};

inline double DotProduct(const Vector& lhs, const Vector& rhs) {
    return lhs[0] * rhs[0] + lhs[1] * rhs[1] + lhs[2] * rhs[2];
}

inline Vector CrossProduct(const Vector& a, const Vector& b) {
    return {a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]};
}

inline double Length(const Vector& vec) {
    return std::__hypot3(vec[0], vec[1], vec[2]);
}

inline Vector operator-(const Vector& l, const Vector& r) {
    return {l[0] - r[0], l[1] - r[1], l[2] - r[2]};
}

inline Vector operator-(const Vector& l) {
    return {-l[0], -l[1], -l[2]};
}

inline Vector operator+(const Vector& l, const Vector& r) {
    return {l[0] + r[0], l[1] + r[1], l[2] + r[2]};
}

inline Vector operator-=(Vector& l, const Vector& r) {
    l[0] -= r[0];
    l[1] -= r[1];
    l[2] -= r[2];
    return l;
}

inline Vector operator+=(Vector& l, const Vector& r) {
    l[0] += r[0];
    l[1] += r[1];
    l[2] += r[2];
    return l;
}

inline Vector operator*(const Vector& l, double r) {
    return {l[0] * r, l[1] * r, l[2] * r};
}

inline Vector operator*(double l, const Vector& r) {
    return r * l;
}

Vector operator*(const Vector& l, const Vector& r) {
    return {l[0] * r[0], l[1] * r[1], l[2] * r[2]};
}

inline Vector operator/(const Vector& l, double r) {
    return {l[0] / r, l[1] / r, l[2] / r};
}

inline Vector Normalized(Vector v) {
    v.Normalize();
    return v;
}

inline std::pair<Vector, Vector> Orthogonal(const Vector& v) {
    if (Length(v) == 0) {
        return {};
    }
    if (v[0] != 0) {
        auto u = Normalized(CrossProduct(v, {0, 1, 0}));
        auto w = Normalized(CrossProduct(v, u));
        return {u, w};
    } else {
        auto u = Normalized(CrossProduct(v, {1, 0, 0}));
        auto w = Normalized(CrossProduct(v, u));
        return {u, w};
    }
}

std::ostream& operator<<(std::ostream& o, Vector v) {
    return o << "{ " << v[0] << " " << v[1] << " " << v[2] << " }";
}

bool operator==(const Vector& l, const Vector& r) {
    return l[0] == r[0] && l[1] == r[1] && l[2] == r[2];
}

bool operator!=(const Vector& l, const Vector& r) {
    return !(l == r);
}
