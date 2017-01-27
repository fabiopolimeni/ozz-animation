//                                                                            //
// ozz-animation is hosted at http://github.com/guillaumeblanc/ozz-animation  //
// and distributed under the MIT License (MIT).                               //
//                                                                            //
// Copyright (c) 2017 Fabio Polimeni                                          //
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
#ifndef OZZ_SAMPLES_FRAMEWORK_INTERNAL_IMGUI_VULKAN_H_
#define OZZ_SAMPLES_FRAMEWORK_INTERNAL_IMGUI_VULKAN_H_

#ifndef OZZ_INCLUDE_PRIVATE_HEADER
#error "This header is private, it cannot be included from public headers."
#endif  // OZZ_INCLUDE_PRIVATE_HEADER

// Implements immediate mode gui.
// See imgui.h for details about function specifications.

#include "framework/imgui.h"
#include "framework/renderer.h"

#include "ozz/base/containers/vector.h"
#include "ozz/base/maths/rect.h"


namespace ozz {
	namespace sample {
		class Renderer;

		namespace internal {

			// Immediate mode gui implementation.
			class ImGuiVulkan : public ImGui {
			public:
				ImGuiVulkan();
				virtual ~ImGuiVulkan();

				// Starts an imgui frame.
				// _inputs describes next frame inputs. It is internally copied.
				// _rect is the windows size.
				virtual void BeginFrame(const Inputs& _inputs, const math::RectInt& _rect,
					Renderer* _renderer) override;

				// Ends the current imgui frame.
				virtual void EndFrame() override;

			protected:
				// Starts ImGui interface implementation.
				// See imgui.h for virtual function specifications.

				virtual void BeginContainer(const char* _title, const math::RectFloat* _rect,
					bool* _open, bool _constrain) override;

				virtual void EndContainer() override;

				virtual bool DoButton(const char* _label, bool _enabled, bool* _state) override;

				virtual bool DoSlider(const char* _label, float _min, float _max,
					float* _value, float _pow, bool _enabled) override;

				virtual bool DoSlider(const char* _label, int _min, int _max, int* _value,
					float _pow, bool _enabled) override;

				virtual bool DoCheckBox(const char* _label, bool* _state, bool _enabled) override;

				virtual bool DoRadioButton(int _ref, const char* _label, int* _value,
					bool _enabled) override;

				virtual void DoLabel(const char* _label, Justification _justification,
					bool _single_line) override;

				virtual void DoGraph(const char* _label, float _min, float _max, float _mean,
					const float* _value_cursor, const float* _value_begin,
					const float* _value_end) override;
			};

		}  // internal
	}  // sample
}  // ozz
#endif  // OZZ_SAMPLES_FRAMEWORK_INTERNAL_IMGUI_VULKAN_H_
		