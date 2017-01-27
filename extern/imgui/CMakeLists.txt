add_library(imgui
  imconfig.h
  imgui.cpp
  imgui.h
  imgui_demo.cpp
  imgui_draw.cpp
  imgui_internal.h
  stb_rect_pack.h
  stb_textedit.h
  stb_truetype.h)

set_target_properties(imgui
  PROPERTIES FOLDER "extern")
