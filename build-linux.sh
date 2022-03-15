#!/bin/bash

[ ! -d "./build-linux" ] && mkdir ./build-linux
cd ./build-linux

compiler_flags="-O2 -g3 -I ../thirdparty/ -pthread -Wall -Wextra -Werror -Wno-missing-braces -Wno-unused-variable -Wno-missing-field-initializers"
files="../src/math/*.cpp ../src/scene/*.cpp ../src/loader.cpp ../src/main.cpp ../src/threads.cpp"
clang $compiler_flags "../thirdparty/pcg-c-basic-0.9/pcg_basic.c" -c
clang++ $compiler_flags $files pcg_basic.o -o pathtracer
rm pcg_basic.o