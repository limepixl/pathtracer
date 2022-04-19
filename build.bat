@echo off
if not exist "build" mkdir "build"
pushd "build"

@REM cmake -S ../ -B . -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang
@REM cmake -S ../ -B . -G "Ninja" -DCMAKE_BUILD_TYPE=Release
@REM cmake -S ../ -B . -G "Ninja" -DCMAKE_BUILD_TYPE=Debug
ninja

popd

echo.
echo Build finished.