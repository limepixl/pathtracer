#!/bin/bash

[ ! -d "./build-linux" ] && mkdir ./build-linux
cd ./build-linux

cmake -S ../ -B . -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc
ninja