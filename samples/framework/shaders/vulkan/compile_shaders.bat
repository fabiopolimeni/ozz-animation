@echo off

C:/VulkanSDK/1.0.37.0/Bin/glslangValidator.exe -V model_shader.vert -o model_vert.spv
C:/VulkanSDK/1.0.37.0/Bin/glslangValidator.exe -V model_shader.frag -o model_frag.spv
C:/VulkanSDK/1.0.37.0/Bin/glslangValidator.exe -V line_shader.vert -o line_vert.spv
C:/VulkanSDK/1.0.37.0/Bin/glslangValidator.exe -V line_shader.frag -o line_frag.spv

copy model_vert.spv ..\..\..\..\build-vulkan\samples\shaders\model_vert.spv
copy model_frag.spv ..\..\..\..\build-vulkan\samples\shaders\model_frag.spv
copy line_vert.spv ..\..\..\..\build-vulkan\samples\shaders\line_vert.spv
copy line_frag.spv ..\..\..\..\build-vulkan\samples\shaders\line_frag.spv
pause