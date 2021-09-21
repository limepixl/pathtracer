@echo off
if not exist "build" mkdir "build"
pushd "build"

set "compiler_flags=-Oi -O2 -Zi -FC -WX -W4 -wd4201 -wd4189 -wd4100 -GR- -EHsc -MT -nologo -D_CRT_SECURE_NO_WARNINGS"
cl %compiler_flags% ../src/main.cpp ../src/loader.cpp ../src/intersect.cpp ../src/vec.cpp ../src/math.cpp /link -opt:ref user32.lib gdi32.lib winmm.lib /out:pathtracer.exe
popd

echo.
echo Build finished.