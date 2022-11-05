#pragma once

#include "image.h"
#include "camera_options.h"
#include "render_options.h"
#include "../raytracer-reader/scene.h"
#include "matrix.h"
#include "../raytracer-geom/geometry.h"

#include <string>
#include <vector>
#include <variant>

Vector Recursive(int depth, const Scene& scene, const Ray& ray, bool in) {

    std::optional<Intersection> closest;
    std::optional<std::variant<Object, SphereObject>> obj;

    for (const auto& obj_iter : scene.GetObjects()) {
        auto intersection = GetIntersection(ray, obj_iter.polygon);
        if (!intersection.has_value()) {
            continue;
        }
        if (!closest.has_value()) {
            closest = intersection;
            obj = obj_iter;
        } else if (intersection->GetDistance() < closest->GetDistance()) {
            closest = intersection;
            obj = obj_iter;
        }
    }

    for (const auto& obj_iter : scene.GetSphereObjects()) {
        auto intersection = GetIntersection(ray, obj_iter.sphere);
        if (!intersection.has_value()) {
            continue;
        }
        if (!closest.has_value()) {
            closest = intersection;
            obj = obj_iter;
        } else if (intersection->GetDistance() < closest->GetDistance()) {
            closest = intersection;
            obj = obj_iter;
        }
    }

    if (!obj.has_value()) {
        return {0, 0, 0};
    }

    Vector normal;
    Material material;

    if (auto sphere = std::get_if<SphereObject>(&obj.value())) {
        normal = Normalized(closest->GetNormal());
        material = *sphere->material;
    } else {
        auto triangle = std::get_if<Object>(&obj.value());
        material = *triangle->material;

        if (*triangle->GetNormal(0) == Vector{0, 0, 0}) {
            normal = Normalized(closest->GetNormal());
        } else {
            auto c = GetBarycentricCoords(triangle->polygon, closest->GetPosition());
            normal = Normalized(*triangle->GetNormal(0) * c[0] + *triangle->GetNormal(1) * c[1] +
                                *triangle->GetNormal(2) * c[2]);
        }
    }

    Vector color = material.ambient_color + material.intensity;
    Vector base;

    for (const auto& l : scene.GetLights()) {
        Ray r(l.position, closest->GetPosition() + normal * 1e-4 - l.position);
        std::optional<Intersection> closest_to_light;

        for (const auto& obj_iter : scene.GetObjects()) {
            auto intersection = GetIntersection(r, obj_iter.polygon);
            if (!intersection.has_value()) {
                continue;
            }
            if (!closest_to_light.has_value()) {
                closest_to_light = intersection;
            } else if (intersection->GetDistance() < closest_to_light->GetDistance()) {
                closest_to_light = intersection;
            }
        }

        for (const auto& obj_iter : scene.GetSphereObjects()) {
            auto intersection = GetIntersection(r, obj_iter.sphere);
            if (!intersection.has_value()) {
                continue;
            }
            if (!closest_to_light.has_value()) {
                closest_to_light = intersection;
            } else if (intersection->GetDistance() < closest_to_light->GetDistance()) {
                closest_to_light = intersection;
            }
        }

        if (!closest_to_light.has_value() ||
            closest_to_light->GetDistance() > Length(closest->GetPosition() - l.position) - 1e-3) {
            Vector v_l = Normalized(l.position - closest->GetPosition());
            base += material.diffuse_color * l.intensity * std::max(.0, DotProduct(v_l, normal));
            Vector v_r = Reflect(-v_l, normal);
            Vector v_e = Normalized(ray.GetOrigin() - closest->GetPosition());
            base += material.specular_color * l.intensity *
                    std::pow(std::max(.0, DotProduct(v_r, v_e)), material.specular_exponent);
        }
    }

    color += base * material.albedo[0];

    if (in && depth != 0) {
        std::optional<Vector> refrac_vec =
            Refract(ray.GetDirection(), normal, material.refraction_index);
        if (refrac_vec.has_value()) {
            Ray refr(closest->GetPosition() - normal * 1e-4, *refrac_vec);
            bool new_in = in ^ (std::get_if<SphereObject>(&obj.value()) != nullptr);
            color += Recursive(depth - 1, scene, refr, new_in);
        }
    }

    if (material.albedo[1] != 0 && depth != 0 && !in) {
        Ray refl(closest->GetPosition() + normal * 1e-4, Reflect(ray.GetDirection(), normal));
        auto temp = Recursive(depth - 1, scene, refl, in);
        color += material.albedo[1] * temp;
    }

    if (material.albedo[2] != 0 && depth != 0 && !in) {
        std::optional<Vector> refrac_vec =
            Refract(ray.GetDirection(), normal, 1 / material.refraction_index);
        if (refrac_vec.has_value()) {
            Ray refr(closest->GetPosition() - normal * 1e-4, refrac_vec.value());
            bool new_in = in ^ (std::get_if<SphereObject>(&obj.value()) != nullptr);
            auto temp = Recursive(depth - 1, scene, refr, new_in);
            color += material.albedo[2] * temp;
        }
    }

    return color;
}

Image Render(const std::string& filename, const CameraOptions& camera_options,
             const RenderOptions& render_options) {

    std::vector<std::vector<double>> depths;
    std::vector<std::vector<Vector>> colors;
    double max_depth;
    double max_intensity;
    Image img(camera_options.screen_width, camera_options.screen_height);
    Scene scene = ReadScene(filename);

    auto mode = render_options.mode;
    RayTransformer rt(camera_options);

    if (mode == RenderMode::kDepth) {
        max_depth = 0;
        depths = std::vector<std::vector<double>>(camera_options.screen_height,
                                                  std::vector<double>(camera_options.screen_width));
    } else if (mode == RenderMode::kFull) {
        max_intensity = 0;
        colors = std::vector<std::vector<Vector>>(camera_options.screen_height,
                                                  std::vector<Vector>(camera_options.screen_width));
    }

    for (int i = 0; i != camera_options.screen_height; ++i) {
        for (int j = 0; j != camera_options.screen_width; ++j) {
            Ray ray = rt(j, i);
            std::optional<Intersection> closest;
            std::optional<std::variant<Object, SphereObject>> obj;
            for (auto obj_iter : scene.GetObjects()) {
                auto intersection = GetIntersection(ray, obj_iter.polygon);
                if (!intersection.has_value()) {
                    continue;
                }
                if (!closest.has_value()) {
                    closest = intersection;
                    obj = obj_iter;
                } else if (intersection->GetDistance() < closest->GetDistance()) {
                    closest = intersection;
                    obj = obj_iter;
                }
            }
            for (auto obj_iter : scene.GetSphereObjects()) {
                auto intersection = GetIntersection(ray, obj_iter.sphere);
                if (!intersection.has_value()) {
                    continue;
                }
                if (!closest.has_value()) {
                    closest = intersection;
                    obj = obj_iter;
                } else if (intersection->GetDistance() < closest->GetDistance()) {
                    closest = intersection;
                    obj = obj_iter;
                }
            }
            if (mode == RenderMode::kDepth) {
                if (!closest.has_value()) {
                    depths[i][j] = -1;
                } else {
                    depths[i][j] = closest->GetDistance();
                    max_depth = std::max(max_depth, depths[i][j]);
                }
            } else if (mode == RenderMode::kNormal) {
                if (!obj.has_value()) {
                    img.SetPixel({0, 0, 0}, i, j);
                } else if (std::get_if<SphereObject>(&obj.value())) {
                    img.SetPixel({static_cast<int>((closest->GetNormal()[0] + 1) / 2 * 255),
                                  static_cast<int>((closest->GetNormal()[1] + 1) / 2 * 255),
                                  static_cast<int>((closest->GetNormal()[2] + 1) / 2 * 255)},
                                 i, j);
                } else {
                    auto p = std::get_if<Object>(&obj.value());
                    if (*p->GetNormal(0) == Vector{0, 0, 0}) {
                        img.SetPixel({static_cast<int>((closest->GetNormal()[0] + 1) / 2 * 255),
                                      static_cast<int>((closest->GetNormal()[1] + 1) / 2 * 255),
                                      static_cast<int>((closest->GetNormal()[2] + 1) / 2 * 255)},
                                     i, j);
                    } else {
                        auto c = GetBarycentricCoords(p->polygon, closest->GetPosition());
                        auto normal = c[0] * *p->GetNormal(0) + c[1] * *p->GetNormal(1) +
                                      c[2] * *p->GetNormal(2);
                        img.SetPixel({static_cast<int>((normal[0] + 1) / 2 * 255),
                                      static_cast<int>((normal[1] + 1) / 2 * 255),
                                      static_cast<int>((normal[2] + 1) / 2 * 255)},
                                     i, j);
                    }
                }
            } else {
                auto color = Recursive(render_options.depth, scene, ray, false);
                max_intensity = std::max(max_intensity, color[0]);
                max_intensity = std::max(max_intensity, color[1]);
                max_intensity = std::max(max_intensity, color[2]);
                colors[i][j] = color;
            }
        }
    }

    if (mode == RenderMode::kDepth) {
        for (int i = 0; i != img.Height(); ++i) {
            for (int j = 0; j != img.Width(); ++j) {
                if (depths[i][j] == -1) {
                    img.SetPixel({255, 255, 255}, i, j);
                } else {
                    int d = depths[i][j] / max_depth * 255;
                    img.SetPixel({d, d, d}, i, j);
                }
            }
        }
    } else if (mode == RenderMode::kFull) {
        for (int i = 0; i != img.Height(); ++i) {
            for (int j = 0; j != img.Width(); ++j) {
                auto color = colors[i][j];
                color[0] *= (1 + color[0] / max_intensity / max_intensity) / (1 + color[0]);
                color[1] *= (1 + color[1] / max_intensity / max_intensity) / (1 + color[1]);
                color[2] *= (1 + color[2] / max_intensity / max_intensity) / (1 + color[2]);
                color[0] = std::pow(color[0], 1 / 2.2);
                color[1] = std::pow(color[1], 1 / 2.2);
                color[2] = std::pow(color[2], 1 / 2.2);
                img.SetPixel({static_cast<int>(color[0] * 255), static_cast<int>(color[1] * 255),
                              static_cast<int>(color[2] * 255)},
                             i, j);
            }
        }
    }
    return img;
}