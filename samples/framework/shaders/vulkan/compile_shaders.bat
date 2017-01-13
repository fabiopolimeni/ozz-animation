@echo off

C:/VulkanSDK/1.0.37.0/Bin/glslangValidator.exe -V shader.vert
C:/VulkanSDK/1.0.37.0/Bin/glslangValidator.exe -V shader.frag

copy vert.spv ..\..\..\..\build\samples\shaders\vert.spv
copy frag.spv ..\..\..\..\build\samples\shaders\frag.spv
pause