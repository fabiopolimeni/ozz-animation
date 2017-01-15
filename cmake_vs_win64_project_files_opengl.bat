@echo off
mkdir build-opengl
pushd build-opengl
cmake -G "Visual Studio 14 Win64" -Dozz_build_with_vulkan=OFF ../
popd
