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

#ifndef OZZ_SAMPLES_FRAMEWORK_IMMEDIATE_RENDERER_H_
#define OZZ_SAMPLES_FRAMEWORK_IMMEDIATE_RENDERER_H_

#include "ozz/base/platform.h"
#include "ozz/base/maths/math_ex.h"
#include "ozz/base/maths/simd_math.h"

namespace ozz {
	namespace sample {

enum class ShaderType : uint8_t
{
	ePC,	// position + color
	ePN,	// position + normal
	ePTC,	// position + texture + color
	eMAX
};

class ImmediateRenderer {
public:

	virtual ~ImmediateRenderer() { }

	// Initializes immediate  mode renderer. Can fail.
	virtual bool Initialize() = 0;

	// Begin stacking vertices.
	virtual void Begin(ShaderType _shader_t) = 0;

	// End stacking vertices. Call GL rendering.
	virtual void End(uint32_t _mode, int32_t _vertex_size, const ozz::math::Float4x4& _transform) = 0;

	// Resize vertex buffer to at least _new_size. This function can only grow
	// vbo size.
	virtual void ResizeVbo(size_t _new_size) = 0;

protected:

	// Returns the current vbo size
	virtual size_t& Size() = 0;
	virtual const size_t& MaxSize() const = 0;
	virtual void GetBuffer(char*& _buffer) = 0;

public:

	// Push a new vertex to the buffer.
	template <typename _Ty>
	OZZ_INLINE void PushVertex(const _Ty& _vertex) {
		// Resize buffer if needed.
		size_t& size_ = Size();
		const size_t& max_size_ = MaxSize();
		const size_t new_size = size_ + sizeof(_Ty);
		if (new_size > max_size_) {
			ResizeVbo(new_size);
			size_ = Size();
		}
		// Copy this last vertex.
		char* buffer_ = NULL;
		GetBuffer(buffer_);
		*reinterpret_cast<_Ty*>(&buffer_[size_]) = _vertex;
		size_ = new_size;
	}
};
} // sample
} // ozz

#endif //OZZ_SAMPLES_FRAMEWORK_IMMEDIATE_RENDERER_H_
