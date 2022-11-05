#pragma once

#include "material.h"
#include "../raytracer-geom/vector.h"
#include "object.h"
#include "light.h"

#include <vector>
#include <map>
#include <string>

#include <fstream>

enum StrType {
    kVertex,
    kNormal,
    kFace,
    kLib,
    kMaterial,
    kSphere,
    kLight,
};

class Scene {
public:
    const std::vector<Object>& GetObjects() const {
        return objects_;
    }

    const std::vector<SphereObject>& GetSphereObjects() const {
        return spheres_;
    }

    const std::vector<Light>& GetLights() const {
        return lights_;
    }

    const std::map<std::string, Material>& GetMaterials() const {
        return materials_;
    }

    friend inline Scene ReadScene(const std::string& filename);

    ~Scene() {
        for (auto& obj : objects_) {
            delete obj.material;
        }

        for (auto& s : spheres_) {
            delete s.material;
        }
    }

private:
    std::vector<Object> objects_;
    std::vector<SphereObject> spheres_;
    std::vector<Light> lights_;
    std::map<std::string, Material> materials_;
};

inline std::array<int, 3> GetTokenInfo(const std::string& token) {
    std::array<int, 3> result = {0, 0, 0};
    int slashes = 0;
    std::string curr;
    for (char c : token) {
        if (c == '/') {
            result[slashes] = curr.empty() ? 0 : std::stoi(curr);
            curr = "";
            ++slashes;
        } else {
            curr.push_back(c);
        }
    }
    result[slashes] = std::stoi(curr);
    curr = "";
    return result;
}

inline void ParseFaceDeclaration(const std::vector<std::string>& tokens,
                                 std::vector<Object>& objects, std::string mat_name,
                                 const std::vector<Vector>& vertices,
                                 const std::vector<Vector>& normals,
                                 const std::map<std::string, Material>& materials) {
    if (tokens.size() < 3) {
        return;
    }

    auto first_elem_info = GetTokenInfo(tokens[0]);
    first_elem_info[0] =
        first_elem_info[0] < 0 ? vertices.size() + first_elem_info[0] : first_elem_info[0] - 1;
    first_elem_info[2] =
        first_elem_info[2] < 0 ? normals.size() + first_elem_info[2] : first_elem_info[2] - 1;
    std::array<int, 3> old_info = GetTokenInfo(tokens[1]), new_info = GetTokenInfo(tokens[2]);
    old_info[0] = old_info[0] < 0 ? vertices.size() + old_info[0] : old_info[0] - 1;
    old_info[2] = old_info[2] < 0 ? normals.size() + old_info[2] : old_info[2] - 1;
    new_info[0] = new_info[0] < 0 ? vertices.size() + new_info[0] : new_info[0] - 1;
    new_info[2] = new_info[2] < 0 ? normals.size() + new_info[2] : new_info[2] - 1;

    Material* material = new Material(materials.at(mat_name));
    auto triangle =
        Triangle({vertices[first_elem_info[0]], vertices[old_info[0]], vertices[new_info[0]]});
    auto normal = std::array<Vector, 3>(
        {first_elem_info[2] == -1 ? Vector{0, 0, 0} : normals[first_elem_info[2]],
         old_info[2] == -1 ? Vector{0, 0, 0} : normals[old_info[2]],
         new_info[2] == -1 ? Vector{0, 0, 0} : normals[new_info[2]]});
    objects.push_back(Object{material, triangle, normal});

    for (size_t i = 3; i < tokens.size(); ++i) {
        old_info = new_info;
        new_info = GetTokenInfo(tokens[i]);
        new_info[0] = new_info[0] < 0 ? vertices.size() + new_info[0] : new_info[0] - 1;
        new_info[2] = new_info[2] < 0 ? normals.size() + new_info[2] : new_info[2] - 1;

        Material* material = new Material(materials.at(mat_name));
        auto triangle =
            Triangle({vertices[first_elem_info[0]], vertices[old_info[0]], vertices[new_info[0]]});
        auto normal = std::array<Vector, 3>(
            {first_elem_info[2] == -1 ? Vector{0, 0, 0} : normals[first_elem_info[2]],
             old_info[2] == -1 ? Vector{0, 0, 0} : normals[old_info[2]],
             new_info[2] == -1 ? Vector{0, 0, 0} : normals[new_info[2]]});
        objects.push_back(Object{material, triangle, normal});
    }
}

inline std::map<std::string, Material> ReadMaterials(std::string filename) {

    std::map<std::string, Material> res;

    std::ifstream f;
    f.open(filename);

    std::vector<Vector> vertices;
    std::vector<Vector> normals;

    std::string current;

    for (std::string line; std::getline(f, line);) {
        std::vector<std::string> tokens;
        std::string token;
        for (auto c : line) {
            if (!isspace(c)) {
                token.push_back(c);
            } else if (!token.empty()) {
                size_t i = 0;
                while (token.size() > i && std::isspace(token[i])) {
                    ++i;
                }
                token = std::string(token.begin() + i, token.end());

                if (token[0] == '#') {
                    break;
                }
                tokens.push_back(token);
                token = "";
            }
        }
        if (!token.empty()) {
            size_t i = 0;
            while (token.size() > i && std::isspace(token[i])) {
                ++i;
            }
            token = std::string(token.begin() + i, token.end());
            tokens.push_back(token);
            token = "";
        }
        if (tokens.empty()) {
            continue;
        }
        if (tokens[0] == "newmtl") {
            res[tokens[1]] = Material();
            res[tokens[1]].name = tokens[1];
            res[tokens[1]].albedo = {1, 0, 0};
            current = tokens[1];
        } else if (tokens[0] == "Ka") {
            res[current].ambient_color = {std::stod(tokens[1]), std::stod(tokens[2]),
                                          std::stod(tokens[3])};
        } else if (tokens[0] == "Kd") {
            res[current].diffuse_color = {std::stod(tokens[1]), std::stod(tokens[2]),
                                          std::stod(tokens[3])};
        } else if (tokens[0] == "Ks") {
            res[current].specular_color = {std::stod(tokens[1]), std::stod(tokens[2]),
                                           std::stod(tokens[3])};
        } else if (tokens[0] == "Ke") {
            res[current].intensity = {std::stod(tokens[1]), std::stod(tokens[2]),
                                      std::stod(tokens[3])};
        } else if (tokens[0] == "Ns") {
            res[current].specular_exponent = std::stod(tokens[1]);
        } else if (tokens[0] == "Ni") {
            res[current].refraction_index = std::stod(tokens[1]);
        } else if (tokens[0] == "al") {
            res[current].albedo = {std::stod(tokens[1]), std::stod(tokens[2]),
                                   std::stod(tokens[3])};
        } else {
            continue;
        }
    }

    return res;
}

inline Scene ReadScene(const std::string& filename) {
    Scene res;

    std::string dir_name = filename;
    while (!dir_name.empty() && dir_name.back() != '/') {
        dir_name.pop_back();
    }

    std::ifstream f;
    f.open(filename);

    std::vector<Vector> vertices;
    std::vector<Vector> normals;
    std::string mat_name;

    for (std::string line; std::getline(f, line);) {
        size_t counter = 0;
        StrType str_type;
        std::vector<std::string> tokenized;

        std::vector<std::string> tokens;
        std::string token;
        for (auto c : line) {
            if (!isspace(c)) {
                token.push_back(c);
            } else if (!token.empty()) {
                if (token[0] == '#') {
                    token = "";
                    break;
                }
                if (counter == 0) {
                    if (token == "v") {
                        str_type = kVertex;
                    } else if (token == "vn") {
                        str_type = kNormal;
                    } else if (token == "f") {
                        str_type = kFace;
                    } else if (token == "mtllib") {
                        str_type = kLib;
                    } else if (token == "usemtl") {
                        str_type = kMaterial;
                    } else if (token == "S") {
                        str_type = kSphere;
                    } else if (token == "P") {
                        str_type = kLight;
                    } else {
                        break;
                    }
                    ++counter;
                } else {
                    tokenized.push_back(token);
                }
                if (counter == 0) {
                    break;
                }
                token = "";
            }
        }
        if (!token.empty()) {
            if (counter == 0) {
                if (token == "v") {
                    str_type = kVertex;
                } else if (token == "vn") {
                    str_type = kNormal;
                } else if (token == "f") {
                    str_type = kFace;
                } else if (token == "mtllib") {
                    str_type = kLib;
                } else if (token == "usemtl") {
                    str_type = kMaterial;
                } else if (token == "S") {
                    str_type = kSphere;
                } else if (token == "P") {
                    str_type = kLight;
                } else {
                    continue;
                }
                ++counter;
            } else {
                tokenized.push_back(token);
            }
            token = "";
        }

        if (counter == 0) {
            continue;
        }
        switch (str_type) {
            case kVertex:
                vertices.push_back(
                    {std::stod(tokenized[0]), std::stod(tokenized[1]), std::stod(tokenized[2])});
                break;
            case kNormal:
                normals.push_back(
                    {std::stod(tokenized[0]), std::stod(tokenized[1]), std::stod(tokenized[2])});
                break;
            case kFace:
                ParseFaceDeclaration(tokenized, res.objects_, mat_name, vertices, normals,
                                     res.materials_);
                break;
            case kLib:
                res.materials_ = ReadMaterials(dir_name + tokenized[0]);
                break;
            case kMaterial:
                mat_name = tokenized[0];
                break;
            case kSphere:
                res.spheres_.push_back(
                    {new Material(res.materials_[mat_name]),
                     Sphere(Vector({std::stod(tokenized[0]), std::stod(tokenized[1]),
                                    std::stod(tokenized[2])}),
                            std::stod(tokenized[3]))});
                break;
            case kLight:
                res.lights_.push_back({Vector({std::stod(tokenized[0]), std::stod(tokenized[1]),
                                               std::stod(tokenized[2])}),
                                       Vector({std::stod(tokenized[3]), std::stod(tokenized[4]),
                                               std::stod(tokenized[5])})});
                break;
            default:
                break;
        }
    }
    return res;
}
