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

#ifndef OZZ_SAMPLES_FRAMEWORK_INTERNAL_IMMEDIATE_H_
#define OZZ_SAMPLES_FRAMEWORK_INTERNAL_IMMEDIATE_H_

#ifndef OZZ_INCLUDE_PRIVATE_HEADER
#error "This header is private, it cannot be included from public headers."
#endif  // OZZ_INCLUDE_PRIVATE_HEADER

#include "renderer_opengl.h"
#include "framework/immediate.h"

namespace ozz {
namespace sample {

namespace internal {

class RendererOpenGL;
class ImmediatePCShader;
class ImmediatePNShader;
class ImmediatePTCShader;

// Declares supported vertex formats.
// Position + Color.
struct VertexPC {
  float pos[3];
  GLubyte rgba[4];
};

// Declares supported vertex formats.
// Position + Normal.
struct VertexPN {
  float pos[3];
  float normal[3];
};

// Position + Texture coordinate + Color.
struct VertexPTC {
  float pos[3];
  float uv[2];
  GLubyte rgba[4];
};

// Declares Immediate mode types.
template <typename _Ty, ShaderType _St>
class GlImmediate;
typedef GlImmediate<VertexPC, ShaderType::ePC> GlImmediatePC;
typedef GlImmediate<VertexPN, ShaderType::ePN> GlImmediatePN;
typedef GlImmediate<VertexPTC, ShaderType::ePTC> GlImmediatePTC;

// GL immediate mode renderer.
// Should be used with a GlImmediate object to push vertices to the renderer.
class GlImmediateRenderer : public ImmediateRenderer {
 public:
  GlImmediateRenderer(RendererOpenGL* _renderer);
  ~GlImmediateRenderer();

  // Initializes immediate  mode renderer. Can fail.
  virtual bool Initialize() override;

 private:
  // GlImmediate is used to work with the renderer.
  template <typename _Ty, ShaderType _St>
  friend class GlImmediate;

  // Begin stacking vertices.
  virtual void Begin(ShaderType _shader_t) override;

  // End stacking vertices. Call GL rendering.
  virtual void End(uint32_t _mode, int32_t _vertex_size, const ozz::math::Float4x4& _transform) override;

  // Resize vertex buffer to at least _new_size. This function can only grow
  // vbo size.
  virtual void ResizeVbo(size_t _new_size) override;

protected:

  // Access the current vbo size
  OZZ_INLINE virtual size_t& Size() override { return size_; }
  OZZ_INLINE virtual const size_t& MaxSize() const override { return max_size_; }
  OZZ_INLINE virtual  void GetBuffer(char*& _buffer) override { _buffer = buffer_; }

  // The vertex object used by the renderer.
  GLuint vbo_;

  // Buffer of vertices.
  char* buffer_;
  size_t max_size_;

  // Number of vertices.
  size_t size_;

  ShaderType shader_type_;
  ShaderOpenGL* immediate_shaders[(size_t)ShaderType::eMAX];

  // Immediate mode shaders;
  ImmediatePCShader* immediate_pc_shader;
  ImmediatePTCShader* immediate_ptc_shader;

  // The renderer object.
  RendererOpenGL* renderer_;
};

// RAII object that allows to push vertices to the imrender stack.
template <typename _Ty, ShaderType _St>
class GlImmediate {
 public:
  // Immediate object vertex format.
  typedef _Ty Vertex;

  // Start a new immediate stack.
  GlImmediate(ImmediateRenderer* _renderer, GLenum _mode,
              const ozz::math::Float4x4& _transform)
      : transform_(_transform), renderer_(_renderer), mode_(_mode) {
    renderer_->Begin(_St);
  }

  // End immediate vertex stacking, and renders all vertices.
  ~GlImmediate() { renderer_->End(mode_, sizeof(_Ty), transform_); }

  // Pushes a new vertex to the stack.
  OZZ_INLINE void PushVertex(const _Ty& _vertex) {
    renderer_->PushVertex(_vertex);
  }

 private:
  // Non copyable.
  GlImmediate(const GlImmediate&);
  void operator=(const GlImmediate&);

  // Transformation matrix.
  const ozz::math::Float4x4 transform_;

  // Shared renderer.
  ImmediateRenderer* renderer_;

  // Draw array mode GL_POINTS, GL_LINE_STRIP, ...
  GLenum mode_;
};
}  // internal
}  // sample
}  // ozz
#endif  // OZZ_SAMPLES_FRAMEWORK_INTERNAL_IMMEDIATE_H_
