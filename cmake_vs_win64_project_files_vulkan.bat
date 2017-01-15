@echo off
mkdir build-vulkan
pushd build-vulkan
cmake -G "Visual Studio 14 Win64" -Dozz_build_with_vulkan=ON ../
popd
