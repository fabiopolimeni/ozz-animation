include_directories(${CMAKE_SOURCE_DIR}/extern/glfw/include)

# Samples requires OpenGL/Vulkan package.
if(NOT EMSCRIPTEN)

  # Vulkan is supported on desktop windows only (so far)
  if (WIN32 AND ozz_build_with_vulkan)
    find_package(Vulkan)
    if (VULKAN_FOUND)

      include_directories(${VULKAN_INCLUDE_DIR})
      add_definitions(-DOZZ_FRAMEWORK_VULKAN_RENDERER)

      set(ozz_renderer_source 
        internal/renderer_vulkan.h
        internal/renderer_vulkan.cc
	internal/context_vulkan.h
	internal/context_vulkan.cc)

      set(ozz_libs_to_link
        glfw)

    else()
      set(ozz_build_with_vulkan OFF)
    endif()
  endif()

  if(NOT ozz_build_with_vulkan)
    find_package(OpenGL REQUIRED)

    include_directories(${OPENGL_INCLUDE_DIR})
    include_directories(${CMAKE_SOURCE_DIR}/extern/glad/include)

    set(ozz_renderer_source 
          internal/immediate_opengl.h
          internal/immediate_opengl.cc
          internal/imgui_opengl.h
          internal/imgui_opengl.cc
          internal/renderer_opengl.h
          internal/renderer_opengl.cc
          internal/shader_opengl.h
          internal/shader_opengl.cc
          internal/shooter_opengl.h
          internal/shooter_opengl.cc)

    set(ozz_libs_to_link
      ${OPENGL_LIBRARIES}
      glfw
      glad)

    add_subdirectory(${CMAKE_SOURCE_DIR}/extern/glad glad)
  endif()

  add_subdirectory(${CMAKE_SOURCE_DIR}/extern/glfw glfw)

endif()

add_library(sample_framework
  application.h
  application.cc
  imgui.h
  image.h
  image.cc
  profile.h
  profile.cc
  renderer.h
  immediate.h
  utils.h
  utils.cc
  mesh.h
  mesh.cc
  shooter.h
  camera.h
  camera.cc
  ${ozz_renderer_source})

target_link_libraries(sample_framework
      ${ozz_libs_to_link})

target_link_libraries(sample_framework
  ozz_geometry
  ozz_animation
  ozz_options
  ozz_base)

if(TARGET BUILD_DATA_SAMPLE)
  add_dependencies(sample_framework BUILD_DATA_SAMPLE)
endif()

set_target_properties(sample_framework
  PROPERTIES FOLDER "samples")
  
add_subdirectory(tools)
