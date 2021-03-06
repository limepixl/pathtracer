# Pathtracer
This project started as a playground and implementation of light transport theory and concepts in the form of a unidirectional pathtracer. It is currently a Work In Progress, with a lot of features to work on still.

### Current list of features:
- Simple BRDFs (Lambertian, Modified Blinn-Phong)
- Naive hemisphere sampling
- NEE (Next Event Estimation / Direct light sampling)
- MIS (Multiple Importance Sampling between Naive and NEE)
- (WIP) OBJ model loading and material creation
- Custom-made BVH implementations using Object Median split and Sweep SAH
- (WIP) A simplistic thread-pool implementation using CPU threads (Win32 and POSIX threads)
- PPM image output
- CMake build that supports Windows (MSVC, Clang for Windows) and Linux (GCC, Clang)

### Results and renders (**From earliest render to current day**):
Very early result using a purely Lambertian BRDF and naive hemisphere sampling
![naive_lambertian](https://raw.githubusercontent.com/limepixl/pathtracer/main/renders/early_naive_lambertian.png)

First tests of MIS between Naive and NEE
![mis](https://raw.githubusercontent.com/limepixl/pathtracer/main/renders/early_mis.png)

MIS (Naive+NEE), Modified Blinn-Phong, Sweep SAH BVH
![bvh](https://raw.githubusercontent.com/limepixl/pathtracer/main/renders/mis_bvh_blinnphong_bunny.png)

### Third-party libraries used:
- Tiny OBJ Loader ([GitHub](https://github.com/tinyobjloader/tinyobjloader))
- PCG Random Number Generation Library (Minimal C Edition) ([GitHub](https://github.com/imneme/pcg-c-basic))

### To-Do List:
- Timing
- GPU support
- Textures
- Implement more variance reduction techniques (stratified sampling, BRDF sampling, low discrepancy sequences).
- More advanced BRDFs and BSDFs
- GLTF model support
- Russian Roulette
- Measuring different radiometric quantities (for example intensity, to get an orthographic projection)
- Progressive rendering over multiple frames but lower sample count
