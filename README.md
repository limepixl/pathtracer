# Pathtracer
This project started as a playground and implementation of light transport theory and concepts in the form of a unidirectional pathtracer. It is currently a Work In Progress, with a lot of features to work on still.

### Current list of features:
- Physically based BRDFs (Lambertian, Oren-Nayar, Specular (GGX))
- Naive BRDF sampling
- NEE (Next Event Estimation / Direct light sampling)
- MIS (Multiple Importance Sampling between Naive and NEE)
- OpenGL Compute Shader implementation (OpenGL 4.6 Core, GLSL)
- Custom-made BVH implementations using Sweep SAH split
- (WIP) glTF model loading and material creation
- CMake build that supports Windows (MSVC, Clang for Windows) and Linux (GCC, Clang)

### Results and renders (**From earliest render to current day**):
Very early result using a purely Lambertian BRDF and naive hemisphere sampling
![naive_lambertian](https://raw.githubusercontent.com/limepixl/pathtracer/main/renders/early_naive_lambertian.png)

First tests of MIS between Naive and NEE
![mis](https://raw.githubusercontent.com/limepixl/pathtracer/main/renders/early_mis.png)

MIS (Naive+NEE), Modified Blinn-Phong, Sweep SAH BVH
![bvh](https://raw.githubusercontent.com/limepixl/pathtracer/main/renders/mis_bvh_blinnphong_bunny.png)

Specular BRDF (GGX NDF with Smith geometry func.), using GGX importance sampling - Metallic Gold material
![specular](https://raw.githubusercontent.com/limepixl/pathtracer/main/renders/naive_specular_gold.png)

(WIP) glTF Model support
![gltf](https://raw.githubusercontent.com/limepixl/pathtracer/main/renders/gltf_support.png)

### Third-party libraries used:
- cgltf ([GitHub](https://github.com/jkuhlmann/cgltf))
- PCG Random Number Generation Library (Minimal C Edition) ([GitHub](https://github.com/imneme/pcg-c-basic))
- GLAD - Multi-Language OpenGL Loader-Generator ([GitHub](https://github.com/Dav1dde/glad))
- Simple DirectMedia Layer 2 - SDL2 ([GitHub](https://github.com/libsdl-org/SDL))
- stb libraries ([GitHub](https://github.com/nothings/stb))

### To-Do List:
- Implement more variance reduction techniques (stratified sampling, BRDF sampling, low discrepancy sequences).
- Russian Roulette
- Measuring different radiometric quantities (for example intensity, to get an orthographic projection)
