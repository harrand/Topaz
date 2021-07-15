@echo off

REM get vcpkg distribution
if not exist vcpkg git clone https://github.com/Microsoft/vcpkg.git

REM build vcpkg
if not exist vcpkg\vcpkg.exe call vcpkg\bootstrap-vcpkg.bat -disableMetrics

REM install required packages
vcpkg\vcpkg.exe install --triplet x64-mingw-static freetype[bzip2,core,png,zlib] glfw3 capstone[arm,arm64,x86]
