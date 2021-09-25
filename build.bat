@echo off
if not exist "build" mkdir "build"
pushd "build"

@REM MSVC
set "compiler_flags=-Oi -O2 -Zi -FC -WX -W4 -wd4201 -wd4189 -wd4100 -GR- -EHsc -MT -nologo -D_CRT_SECURE_NO_WARNINGS"
set "files=../src/threads.cpp ../src/main.cpp ../src/loader.cpp ../src/intersect.cpp ../src/vec.cpp ../src/mat4.cpp ../src/math.cpp ../src/material.cpp"
cl %compiler_flags% %files% /link -opt:ref user32.lib gdi32.lib winmm.lib /out:pathtracer.exe

@REM Clang
@REM set "compiler_flags=-O2 -g3 -Wall -Wextra -Werror -Wno-missing-braces -Wno-unused-variable -Wno-missing-field-initializers -D_CRT_SECURE_NO_WARNINGS"
@REM clang++ %compiler_flags% ../src/main.cpp ../src/loader.cpp ../src/intersect.cpp ../src/vec.cpp ../src/mat4.cpp ../src/math.cpp -o pathtracer.exe

popd

echo.
echo Build finished.