add_library(glad
  include/glad/glad.h
  include/KHR/khrplatform.h
  src/glad.c)

#target_link_libraries(glad
#  ${OPENGL_gl_LIBRARY})

set_target_properties(glad
  PROPERTIES FOLDER "extern")
