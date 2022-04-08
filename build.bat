@echo off
if not exist "build" mkdir "build"
pushd "build"

cmake -S ../ -B . -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang
@REM cmake -S ../ -B . -G "Ninja" -DCMAKE_BUILD_TYPE=Release
ninja

popd

echo.
echo Build finished.