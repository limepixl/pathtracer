#!/bin/bash

[ ! -d "./build-linux" ] && mkdir ./build-linux
cd ./build-linux

# cmake -S ../ -B . -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang
# cmake -S ../ -B . -G "Ninja" -DCMAKE_BUILD_TYPE=RELEASE
ninja