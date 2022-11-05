#include "raytracer.h"
#include "../tools/util/util.h"
#include "../raytracer-reader/config_reader.h"

#include <iostream>
#include <filesystem>

void QuitIncorrectArguments(char** argv) {
    std::cerr << "Incorrect arguments\n"
                 "Usage: " << argv[0] << " [path/to/obj/file] [path/to/png/file] (optional)[path/to/config]\n"
                 "\n"
                 "obj file: standart .obj file (supported options are: v, vn, f, P, S, usemtl, mtllib)\n"
                 ".mtl supported options are newmtl, Ka, Kd, Ks, Ke, Ns, Ni, al\n"
                 "\n"
                 "png file: path to the future .png image of the scene\n"
                 "\n"
                 "config: file containing render options & camera options\n"
                 "(default config is provided in example/box/config)\n"
                 "\n";
    exit(1);
}

int main(int argc, char** argv) {
    if (argc < 3) {
        QuitIncorrectArguments(argv);
    }

    std::string obj = weakly_canonical(std::filesystem::current_path() / std::string(argv[1]));
    std::string img_path = weakly_canonical(std::filesystem::current_path() / std::string(argv[2]));
    CameraOptions co(640, 480);
    RenderOptions ro{1};
    if (argc >= 4) {
        std::string config = weakly_canonical(std::filesystem::current_path() / std::string(argv[3]));
        auto [ro_, co_] = ReadConfig(config);
        ro = ro_;
        co = co_;
    }
    auto img = Render(obj, co, ro);
    img.Write(img_path);
}