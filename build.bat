@echo off
if not exist "build" mkdir "build"
pushd "build"

@REM Clang
@REM cmake -S ../ -B . -G "Ninja" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang
@REM cmake -S ../ -B . -G "Ninja" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang
@REM cmake -S ../ -B . -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang

@REM MSVC
@REM cmake -S ../ -B . -G "Ninja" -DCMAKE_BUILD_TYPE=Debug
@REM cmake -S ../ -B . -G "Ninja" -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake -S ../ -B . -G "Ninja" -DCMAKE_BUILD_TYPE=Release
ninja

popd

echo.
echo Build finished.