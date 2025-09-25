# Raytracer

A simple raytracer using:
- argtable3 for CLI parsing
- zlog for logging
- libpng for image output

## Build

```bash
conan install . --output-folder=build --build=missing
cmake --preset conan-release
cmake --build --preset conan-release
