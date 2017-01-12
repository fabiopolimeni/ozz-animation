//----------------------------------------------------------------------------//
//                                                                            //
// ozz-animation is hosted at http://github.com/guillaumeblanc/ozz-animation  //
// and distributed under the MIT License (MIT).                               //
//                                                                            //
// Copyright (c) 2016 Fabio Polimeni                                          //
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
#include "framework/renderer.h"
#include "framework/camera.h"
#include "framework/internal/renderer_vulkan.h"
#include "framework/internal/context_vulkan.h"
#include "framework/internal/model_vulkan.h"
#include "framework/internal/mesh_vulkan.h"
#include "framework/internal/line_vulkan.h"
#include "framework/internal/skeleton_vulkan.h"

#include "glfw/glfw3.h"

// TODO: Remove this warning disable instruction once all functions have been implemented
#pragma warning(disable: 4100)

ozz::sample::internal::RendererVulkan::RendererVulkan(Camera * _camera)
	: ozz::sample::Renderer(), camera_(_camera)
{
}

ozz::sample::internal::RendererVulkan::~RendererVulkan()
{
	context_->shutdown();
	memory::default_allocator()->Delete(context_);
}

bool ozz::sample::internal::RendererVulkan::Initialize()
{
	log::Log() << "Render engine: Vulkan" << std::endl;

	// Initialise to default values the render state infos
	for (auto rsiIt = 0; rsiIt < RenderStateType::RST_MAX; ++rsiIt) {
		auto& rsi = stateInfos_[rsiIt];
		rsi.stateType = (RenderStateType)rsiIt;
		rsi.numOfInstances = 0;
	}

	context_ = memory::default_allocator()->New<internal::ContextVulkan>();
	return context_->initialize();
}

void ozz::sample::internal::RendererVulkan::OnResize(int32_t _width, int32_t _height)
{
	context_->recreateSwapChain();
}

void ozz::sample::internal::RendererVulkan::DrawAxes(const ozz::math::Float4x4 & _transform)
{
}

void ozz::sample::internal::RendererVulkan::DrawGrid(int _cell_count, float _cell_size)
{
}

bool ozz::sample::internal::RendererVulkan::DrawSkeleton(const animation::Skeleton & _skeleton, const ozz::math::Float4x4 & _transform, bool _draw_joints)
{
	return true;
}

bool ozz::sample::internal::RendererVulkan::DrawPosture(const animation::Skeleton & _skeleton, ozz::Range<const ozz::math::Float4x4> _matrices, const ozz::math::Float4x4 & _transform, bool _draw_joints)
{
	return true;
}

bool ozz::sample::internal::RendererVulkan::DrawBoxIm(const ozz::math::Box & _box, const ozz::math::Float4x4 & _transform, const Color _colors[2])
{
	return true;
}

bool ozz::sample::internal::RendererVulkan::DrawBoxShaded(const ozz::math::Box & _box, ozz::Range<const ozz::math::Float4x4> _transforms, Color _color)
{
	for (auto mIt = 0; mIt < _transforms.Count(); ++mIt) {
		RenderStateInfo& state_info = stateInfos_[RenderStateType::RST_MODEL];
		if (state_info.numOfInstances >= state_info.stateInstances.size()) {
			auto* stateInstance = context_->createRenderState<vk::ModelRenderState>();
			state_info.stateInstances.push_back(stateInstance);
		}

		auto* model_render_state = static_cast<vk::ModelRenderState*>(state_info.stateInstances[state_info.numOfInstances++]);

		vk::ModelRenderState::UpdateData update_data;

		// front-face
		{
			std::vector<vk::ModelRenderState::Vertex> front_face = {
				//				position							normal								uvs										color
				{ ozz::math::Float3(-0.5f, -0.5f, 0.5f), ozz::math::Float3(0.0f, 0.0f, 1.0f), ozz::math::Float2(0.0f, 0.0f), { _color.r, _color.g, _color.b, _color.a} },
				{ ozz::math::Float3(0.5f, -0.5f, 0.5f),  ozz::math::Float3(0.0f, 0.0f, 1.0f), ozz::math::Float2(1.0f, 0.0f), { _color.r, _color.g, _color.b, _color.a } },
				{ ozz::math::Float3(0.5f,  0.5f, 0.5f),  ozz::math::Float3(0.0f, 0.0f, 1.0f), ozz::math::Float2(1.0f, 1.0f), { _color.r, _color.g, _color.b, _color.a } },
				{ ozz::math::Float3(-0.5f,  0.5f, 0.5f), ozz::math::Float3(0.0f, 0.0f, 1.0f), ozz::math::Float2(0.0f, 1.0f), { _color.r, _color.g, _color.b, _color.a } }
			};

			uint32_t start_index = static_cast<uint32_t>(update_data.vertices.size());
			update_data.indices.insert(std::begin(update_data.indices), {
				start_index + 0, start_index + 1, start_index + 2, start_index + 2, start_index + 3, start_index + 0 });
			update_data.vertices.insert(std::begin(update_data.vertices), std::begin(front_face), std::begin(front_face));
		}

		// TODO: ...
		// back-face
		// top-face
		// bottom-face
		// right-face
		// left-face

		// Uniform data
		update_data.ubo.model = _transforms[mIt];
		update_data.ubo.view = camera_->view();
		update_data.ubo.proj = camera_->projection();

		model_render_state->update(update_data);
	}
	return true;
}

bool ozz::sample::internal::RendererVulkan::DrawSkinnedMesh(const Mesh & _mesh, const Range<math::Float4x4> _skinning_matrices, const ozz::math::Float4x4 & _transform, const Options & _options)
{
	return true;
}

bool ozz::sample::internal::RendererVulkan::DrawMesh(const Mesh & _mesh, const ozz::math::Float4x4 & _transform, const Options & _options)
{
	return true;
}

bool ozz::sample::internal::RendererVulkan::DrawVectors(ozz::Range<const float> _positions, size_t _positions_stride, ozz::Range<const float> _directions, size_t _directions_stride, int _num_vectors, float _vector_length, Renderer::Color _color, const ozz::math::Float4x4 & _transform)
{
	return true;
}

bool ozz::sample::internal::RendererVulkan::DrawBinormals(ozz::Range<const float> _positions, size_t _positions_stride, ozz::Range<const float> _normals, size_t _normals_stride, ozz::Range<const float> _tangents, size_t _tangents_stride, ozz::Range<const float> _handenesses, size_t _handenesses_stride, int _num_vectors, float _vector_length, Renderer::Color _color, const ozz::math::Float4x4 & _transform)
{
	return true;
}

bool ozz::sample::internal::RendererVulkan::RenderFrame()
{
	// Reset numOfInstance of each render state.
	for (auto& rsi : stateInfos_) {
		rsi.numOfInstances = 0;
	}

	return context_->drawFrame();
}

bool ozz::sample::internal::RendererVulkan::InitPostureRendering()
{
	return true;
}

bool ozz::sample::internal::RendererVulkan::InitCheckeredTexture()
{
	return true;
}

void ozz::sample::internal::RendererVulkan::DrawPosture_Impl(const ozz::math::Float4x4 & _transform, const float * _uniforms, int _instance_count, bool _draw_joints)
{
}

void ozz::sample::internal::RendererVulkan::DrawPosture_InstancedImpl(const ozz::math::Float4x4 & _transform, const float * _uniforms, int _instance_count, bool _draw_joints)
{
}
