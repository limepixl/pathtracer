@echo off
if not exist "build" mkdir "build"
pushd "build"

@REM cmake -S ../ -B . -G "Ninja" -DCMAKE_BUILD_TYPE=Release
ninja

popd

echo.
echo Build finished.