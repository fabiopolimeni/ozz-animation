if(NOT ozz_build_samples)
  return()
endif()

# Options for Vulkan backend
set(ozz_build_with_vulkan ON CACHE BOOL "Use vulkan as renderer for the samples")

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
