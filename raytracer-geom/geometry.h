#include "ray.h"
#include "vector.h"
#include "sphere.h"
#include "intersection.h"
#include "triangle.h"

#include <optional>

std::optional<Intersection> GetIntersection(const Ray& ray, const Sphere& sphere) {
    auto dir = Normalized(ray.GetDirection());
    Vector center_relative = sphere.GetCenter() - ray.GetOrigin();
    Vector center_ray_closest = dir * DotProduct(dir, center_relative) / DotProduct(dir, dir);
    if (Length(center_ray_closest - center_relative) > sphere.GetRadius() + 1e-6) {
        return {};
    }
    double dist =
        std::sqrt(std::max<double>(0, sphere.GetRadius() * sphere.GetRadius() -
                                          Length(center_ray_closest - center_relative) *
                                              Length(center_ray_closest - center_relative)));
    auto l = center_ray_closest - dist * dir;
    auto r = center_ray_closest + dist * dir;
    bool inside_of_sphere = Length(center_relative) < sphere.GetRadius();
    if (DotProduct(l, dir) > 1e-4) {
        Vector norm = Normalized(l - center_relative) * (inside_of_sphere ? -1 : 1);
        return Intersection(l + ray.GetOrigin() + norm * 1e-5, norm, Length(l));
    }
    if (DotProduct(r, dir) > 1e-4) {
        Vector norm = Normalized(r - center_relative) * (inside_of_sphere ? -1 : 1);
        return Intersection(r + ray.GetOrigin() + norm * 1e-5, norm, Length(r));
    }
    return {};
}

std::optional<Intersection> GetIntersection(const Ray& ray, const Triangle& triangle) {
    Vector edge_1, edge_2, h, s, q;
    float a, f, u, v;
    edge_1 = triangle.GetVertex(1) - triangle.GetVertex(0);
    edge_2 = triangle.GetVertex(2) - triangle.GetVertex(0);
    h = CrossProduct(ray.GetDirection(), edge_2);
    a = DotProduct(edge_1, h);
    if (a > -1e-6 && a < 1e-6) {
        return {};
    }
    f = 1.0 / a;
    s = ray.GetOrigin() - triangle.GetVertex(0);
    u = f * DotProduct(s, h);
    if (u < 0.0 || u > 1.0) {
        return {};
    }
    q = CrossProduct(s, edge_1);
    v = f * DotProduct(ray.GetDirection(), q);
    if (v < 0.0 || u + v > 1.0) {
        return {};
    }
    float t = f * DotProduct(edge_2, q);
    if (t > 1e-6) {
        Vector dir = Normalized(ray.GetDirection());
        Vector perp = Normalized(CrossProduct(edge_1, edge_2));
        double a_0 = DotProduct(perp, s);
        double a_1 = DotProduct(perp, s + dir);
        double len = -a_0 / (a_1 - a_0);
        Vector intersection = ray.GetOrigin() + len * dir;
        Vector normal = DotProduct(perp, ray.GetDirection()) < 0 ? perp : -perp;
        double distance = Length(ray.GetOrigin() - intersection);
        return Intersection(intersection + normal * 1e-5, normal, distance);
    }
    return {};
}

std::optional<Vector> Refract(const Vector& ray, const Vector& normal, double eta) {
    // std::cout << "refract " << ray << " " << normal << "\n";
    double cos = -DotProduct(ray, normal) / Length(ray) / Length(normal);
    double sin = std::sqrt(1 - cos * cos);
    if (sin * eta - 1 > -1e-6) {
        return {};
    }
    Vector projection = normal * DotProduct(normal, ray) / DotProduct(normal, normal);
    Vector delta = ray - projection;
    double coefficient = eta * cos / std::sqrt(1 - sin * sin * eta * eta);
    // std::cout << "ret " << Normalized(projection + coefficient * delta) << "\n";
    return Normalized(projection + coefficient * delta);
}

Vector Reflect(const Vector& ray, const Vector& normal) {
    Vector projection = normal * DotProduct(normal, ray) / DotProduct(normal, normal);
    return ray - 2 * projection;
}

Vector GetBarycentricCoords(const Triangle& triangle, const Vector& point) {
    const Vector& a = triangle.GetVertex(0);
    const Vector& b = triangle.GetVertex(1);
    const Vector& c = triangle.GetVertex(2);
    Vector a_proj = DotProduct(a - b, c - b) / DotProduct(c - b, c - b) * (c - b) + b;
    float a_coord = 1 - DotProduct(a_proj - a, point - a) / DotProduct(a_proj - a, a_proj - a);
    Vector b_proj = DotProduct(b - c, a - c) / DotProduct(a - c, a - c) * (a - c) + c;
    float b_coord = 1 - DotProduct(b_proj - b, point - b) / DotProduct(b_proj - b, b_proj - b);
    return {a_coord, b_coord, 1 - a_coord - b_coord};
}
