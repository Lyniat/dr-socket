@echo off

set OS_TYPE="windows"
set BUILD_TYPE="Debug"

if %1=="--release" set BUILD_TYPE="Release"

cmake -S. -Bcmake-build-%OS_TYPE%-%BUILD_TYPE% -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -G "MinGW Makefiles"
cmake --build cmake-build-%OS_TYPE%-%BUILD_TYPE% -j 8