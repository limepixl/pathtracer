@echo off
if not exist "build" mkdir "build"
pushd "build"

set "compiler_flags=-Oi -Od -Zi -FC -WX -W4 -wd4201 -wd4189 -GR- -EHa- -MT -nologo -D_CRT_SECURE_NO_WARNINGS"
cl %compiler_flags% ../src/main.cpp /link -opt:ref user32.lib gdi32.lib winmm.lib /out:pathtracer.exe
popd

echo.
echo Build finished.