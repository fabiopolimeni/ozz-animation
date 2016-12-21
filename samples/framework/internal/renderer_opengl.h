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

#ifndef OZZ_SAMPLES_FRAMEWORK_INTERNAL_RENDERER_OPENGL_H_
#define OZZ_SAMPLES_FRAMEWORK_INTERNAL_RENDERER_OPENGL_H_

#ifndef OZZ_INCLUDE_PRIVATE_HEADER
#error "This header is private, it cannot be included from public headers."
#endif  // OZZ_INCLUDE_PRIVATE_HEADER

// GL and GL ext requires that ptrdif_t is defined on APPLE platforms.
#include <cstddef>

// Don't allow gl.h to automatically include glext.h
#define GL_GLEXT_LEGACY

#ifdef _WIN32
#define NOMINMAX
#endif

// Including glfw includes gl.h
#include "glad/glad.h"

#include "framework/renderer.h"
#include "ozz/base/containers/vector.h"

// Provides helper macro to test for glGetError on a gl call.
#ifndef NDEBUG
#define GL(_f)                    \
  do {                            \
    gl##_f;                       \
    GLenum error = glGetError();  \
    assert(error == GL_NO_ERROR); \
  \
} while (void(0), 0)
#else  // NDEBUG
#define GL(_f) gl##_f
#endif  // NDEBUG

// Convenient macro definition for specifying buffer offsets.
#define GL_PTR_OFFSET(i) reinterpret_cast<void*>(static_cast<intptr_t>(i))

namespace ozz {
namespace animation {
class Skeleton;
}
namespace math {
struct Float4x4;
}
namespace sample {
	class Camera;

namespace internal {
class ShaderOpenGL;
class SkeletonShader;
class AmbientShader;
class AmbientTexturedShader;
class AmbientShaderInstanced;
class GlImmediateRenderer;

// Implements Renderer interface.
class RendererOpenGL : public ozz::sample::Renderer {
 public:
  RendererOpenGL(Camera* _camera);
  virtual ~RendererOpenGL() override;

  // See Renderer for all the details about the API.
  virtual bool Initialize() override;

  virtual void OnResize(int32_t _width, int32_t _height) override;
  
  virtual void DrawAxes(const ozz::math::Float4x4& _transform) override;

  virtual void DrawGrid(int _cell_count, float _cell_size) override;

  virtual bool DrawSkeleton(const animation::Skeleton& _skeleton,
                            const ozz::math::Float4x4& _transform,
                            bool _draw_joints) override;

  virtual bool DrawPosture(const animation::Skeleton& _skeleton,
                           ozz::Range<const ozz::math::Float4x4> _matrices,
                           const ozz::math::Float4x4& _transform,
                           bool _draw_joints) override;

  virtual bool DrawBoxIm(const ozz::math::Box& _box,
                         const ozz::math::Float4x4& _transform,
                         const Color _colors[2]) override;

  virtual bool DrawBoxShaded(const ozz::math::Box& _box,
                             ozz::Range<const ozz::math::Float4x4> _transforms,
                             Color _color) override;

  virtual bool DrawSkinnedMesh(const Mesh& _mesh,
                               const Range<math::Float4x4> _skinning_matrices,
                               const ozz::math::Float4x4& _transform,
                               const Options& _options = Options()) override;

  virtual bool DrawMesh(const Mesh& _mesh,
                        const ozz::math::Float4x4& _transform,
                        const Options& _options = Options()) override;

  virtual bool DrawVectors(ozz::Range<const float> _positions,
                           size_t _positions_stride,
                           ozz::Range<const float> _directions,
                           size_t _directions_stride, int _num_vectors,
                           float _vector_length, Renderer::Color _color,
                           const ozz::math::Float4x4& _transform) override;

  virtual bool DrawBinormals(
      ozz::Range<const float> _positions, size_t _positions_stride,
      ozz::Range<const float> _normals, size_t _normals_stride,
      ozz::Range<const float> _tangents, size_t _tangents_stride,
      ozz::Range<const float> _handenesses, size_t _handenesses_stride,
      int _num_vectors, float _vector_length, Renderer::Color _color,
      const ozz::math::Float4x4& _transform) override;

  // Get application camera that provides rendering matrices.
  ozz::sample::Camera* camera() const { return camera_; }

 private:
  // Defines the internal structure used to define a model.
  struct Model {
    Model();
    ~Model();

    GLuint vbo;
    GLenum mode;
    GLsizei count;
    SkeletonShader* shader;
  };

  // Detects and initializes all OpenGL extension.
  // Return true if all mandatory extensions were found.
  bool InitOpenGLExtensions();

  // Initializes posture rendering.
  // Return true if initialization succeeded.
  bool InitPostureRendering();

  // Initializes the checkered texture.
  // Return true if initialization succeeded.
  bool InitCheckeredTexture();

  // Draw posture internal non-instanced rendering fall back implementation.
  void DrawPosture_Impl(const ozz::math::Float4x4& _transform,
                        const float* _uniforms, int _instance_count,
                        bool _draw_joints);

  // Draw posture internal instanced rendering implementation.
  void DrawPosture_InstancedImpl(const ozz::math::Float4x4& _transform,
                                 const float* _uniforms, int _instance_count,
                                 bool _draw_joints);

  // Array of matrices used to store model space matrices during DrawSkeleton
  // execution.
  ozz::Range<ozz::math::Float4x4> prealloc_models_;

  // Application camera that provides rendering matrices.
  ozz::sample::Camera* camera_;

  // Bone and joint model objects.
  Model models_[2];

  // Dynamic vbo used for arrays.
  GLuint dynamic_array_bo_;

  // Dynamic vbo used for indices.
  GLuint dynamic_index_bo_;

  // Volatile memory buffer that can be used within function scope.
  class ScratchBuffer {
   public:
    ScratchBuffer();
    ~ScratchBuffer();

    // Resizes the buffer to the new size and return the memory address.
    void* Resize(size_t _size);

   private:
    void* buffer_;
    size_t size_;
  };
  ScratchBuffer scratch_buffer_;

  // Ambient rendering shader.
  AmbientShader* ambient_shader;
  AmbientTexturedShader* ambient_textured_shader;
  AmbientShaderInstanced* ambient_shader_instanced;

  // Checkered texture
  unsigned int checkered_texture_;
};
}  // internal
}  // sample
}  // ozz

#endif  // OZZ_SAMPLES_FRAMEWORK_INTERNAL_RENDERER_OPENGL_H_
