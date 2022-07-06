@echo off
if not exist "build" mkdir "build"
pushd "build"

@REM Clang
@REM cmake -S ../ -B . -G "Ninja" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang
@REM cmake -S ../ -B . -G "Ninja" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang
cmake -S ../ -B . -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang

@REM MSVC
@REM cmake -S ../ -B . -G "Ninja" -DCMAKE_BUILD_TYPE=Debug
@REM cmake -S ../ -B . -G "Ninja" -DCMAKE_BUILD_TYPE=RelWithDebInfo
@REM cmake -S ../ -B . -G "Ninja" -DCMAKE_BUILD_TYPE=Release
ninja

popd

echo Copying SDL DLL to build directory.....
copy thirdparty\SDL2-2.0.22\lib\x64\SDL2.dll build\SDL2.dll

echo.
echo Build finished.