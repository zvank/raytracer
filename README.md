# raytracer

This application allows to render simple 3D scenes with polygons and spheres.

Usage:   ``raytracer [path/to/obj/file] [path/to/png/file] (optional)[path/to/config]``<br>
``obj file``: standart ``.obj`` file (supported options are: ``v``, ``vn``, ``f``, ``P``, ``S``, ``usemtl``, ``mtllib``)<br><br>
``.mtl`` supported options are newmtl, ``Ka``, ``Kd``, ``Ks``, ``Ke``, ``Ns``, ``Ni``, ``al``<br><br>
``png file``: path to the future ``.png`` image of the scene<br><br>
``config``: file containing render options & camera options<br>

This repo contains ``example`` directory. You can build image of spheres in a box by running following sequence of commands in the root of this repo:<br>
```
mkdir build
cd build
cmake ..
make raytracer
./raytracer ../example/box/box.obj box.png ../example/box/config
```
                 
![bebra](https://github.com/zvank/raytracer/blob/master/demo.png)
