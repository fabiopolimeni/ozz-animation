//----------------------------------------------------------------------------//
//                                                                            //
// ozz-animation is hosted at http://github.com/guillaumeblanc/ozz-animation  //
// and distributed under the MIT License (MIT).                               //
//                                                                            //
// Copyright (c) 2015 Guillaume Blanc                                         //
//                                                                            //
// Permission is hereby granted, free of charge, to any person obtaining a    //
// copy of this software and associated documentation files (the "Software"), //
// to deal in the Software without restriction, including without limitation  //
// the rights to use, copy, modify, merge, publish, distribute, sublicense,   //
// and/or sell copies of the Software, and to permit persons to whom the      //
// Software is furnished to do so, subject to the following conditions:       //
//                                                                            //
// The above copyright notice and this permission notice shall be included in //
// all copies or substantial portions of the Software.                        //
//                                                                            //
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR //
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   //
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    //
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER //
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    //
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER        //
// DEALINGS IN THE SOFTWARE.                                                  //
//                                                                            //
//----------------------------------------------------------------------------//

#define OZZ_INCLUDE_PRIVATE_HEADER  // Allows to include private headers.

#include "framework/application.h"

#include <cassert>
#include <cstdlib>
#include <cstring>

#ifdef __APPLE__
#include <unistd.h>
#endif  // __APPLE__

#if EMSCRIPTEN
#include "emscripten.h"
#endif  // EMSCRIPTEN

#include "framework/image.h"
#include "framework/imgui.h"
#include "framework/camera.h"
#include "framework/shooter.h"
#include "framework/profile.h"
#include "framework/renderer.h"

#ifdef OZZ_FRAMEWORK_VULKAN_RENDERER
#include "framework/internal/renderer_vulkan.h"
#else
#include "framework/internal/imgui_opengl.h"
#include "framework/internal/renderer_opengl.h"
#include "framework/internal/shader_opengl.h"
#include "framework/internal/shooter_opengl.h"
#endif

#include "glfw/glfw3.h"

#include "ozz/base/io/stream.h"
#include "ozz/base/log.h"
#include "ozz/base/maths/box.h"
#include "ozz/base/memory/allocator.h"
#include "ozz/options/options.h"
#include "ozz/base/maths/rect.h"

OZZ_OPTIONS_DECLARE_INT(
    max_idle_loops,
    "The maximum number of idle loops the sample application can perform."
    " Application automatically exit when this number of loops is reached."
    " A negative value disables this feature.",
    -1, false);

OZZ_OPTIONS_DECLARE_BOOL(render, "Enables sample redering.", true, false);

namespace {
// Screen resolution presets.
const ozz::sample::Resolution resolution_presets[] = {
    {640, 360},   {640, 480},  {800, 450},  {800, 600},   {1024, 576},
    {1024, 768},  {1280, 720}, {1280, 800}, {1280, 960},  {1280, 1024},
    {1400, 1050}, {1440, 900}, {1600, 900}, {1600, 1200}, {1680, 1050},
    {1920, 1080}, {1920, 1200}};
const int kNumPresets = OZZ_ARRAY_SIZE(resolution_presets);
}

// Check resolution argument is within 0 - kNumPresets
static bool ResolutionCheck(const ozz::options::Option& _option,
                            int /*_argc*/) {
  const ozz::options::IntOption& option =
      static_cast<const ozz::options::IntOption&>(_option);
  return option >= 0 && option < kNumPresets;
}

OZZ_OPTIONS_DECLARE_INT_FN(resolution, "Resolution index (0 to 17).", 5, false,
                           &ResolutionCheck);

GLFWwindow* g_glfwWindow = nullptr;

void glfw_error_callback(int _error, const char* _description)
{
	ozz::log::Err() << "GLFW Error! code: " << _error << " msg: "
		<< _description << std::endl;
}

void glfw_key_callback(GLFWwindow* _window, int _key, int /*_scancode*/,
	int _action, int /*_mods*/)
{
	if (_key == GLFW_KEY_ESCAPE && _action == GLFW_PRESS)
		glfwSetWindowShouldClose(_window, GLFW_TRUE);
}

void glfw_resize_callback(GLFWwindow* /*_window*/, int _width, int _height) {
	// Stores new resolution settings.
	auto* application_ = ozz::sample::Application::GetInstance();
	application_->SetResolution({ _width, _height });
}


void glfw_fb_resize_callback(GLFWwindow* /*_window*/, int _width, int _height) {
	// Stores new resolution settings.
	auto* application_ = ozz::sample::Application::GetInstance();
	application_->SetResolution({ _width, _height });

	application_->GetRenderer()->OnResize(_width, _height);

	// Forwards screen size to camera and shooter.
	application_->GetCamera()->Resize(_width, _height);
	application_->GetShooter()->Resize(_width, _height);
}

void glfw_close_callback(GLFWwindow* /*_window*/) {
	auto* application_ = ozz::sample::Application::GetInstance();
	application_->RequestExit();
}

namespace ozz {
namespace sample {
Application* Application::application_ = NULL;

Application::Application()
    : exit_(false),
      freeze_(false),
      fix_update_rate(false),
      fixed_update_rate(60.f),
      time_factor_(1.f),
      last_idle_time_(0.),
      camera_(NULL),
      shooter_(NULL),
      show_help_(false),
      show_grid_(true),
      show_axes_(true),
      capture_video_(false),
      capture_screenshot_(false),
      renderer_(NULL),
      im_gui_(NULL),
      fps_(memory::default_allocator()->New<Record>(128)),
      update_time_(memory::default_allocator()->New<Record>(128)),
      render_time_(memory::default_allocator()->New<Record>(128)),
      resolution_(resolution_presets[0]) {
#ifndef NDEBUG
  // Assert presets are correctly sorted.
  for (int i = 1; i < kNumPresets; ++i) {
    const Resolution& preset_m1 = resolution_presets[i - 1];
    const Resolution& preset = resolution_presets[i];
    assert(preset.width > preset_m1.width || preset.height > preset_m1.height);
  }
#endif  //  NDEBUG
}

Application::~Application() {
  memory::default_allocator()->Delete(fps_);
  memory::default_allocator()->Delete(update_time_);
  memory::default_allocator()->Delete(render_time_);
}

int Application::Run(int _argc, const char** _argv, const char* _version,
                     const char* _title) {
  // Only one application at a time can be ran.
  if (application_) {
    return EXIT_FAILURE;
  }
  application_ = this;

  // Starting application
  log::Out() << "Starting sample \"" << _title << "\" version \"" << _version
             << "\"" << std::endl;
  log::Out() << "Ozz libraries were built with \""
             << math::SimdImplementationName() << "\" SIMD math implementation."
             << std::endl;

  // Parse command line arguments.
  const char* usage = "Ozz animation sample. See README.md file for more details.";
  ozz::options::ParseResult result =
      ozz::options::ParseCommandLine(_argc, _argv, _version, usage);
  if (result != ozz::options::kSuccess) {
    exit_ = true;
    return result == ozz::options::kExitSuccess ? EXIT_SUCCESS : EXIT_FAILURE;
  }

  // Fetch initial resolution.
  resolution_ = resolution_presets[OPTIONS_resolution];

#ifdef __APPLE__
  // On OSX, when run from Finder, working path is the root path. This does not
  // allow to load resources from relative path.
  // The workaround is to change the working directory to application directory.
  // The proper solution would probably be to use bundles and load data from
  // resource folder.
  chdir(ozz::options::ParsedExecutablePath().c_str());
#endif  // __APPLE__

  // Initialize help.
  ParseReadme();

  // Initialise GLFW and create the main window
  glfwSetErrorCallback(glfw_error_callback);

  // Open an OpenGL window
  bool success = true;
  if (OPTIONS_render) {
    // Initialize GLFW
    if (!glfwInit()) {
      application_ = NULL;
      return EXIT_FAILURE;
    }

#ifdef OZZ_FRAMEWORK_VULKAN_RENDERER
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);		// This tells GLFW to not create an OpenGL context with the window
	if (!glfwVulkanSupported())
	{
		// Vulkan is not available
		log::Err() << "Vulkan is not available, exit!" << std::endl;

		glfwTerminate();
		success = false;
		return EXIT_FAILURE;
	}
#else
    // Setup GL context.
    const int gl_version_major = 2, gl_version_minor = 0;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gl_version_major);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gl_version_minor);
	glfwWindowHint(GLFW_SAMPLES, 4);

#ifndef NDEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif  // NDEBUG

#endif

	g_glfwWindow = glfwCreateWindow(resolution_.width, resolution_.height, "", nullptr, nullptr);
	if (g_glfwWindow == nullptr)
	{
		log::Err() << "Failed to create window!" << std::endl;

		glfwTerminate();
		success = false;
		return EXIT_FAILURE;
	}

	title_ = _title;

#ifndef EMSCRIPTEN // Better not rename web page.
	glfwSetWindowTitle(g_glfwWindow, _title);
#endif
	
	// Allocates and initializes internal objects.
	camera_ = memory::default_allocator()->New<Camera>();

#ifdef OZZ_FRAMEWORK_VULKAN_RENDERER
	renderer_ = memory::default_allocator()->New<internal::RendererVulkan>(camera_);
	success = renderer_->Initialize();
#else
	// Loading OpenGL extensions
	glfwMakeContextCurrent(g_glfwWindow);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	log::Out() << "Successfully initialized OpenGL \""
		<< glGetString(GL_VERSION) << "\"." << std::endl;

	renderer_ = memory::default_allocator()->New<internal::RendererOpenGL>(camera_);
	success = renderer_->Initialize();

	if (success) {
		shooter_ = memory::default_allocator()->New<internal::ShooterOpenGL>();
		im_gui_ = memory::default_allocator()->New<internal::ImGuiOpenGL>();
	}
	else {
		log::Err() << "Couldn't initialize OpenGL renderer" << std::endl;
	}
	
	if (success) {
		glfwSwapInterval(1);  // Enables vertical sync by default.
	}
#endif

	if (success) {
      // Setup the window and installs callbacks.
	  glfwSetKeyCallback(g_glfwWindow, glfw_key_callback);
	  glfwSetWindowSizeCallback(g_glfwWindow, &glfw_resize_callback);
	  glfwSetFramebufferSizeCallback(g_glfwWindow, &glfw_fb_resize_callback);
	  glfwSetWindowCloseCallback(g_glfwWindow, &glfw_close_callback);

	  {
		  // The first time the window is created, it doesn't seem
		  // to fire the resize callback, therefore we need to call
		  // it manually, otherwise we will never set the viewport
		  // but manually resizing the window.
	     int width_, height_;
	     glfwGetFramebufferSize(g_glfwWindow, &width_, &height_);
	     glfw_resize_callback(g_glfwWindow, width_, height_);
	  }
       
      // Loop the sample.
      success = Loop();
       
      memory::default_allocator()->Delete(shooter_);
      shooter_ = NULL;
      memory::default_allocator()->Delete(im_gui_);
      im_gui_ = NULL;
   }

    memory::default_allocator()->Delete(renderer_);
    renderer_ = NULL;
    memory::default_allocator()->Delete(camera_);
    camera_ = NULL;

    // Closes window and terminates GLFW.
	glfwDestroyWindow(g_glfwWindow);
    glfwTerminate();

  } else {
    // Loops without any rendering initialization.
    success = Loop();
  }

  // Notifies that an error occurred.
  if (!success) {
    log::Err() << "An error occurred during sample execution." << std::endl;
  }

  application_ = NULL;
  return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

void OneLoopCbk(void* _arg) {
  Application* app = reinterpret_cast<Application*>(_arg);
  static int loops = 0;
  app->OneLoop(loops++);
}

Application::LoopStatus Application::OneLoop(int _loops) {
  Profiler profile(fps_);  // Profiles frame.
  
  // Tests for a manual exit request.
  if (exit_ || glfwGetKey(g_glfwWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    return kBreak;
  }

  // Test for an exit request.
  if (OPTIONS_max_idle_loops > 0 && _loops > OPTIONS_max_idle_loops) {
    return kBreak;
  }

  // exit request from the window
  if (glfwWindowShouldClose(g_glfwWindow)) {
	return kBreak;
  }

// Don't overload the cpu if the window is not visible.
#ifndef EMSCRIPTEN
  int _visible = glfwGetWindowAttrib(g_glfwWindow, GLFW_VISIBLE);
  if (OPTIONS_render && _visible == 0) {
    glfwWaitEvents();  // Wait...

    // Reset last update time in order to stop the time while the app isn't
    // active.
    last_idle_time_ = glfwGetTime();

    return kContinue;  // ...but don't do anything.
  }
#else   // EMSCRIPTEN
  // Detect canvas resizing which isn't automatically detected.
  int width, height, fullscreen;
  emscripten_get_canvas_size(&width, &height, &fullscreen);
  if (width != resolution_.width || height != resolution_.height) {
    glfw_resize_callback(g_glfwWindow, width, height);
  }
#endif  // EMSCRIPTEN

  // Enable/disable help on F1 key.
  static int previous_f1 = glfwGetKey(g_glfwWindow, GLFW_KEY_F1);
  const int f1 = glfwGetKey(g_glfwWindow, GLFW_KEY_F1);
  if (previous_f1 == GLFW_RELEASE && f1 == GLFW_PRESS) {
    show_help_ = !show_help_;
  }
  previous_f1 = f1;

  // Do the main loop.
  if (!Idle(_loops == 0)) {
    return kBreakFailure;
  }

  // Skips display if "no_render" option is enabled.
  if (OPTIONS_render) {
    if (!Display()) {
      return kBreakFailure;
    }
  }

  // Give the chance to process inputs
  glfwPollEvents();

  return kContinue;
}

bool Application::Loop() {
  // Initialize sample.
  bool success = OnInitialize();

// Emscripten requires to manage the main loop on their own, as browsers don't
// like infinite blocking functions.
#ifdef EMSCRIPTEN
  emscripten_set_main_loop_arg(OneLoopCbk, this, 0, 1);
#else   // EMSCRIPTEN
  // Loops.
  for (int loops = 0; success; ++loops) {
    const LoopStatus status = OneLoop(loops);
    success = status != kBreakFailure;
    if (status != kContinue) {
      break;
    }
  }
#endif  // EMSCRIPTEN

  // De-initialize sample, even in case of initialization failure.
  OnDestroy();

  return success;
}

bool Application::Display() {
  assert(OPTIONS_render);

  bool success = true;

  {  // Profiles rendering excluding GUI.
    Profiler profile(render_time_);

#ifdef OZZ_FRAMEWORK_VULKAN_RENDERER
#else
    GL(ClearDepth(1.f));
    GL(ClearColor(.4f, .42f, .38f, 0.f));
    GL(Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    // Setup default states
    GL(Enable(GL_CULL_FACE));
    GL(CullFace(GL_BACK));
    GL(Enable(GL_DEPTH_TEST));
    GL(DepthMask(GL_TRUE));
    GL(DepthFunc(GL_LEQUAL));
#endif

    // Bind 3D camera matrices.
    camera_->Bind3D();

    // Forwards display event to the inheriting application.
    if (success) {
      success = OnDisplay(renderer_);
    }
  }  // Ends profiling.

  // Renders grid and axes at the end as they are transparent.
  if (show_grid_) {
    renderer_->DrawGrid(20, 1.f);
  }
  if (show_axes_) {
    renderer_->DrawAxes(ozz::math::Float4x4::identity());
  }

  // Bind 2D camera matrices.
  camera_->Bind2D();

  // Forwards gui event to the inheriting application.
  if (success) {
    success = Gui();
  }

  // Capture back buffer.
  if (capture_screenshot_ || capture_video_) {
    shooter_->Capture(GL_BACK);
  }

#ifndef OZZ_FRAMEWORK_VULKAN_RENDERER
  // Swaps current window.
  glfwSwapBuffers(g_glfwWindow);
#endif

  return success;
}

bool Application::Idle(bool _first_frame) {
  // Early out if displaying help.
  if (show_help_) {
    last_idle_time_ = glfwGetTime();
    return true;
  }

  // Compute elapsed time since last idle, and delta time.
  float delta;
  double time = glfwGetTime();
  if (_first_frame ||  // Don't take into account time spent initializing.
      time == 0.) {    // Means glfw isn't initialized (rendering's disabled).
    delta = 1.f / 60.f;
  } else {
    delta = static_cast<float>(time - last_idle_time_);
  }
  last_idle_time_ = time;

  // Update dt, can be scaled, fixed, freezed...
  float update_delta;
  if (freeze_) {
    update_delta = 0.f;
  } else {
    if (fix_update_rate) {
      update_delta = 1.f / fixed_update_rate;
    } else {
      update_delta = delta * time_factor_;
    }
  }

  // Updates screen shooter object.
  if (shooter_) {
    shooter_->Update();
  }

  // Forwards update event to the inheriting application.
  bool update_result;
  {  // Profiles update scope.
    Profiler profile(update_time_);
    update_result = OnUpdate(update_delta);
  }

  // Update camera model-view matrix.
  if (camera_) {
    math::Box scene_bounds;
    GetSceneBounds(&scene_bounds);

    math::Float4x4 camera_transform;
    if (GetCameraOverride(&camera_transform)) {
      camera_->Update(camera_transform, scene_bounds, delta, _first_frame);
    } else {
      camera_->Update(scene_bounds, delta, _first_frame);
    }
  }

  return update_result;
}

bool Application::Gui() {
  bool success = true;
  const float kFormWidth = 200.f;
  const float kHelpMargin = 16.f;

  // Finds gui area.
  const float kGuiMargin = 2.f;
  ozz::math::RectInt window_rect(0, 0, resolution_.width, resolution_.height);

  // Fills ImGui's input structure.
  ImGui::Inputs input;
  double xpos, ypos;
  glfwGetCursorPos(g_glfwWindow, &xpos, &ypos);
  
  input.mouse_x = (int)xpos;
  input.mouse_y = window_rect.height - (int)ypos;
  input.lmb_pressed =
	  glfwGetMouseButton(g_glfwWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

  // Starts frame
  if (im_gui_)
  {
	  im_gui_->BeginFrame(input, window_rect, renderer_);

	  // Help gui.
	  {
		  math::RectFloat rect(kGuiMargin, kGuiMargin,
			  window_rect.width - kGuiMargin * 2.f,
			  window_rect.height - kGuiMargin * 2.f);
		  ImGui::Form form(im_gui_, "Show help", rect, &show_help_, false);
		  if (show_help_) {
			  im_gui_->DoLabel(help_.c_str(), ImGui::kLeft, false);
		  }
	  }

	  // Do framework gui.
	  if (!show_help_ && success &&
		  window_rect.width > (kGuiMargin + kFormWidth) * 2.f) {
		  static bool open = true;
		  math::RectFloat rect(kGuiMargin, kGuiMargin, kFormWidth,
			  window_rect.height - kGuiMargin * 2.f - kHelpMargin);
		  ImGui::Form form(im_gui_, "Framework", rect, &open, true);
		  if (open) {
			  success = FrameworkGui();
		  }
	  }

	  // Do sample gui.
	  if (!show_help_ && success && window_rect.width > kGuiMargin + kFormWidth) {
		  static bool open = true;
		  math::RectFloat rect(window_rect.width - kFormWidth - kGuiMargin,
			  kGuiMargin, kFormWidth,
			  window_rect.height - kGuiMargin * 2 - kHelpMargin);
		  ImGui::Form form(im_gui_, "Sample", rect, &open, true);
		  if (open) {
			  // Forwards event to the inherited application.
			  success = OnGui(im_gui_);
		  }
	  }

	  // Ends frame
	  im_gui_->EndFrame();
  }

  return success;
}

bool Application::FrameworkGui() {
  {  // Render statistics
    static bool open = true;
    ImGui::OpenClose stat_oc(im_gui_, "Statistics", &open);
    if (open) {
      char szLabel[64];
      {  // FPS
        Record::Statistics statistics = fps_->GetStatistics();
        std::sprintf(szLabel, "FPS: %.0f",
                     statistics.mean == 0.f ? 0.f : 1000.f / statistics.mean);
        static bool fps_open = false;
        ImGui::OpenClose stats(im_gui_, szLabel, &fps_open);
        if (fps_open) {
          std::sprintf(szLabel, "Frame: %.2f ms", statistics.mean);
          im_gui_->DoGraph(szLabel, 0.f, statistics.max, statistics.latest,
                           fps_->cursor(), fps_->record_begin(),
                           fps_->record_end());
        }
      }
      {  // Update time
        Record::Statistics statistics = update_time_->GetStatistics();
        std::sprintf(szLabel, "Update: %.2f ms", statistics.mean);
        static bool update_open = true;  // This is the most relevant for ozz.
        ImGui::OpenClose stats(im_gui_, szLabel, &update_open);
        if (update_open) {
          im_gui_->DoGraph(NULL, 0.f, statistics.max, statistics.latest,
                           update_time_->cursor(), update_time_->record_begin(),
                           update_time_->record_end());
        }
      }
      {  // Render time
        Record::Statistics statistics = render_time_->GetStatistics();
        std::sprintf(szLabel, "Render: %.2f ms", statistics.mean);
        static bool render_open = false;
        ImGui::OpenClose stats(im_gui_, szLabel, &render_open);
        if (render_open) {
          im_gui_->DoGraph(NULL, 0.f, statistics.max, statistics.latest,
                           render_time_->cursor(), render_time_->record_begin(),
                           render_time_->record_end());
        }
      }
    }
  }

  {  // Time control
    static bool open = false;
    ImGui::OpenClose stats(im_gui_, "Time control", &open);
    if (open) {
      im_gui_->DoButton("Freeze", true, &freeze_);
      im_gui_->DoCheckBox("Fix update rate", &fix_update_rate, true);
      if (!fix_update_rate) {
        char sz_factor[64];
        std::sprintf(sz_factor, "Time factor: %.2f", time_factor_);
        im_gui_->DoSlider(sz_factor, -5.f, 5.f, &time_factor_);
        if (im_gui_->DoButton("Reset time factor", time_factor_ != 1.f)) {
          time_factor_ = 1.f;
        }
      } else {
        char sz_fixed_update_rate[64];
        std::sprintf(sz_fixed_update_rate, "Update rate: %.0f fps",
                     fixed_update_rate);
        im_gui_->DoSlider(sz_fixed_update_rate, 1.f, 200.f, &fixed_update_rate,
                          .5f, true);
        if (im_gui_->DoButton("Reset update rate", fixed_update_rate != 60.f)) {
          fixed_update_rate = 60.f;
        }
      }
    }
  }

  {  // Rendering options
    static bool open = false;
    ImGui::OpenClose options(im_gui_, "Options", &open);
    if (open) {
      // Multi-sampling.
      static bool fsaa_available =
		  glfwGetWindowAttrib(g_glfwWindow, GLFW_SAMPLES) != 0;
      static bool fsaa_enabled = fsaa_available;

#ifndef OZZ_FRAMEWORK_VULKAN_RENDERER
	  if (im_gui_->DoCheckBox("Anti-aliasing", &fsaa_enabled, fsaa_available)) {
		  if (fsaa_enabled) {
			  GL(Enable(GL_MULTISAMPLE));
		  }
		  else {
			  GL(Disable(GL_MULTISAMPLE));
		  }
	  }
#endif

      // Vertical sync
      static bool vertical_sync_ = true;  // On by default.
      if (im_gui_->DoCheckBox("Vertical sync", &vertical_sync_, true)) {
        glfwSwapInterval(vertical_sync_ ? 1 : 0);
      }

      im_gui_->DoCheckBox("Show grid", &show_grid_, true);
      im_gui_->DoCheckBox("Show axes", &show_axes_, true);
    }

    // Searches for matching resolution settings.
    int preset_lookup = 0;
    for (; preset_lookup < kNumPresets - 1; ++preset_lookup) {
      const Resolution& preset = resolution_presets[preset_lookup];
      if (preset.width > resolution_.width) {
        break;
      } else if (preset.width == resolution_.width) {
        if (preset.height >= resolution_.height) {
          break;
        }
      }
    }

    char szResolution[64];
    std::sprintf(szResolution, "Resolution: %dx%d", resolution_.width,
                 resolution_.height);
    if (im_gui_->DoSlider(szResolution, 0, kNumPresets - 1, &preset_lookup)) {
      // Resolution changed.
      resolution_ = resolution_presets[preset_lookup];
      glfwSetWindowSize(g_glfwWindow, resolution_.width, resolution_.height);
    }
  }

  {  // Capture
    static bool open = false;
    ImGui::OpenClose controls(im_gui_, "Capture", &open);
    if (open) {
      im_gui_->DoButton("Capture video", true, &capture_video_);
      capture_screenshot_ =
          im_gui_->DoButton("Capture screenshot", !capture_video_);
    }
  }

  {  // Controls
    static bool open = false;
    ImGui::OpenClose controls(im_gui_, "Camera controls", &open);
    if (open) {
      camera_->OnGui(im_gui_);
    }
  }
  return true;
}

// Default implementation doesn't override camera location.
bool Application::GetCameraOverride(math::Float4x4* _transform) const {
  (void)_transform;
  assert(_transform);
  return false;
}

void Application::ParseReadme() {
  const char* error_message = "Unable to find README.md help file.";

  // Get README file, opens as binary to avoid conversions.
  ozz::io::File file("README.md", "rb");
  if (!file.opened()) {
    help_ = error_message;
    return;
  }

  // Allocate enough space to store the whole file.
  const size_t read_length = file.Size();
  ozz::memory::Allocator* allocator = ozz::memory::default_allocator();
  char* content = allocator->Allocate<char>(read_length);

  // Read the content
  if (file.Read(content, read_length) == read_length) {
    help_ = ozz::String::Std(content, content + read_length);
  } else {
    help_ = error_message;
  }

  // Deallocate temporary buffer;
  allocator->Deallocate(content);
}
}  // sample
}  // ozz
