#pragma once

#include "../raytracer/render_options.h"
#include "../raytracer/camera_options.h"
#include "../raytracer-geom/vector.h"

#include <utility>
#include <string>
#include <fstream>
#include <vector>

inline std::pair<RenderOptions, CameraOptions> ReadConfig(std::string filename) {
    std::ifstream f;
    f.open(filename);
    
    RenderOptions ro;
    CameraOptions co{640, 480};

    for (std::string line; std::getline(f, line);) {
        std::vector<std::string> tokens;
        {
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
                        token = "";
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
        }
        
        if (tokens[0] == "camera") {
            if (tokens[1] == "w") {
                co.screen_width = std::stoi(tokens[2]);
            } else if (tokens[1] == "h") {
                co.screen_height = std::stoi(tokens[2]);
            } else if (tokens[1] == "fov") {
                co.fov = std::stod(tokens[2]);
            } else if (tokens[1] == "from") {
                co.look_from = {std::stod(tokens[2]), std::stod(tokens[3]), std::stod(tokens[4])};
            } else if (tokens[1] == "to") {
                co.look_to = {std::stod(tokens[2]), std::stod(tokens[3]), std::stod(tokens[4])};
            }
        } else if (tokens[0] == "render") {
            if (tokens[1] == "mode") {
                ro.mode = (tokens[2] == "depth") ?  RenderMode::kDepth :
                          (tokens[2] == "normal") ? RenderMode::kNormal :
                                                    RenderMode::kFull;
            } else if (tokens[1] == "depth") {
                ro.depth = std::stoi(tokens[2]);
            }
        }
    }
    return {ro, co};
}