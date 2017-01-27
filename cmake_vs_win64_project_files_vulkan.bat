@echo off
if exist ".\build-vulkan" rmdir /S /Q .\build-vulkan

mkdir build-vulkan
pushd build-vulkan
cmake -G "Visual Studio 14 Win64" -Dozz_build_with_vulkan=ON ../
popd
