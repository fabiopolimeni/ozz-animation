if(NOT ozz_build_samples)
  return()
endif()

# Options for Vulkan backend
option(ozz_build_with_vulkan "Use vulkan as renderer for the samples" ON)
set(ozz_vulkan_shader_directory "${CMAKE_SOURCE_DIR}/samples/framework/shaders/vulkan")

if (WIN32 AND ozz_build_with_vulkan)
  add_custom_target(vulkan_shaders
    COMMAND ${CMAKE_COMMAND} -E make_directory shaders
    COMMAND ${CMAKE_COMMAND} -E copy "${ozz_vulkan_shader_directory}/vert.spv" "./shaders/vert.spv"
    COMMAND ${CMAKE_COMMAND} -E copy "${ozz_vulkan_shader_directory}/frag.spv" "./shaders/frag.spv")
endif()

# Adds the sample framework to the include path.
include_directories(./)

# Adds framework.
add_subdirectory(framework)

# Adds samples and helper libraries.
add_subdirectory(additive)
add_subdirectory(attach)
add_subdirectory(baked)
add_subdirectory(blend)
add_subdirectory(millipede)
add_subdirectory(multithread)
add_subdirectory(optimize)
add_subdirectory(partial_blend)
add_subdirectory(playback)
