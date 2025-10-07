IF NOT EXIST "build" (
    MKDIR "build"
)

PUSHD build

SET MAKE_PROGRAM="C:/Users/Danial/AppData/Local/Programs/CLion/bin/ninja/win/x64/ninja.exe"
SET TOOLCHAIN_FILE="%VCPKG%\scripts\buildsystems\vcpkg.cmake"
SET GENERATOR="Ninja"

REM cmake -DCMAKE_TOOLCHAIN_FILE=%TOOLCHAIN_FILE% -DCMAKE_MAKE_PROGRAM=%MAKE_PROGRAM% -G %GENERATOR% ..
REM cmake --build .

cmake ..

POPD

REM CALL compile-shaders.bat
