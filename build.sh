#!/bin/bash

[ ! -d "./build-linux" ] && mkdir ./build-linux
cd ./build-linux

compiler_flags="-O2 -g3 -Wall -Wextra -Werror -Wno-missing-braces -Wno-unused-variable -Wno-missing-field-initializers"
files="../src/*.cpp"
clang++ $compiler_flags $files -o pathtracer