@echo off

C:/VulkanSDK/1.0.37.0/Bin/glslangValidator.exe -V shader.vert
C:/VulkanSDK/1.0.37.0/Bin/glslangValidator.exe -V shader.frag

copy vert.spv ..\..\..\..\build-vulkan\samples\shaders\vert.spv
copy frag.spv ..\..\..\..\build-vulkan\samples\shaders\frag.spv
pause