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

#define OZZ_INCLUDE_PRIVATE_HEADER  // Allows to include private headers.

#include "ozz/base/log.h"

#include "framework/internal/tools_vulkan.h"
#include "framework/internal/context_vulkan.h"

#include "framework/internal/imgui_vulkan.h"
#include "framework/internal/imgui/imgui_impl_glfw_vulkan.h"

// TODO: Re-enable this warning, once all the function have been properly reimplemented
#pragma warning (disable:4100)

ozz::sample::internal::ImGuiVulkan::ImGuiVulkan()
{

}

ozz::sample::internal::ImGuiVulkan::~ImGuiVulkan()
{

}

void ozz::sample::internal::ImGuiVulkan::BeginFrame(const Inputs & _inputs, const math::RectInt & _rect, Renderer * _renderer)
{
}

void ozz::sample::internal::ImGuiVulkan::EndFrame()
{
}

void ozz::sample::internal::ImGuiVulkan::BeginContainer(const char * _title, const math::RectFloat * _rect, bool * _open, bool _constrain)
{
}

void ozz::sample::internal::ImGuiVulkan::EndContainer()
{
}

bool ozz::sample::internal::ImGuiVulkan::DoButton(const char * _label, bool _enabled, bool * _state)
{
	return false;
}

bool ozz::sample::internal::ImGuiVulkan::DoSlider(const char * _label, float _min, float _max, float * _value, float _pow, bool _enabled)
{
	return false;
}

bool ozz::sample::internal::ImGuiVulkan::DoSlider(const char * _label, int _min, int _max, int * _value, float _pow, bool _enabled)
{
	return false;
}

bool ozz::sample::internal::ImGuiVulkan::DoCheckBox(const char * _label, bool * _state, bool _enabled)
{
	return false;
}

bool ozz::sample::internal::ImGuiVulkan::DoRadioButton(int _ref, const char * _label, int * _value, bool _enabled)
{
	return false;
}

void ozz::sample::internal::ImGuiVulkan::DoLabel(const char * _label, Justification _justification, bool _single_line)
{
}

void ozz::sample::internal::ImGuiVulkan::DoGraph(const char * _label, float _min, float _max, float _mean,
	const float * _value_cursor, const float * _value_begin, const float * _value_end)
{
}
