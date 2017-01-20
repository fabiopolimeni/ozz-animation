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
#include "ozz/base/maths/box.h"
#include "framework/application.h"
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

namespace {

	// Following code based on http://answers.unity3d.com/storage/temp/12048-lefthandedtorighthanded.pdf

	static ozz::math::Float4x4 getCorrectedModel(const ozz::math::Float4x4 model) {
		ozz::math::Float4x4 model_matrix = model;
		
		model_matrix.cols[3] = model.cols[3] * ozz::math::simd_float4::Load(-1.f, 1.f, 1.f, 1.f);
		return model_matrix;
	}

	static ozz::math::Float4x4 getCorrectedView(const ozz::math::Float4x4 view) {
		ozz::math::Float4x4 view_matrix;
		
		view_matrix.cols[0] = view.cols[0] * ozz::math::simd_float4::Load(-1.f, -1.f, 1.f, 1.f);
		view_matrix.cols[1] = view.cols[1] * ozz::math::simd_float4::Load(1.f, 1.f, -1.f, 1.f);
		view_matrix.cols[2] = view.cols[2] * ozz::math::simd_float4::Load(1.f, 1.f, -1.f, 1.f);
		view_matrix.cols[3] = view.cols[3] * ozz::math::simd_float4::Load(1.f, 1.f, -1.f, 1.f);
		return view_matrix;
	}

	static ozz::math::Float4x4 getCorrectedProjection(const ozz::math::Float4x4 projection) {
		ozz::math::Float4x4 projection_matrix = projection;
		
		ozz::math::Float4x4 clip;
		clip.cols[0] = ozz::math::simd_float4::Load(1.0f, 0.0f, 0.0f, 0.0f);
		clip.cols[1] = ozz::math::simd_float4::Load(0.0f, -1.0f, 0.0f, 0.0f);
		clip.cols[2] = ozz::math::simd_float4::Load(0.0f, 0.0f, 0.5f, 0.0f);
		clip.cols[3] = ozz::math::simd_float4::Load(0.0f, 0.0f, 0.5f, 1.0f);

		projection_matrix.cols[2] = projection.cols[2] * ozz::math::simd_float4::Load(0.f, 0.f, -1.f, -1.f);
		return clip * projection_matrix;
	}
}

ozz::sample::internal::RendererVulkan::RendererVulkan(Camera * _camera)
	: ozz::sample::Renderer(), camera_(_camera)
	, rs_shaded_boxes_(nullptr)
	, rs_immediate_box_(nullptr)
	, rs_line_batcher_(nullptr)
{
}

ozz::sample::internal::RendererVulkan::~RendererVulkan()
{
	context_->destroyRenderState(rs_shaded_boxes_);
	context_->destroyRenderState(rs_immediate_box_);
	context_->destroyRenderState(rs_line_batcher_);

	context_->shutdown();
	memory::default_allocator()->Delete(context_);
}

bool ozz::sample::internal::RendererVulkan::Initialize()
{
	log::Log() << "Render engine: Vulkan" << std::endl;
	context_ = memory::default_allocator()->New<internal::ContextVulkan>();
	return context_->initialize();
}

void ozz::sample::internal::RendererVulkan::OnResize(int32_t _width, int32_t _height)
{
	context_->recreateSwapChain();
}

void ozz::sample::internal::RendererVulkan::DrawAxes(const ozz::math::Float4x4 & _transform)
{
	if (!rs_line_batcher_) {
		rs_line_batcher_ = context_->createRenderState<vk::LineRenderState>();
		CHECK_AND_REPORT(rs_line_batcher_, "Line-batcher render-state can't be created!");
	}

	auto corrected_transform = getCorrectedModel(_transform);

	// X axis (red).
	rs_line_batcher_->add(
		ozz::math::Float3(0, 0 ,0),
		ozz::math::Float3(1, 0, 0),
		{0xff, 0x00, 0x00, 0xff},
		corrected_transform
	);

	// Y axis (green).
	rs_line_batcher_->add(
		ozz::math::Float3(0, 0, 0),
		ozz::math::Float3(0, 1, 0),
		{ 0x00, 0xff, 0x00, 0xff },
		corrected_transform
	);

	// Z axis (blue).
	rs_line_batcher_->add(
		ozz::math::Float3(0, 0, 0),
		ozz::math::Float3(0, 0, 1),
		{ 0x00, 0x00, 0xff, 0xff },
		corrected_transform
	);
}

void ozz::sample::internal::RendererVulkan::DrawGrid(int _cell_count, float _cell_size)
{
	if (!rs_line_batcher_) {
		rs_line_batcher_ = context_->createRenderState<vk::LineRenderState>();
		CHECK_AND_REPORT(rs_line_batcher_, "Line-batcher render-state can't be created!");
	}

	const float extent = _cell_count * _cell_size;
	const float half_extent = extent * 0.5f;
	const ozz::math::Float3 corner(-half_extent, 0, -half_extent);

	ozz::math::Float3 begin(corner.x, corner.y, corner.z);
	ozz::math::Float3 end = begin;
	end.x += extent;

	// Renders lines along X axis.
	for (int i = 0; i < _cell_count + 1; ++i) {		
		rs_line_batcher_->add(begin, end,
			{ 0x54, 0x55, 0x50, 0xff },
			math::Float4x4::identity()
		);

		begin.z += _cell_size;
		end.z += _cell_size;
	}

	// Renders lines along Z axis.
	begin.x = corner.x;
	begin.y = corner.y;
	begin.z = corner.z;
	end = begin;
	end.z += extent;

	for (int i = 0; i < _cell_count + 1; ++i) {
		rs_line_batcher_->add(begin, end,
			{ 0x54, 0x55, 0x50, 0xff },
			math::Float4x4::identity()
		);

		begin.x += _cell_size;
		end.x += _cell_size;
	}
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
	auto corrected_transform = getCorrectedModel(_transform);

	// Fill box
	{
		if (!rs_immediate_box_) {
			const math::Float3 pos[8] = {
				math::Float3(_box.min.x, _box.min.y, _box.min.z),
				math::Float3(_box.max.x, _box.min.y, _box.min.z),
				math::Float3(_box.max.x, _box.max.y, _box.min.z),
				math::Float3(_box.min.x, _box.max.y, _box.min.z),
				math::Float3(_box.min.x, _box.min.y, _box.max.z),
				math::Float3(_box.max.x, _box.min.y, _box.max.z),
				math::Float3(_box.max.x, _box.max.y, _box.max.z),
				math::Float3(_box.min.x, _box.max.y, _box.max.z)
			};

			const math::Float3 normals[6] = {
				math::Float3(-1,  0,  0),	// n0. left
				math::Float3(1,  0,  0),	// n1. right
				math::Float3(0, -1,  0),	// n2. bottom
				math::Float3(0,  1,  0),	// n3. top
				math::Float3(0,  0, -1),	// n4. front
				math::Float3(0,  0,  1)		// n5. back
			};

			const math::Float2 uvs[4] = {
				math::Float2(0.0f, 0.0f),	// t0. top-left
				math::Float2(1.0f, 0.0f),	// t1. top-right
				math::Float2(1.0f, 1.0f),	// t2. bottom-right
				math::Float2(0.0f, 1.0f)	// t3. bottom-left
			};

			std::vector<vk::ModelRenderState::Vertex> vertices = {
				// n4. front
				{ pos[0], normals[4], uvs[3], _colors[0] },
				{ pos[1], normals[4], uvs[0], _colors[0] },
				{ pos[2], normals[4], uvs[2], _colors[0] },
				{ pos[3], normals[4], uvs[0], _colors[0] },
				// n1. right
				{ pos[1], normals[1], uvs[3], _colors[0] },
				{ pos[5], normals[1], uvs[0], _colors[0] },
				{ pos[6], normals[1], uvs[2], _colors[0] },
				{ pos[2], normals[1], uvs[0], _colors[0] },
				// n5. top
				{ pos[3], normals[3], uvs[3], _colors[0] },
				{ pos[2], normals[3], uvs[0], _colors[0] },
				{ pos[6], normals[3], uvs[2], _colors[0] },
				{ pos[7], normals[3], uvs[0], _colors[0] },
				// n0. left
				{ pos[4], normals[0], uvs[3], _colors[0] },
				{ pos[0], normals[0], uvs[0], _colors[0] },
				{ pos[3], normals[0], uvs[2], _colors[0] },
				{ pos[7], normals[0], uvs[0], _colors[0] },
				// n2. bottom
				{ pos[4], normals[2], uvs[3], _colors[0] },
				{ pos[5], normals[2], uvs[0], _colors[0] },
				{ pos[1], normals[2], uvs[2], _colors[0] },
				{ pos[0], normals[2], uvs[0], _colors[0] },
				// n5. back
				{ pos[5], normals[5], uvs[3], _colors[0] },
				{ pos[4], normals[5], uvs[0], _colors[0] },
				{ pos[7], normals[5], uvs[2], _colors[0] },
				{ pos[6], normals[5], uvs[0], _colors[0] }
			};

			static std::vector<uint32_t> indices = {
				0,  1,  2,  2,  3,  0,
				4,  5,  6,  6,  7,  4,
				8,  9, 10, 10, 11,  8,
				12, 13, 14, 14, 15, 12,
				16, 17, 18, 18, 19, 16,
				20, 21, 22, 22, 23, 20
			};

			// Create a new model-render-state template
			vk::ModelRenderState::InitData init_data;

			// Set vertices and indices
			init_data.gbo.vertices = vertices;
			init_data.gbo.indices = indices;

			// Static magenta texture
			static const uint32_t magenta_color = 0xFFFF00FF;
			init_data.tso.width = 1;
			init_data.tso.height = 1;
			init_data.tso.pixels = (const uint8_t*)(&magenta_color);

			rs_immediate_box_ = context_->createRenderState<vk::ModelRenderState>();
			CHECK_AND_REPORT(rs_immediate_box_, "Immediate-box render-state can't be created!");

			// Create device resources, buffers and texture images
			CHECK_AND_REPORT(rs_immediate_box_->init(init_data), "Cannot initialise the device-resources for the shadeed-boxs render-state!");
		}

		vk::ModelRenderState::UpdateData update_data;
		update_data.flags = vk::ModelRenderState::UpdateData::UDF_NONE;

		// Update instance buffer
		{
			const auto numOfInstances = 1;
			update_data.ibo.transforms.resize(numOfInstances);
			update_data.ibo.transforms[0] = corrected_transform;

			// Trigger instance buffer update
			update_data.flags |= vk::ModelRenderState::UpdateData::UDF_INSTANCE_BUFFER;
		}

		// Update uniform buffer
		{
			// Uniform data
			update_data.ubo.model = math::Float4x4::identity();
			update_data.ubo.view = getCorrectedView(camera_->view());
			update_data.ubo.proj = getCorrectedProjection(camera_->projection());

			// Trigger uniform buffer update
			update_data.flags |= vk::ModelRenderState::UpdateData::UDF_UNIFORM_BUFFER;
		}

		rs_immediate_box_->update(update_data);
	}

	// Wireframe box
	{
		if (!rs_line_batcher_) {
			rs_line_batcher_ = context_->createRenderState<vk::LineRenderState>();
			CHECK_AND_REPORT(rs_line_batcher_, "Line-batcher render-state can't be created!");
		}

		ozz::math::Float3 v0 = { _box.min.x, _box.min.y, _box.min.z };
		auto v1 = v0;

		// First face.
		v1.y = _box.max.y;
		rs_line_batcher_->add(v0, v1, _colors[1], corrected_transform);
		
		v0 = v1;
		v1.x = _box.max.x;
		rs_line_batcher_->add(v0, v1, _colors[1], corrected_transform);

		v0 = v1;
		v1.y = _box.min.y;
		rs_line_batcher_->add(v0, v1, _colors[1], corrected_transform);

		v0 = v1;
		v1.x = _box.min.x;
		rs_line_batcher_->add(v0, v1, _colors[1], corrected_transform);

		// Second face.
		v0 = v1;
		v0.z = _box.max.z;
		v1 = v0;
		v1.y = _box.max.y;
		rs_line_batcher_->add(v0, v1, _colors[1], corrected_transform);

		v0 = v1;
		v1.x = _box.max.x;
		rs_line_batcher_->add(v0, v1, _colors[1], corrected_transform);

		v0 = v1;
		v1.y = _box.min.y;
		rs_line_batcher_->add(v0, v1, _colors[1], corrected_transform);

		v0 = v1;
		v1.x = _box.min.x;
		rs_line_batcher_->add(v0, v1, _colors[1], corrected_transform);

		// Link faces.
		v0 = v1;
		v1.z = _box.min.z;
		rs_line_batcher_->add(v0, v1, _colors[1], corrected_transform);

		v0 = v1;
		v0.y = _box.max.y;
		v1 = v0;
		v1.z = _box.max.z;
		rs_line_batcher_->add(v0, v1, _colors[1], corrected_transform);
		
		v0 = v1;
		v0.x = _box.max.x;
		v1 = v0;
		v1.z = _box.min.z;
		rs_line_batcher_->add(v0, v1, _colors[1], corrected_transform);

		v0 = v1;
		v0.y = _box.min.y;
		v1 = v0;
		v1.z = _box.max.z;
		rs_line_batcher_->add(v0, v1, _colors[1], corrected_transform);
	}

	return true;
}

bool ozz::sample::internal::RendererVulkan::DrawBoxShaded(const ozz::math::Box & _box, ozz::Range<const ozz::math::Float4x4> _transforms, Color _color)
{
	// If a shaded box render state doesn't exist yet, create one
	if (!rs_shaded_boxes_) {
		const math::Float3 pos[8] = {
			math::Float3(_box.min.x, _box.min.y, _box.min.z),
			math::Float3(_box.max.x, _box.min.y, _box.min.z),
			math::Float3(_box.max.x, _box.max.y, _box.min.z),
			math::Float3(_box.min.x, _box.max.y, _box.min.z),
			math::Float3(_box.min.x, _box.min.y, _box.max.z),
			math::Float3(_box.max.x, _box.min.y, _box.max.z),
			math::Float3(_box.max.x, _box.max.y, _box.max.z),
			math::Float3(_box.min.x, _box.max.y, _box.max.z)
		};

		const math::Float3 normals[6] = {
			math::Float3(-1,  0,  0),	// n0. left
			math::Float3(1,  0,  0),	// n1. right
			math::Float3(0, -1,  0),	// n2. bottom
			math::Float3(0,  1,  0),	// n3. top
			math::Float3(0,  0, -1),	// n4. front
			math::Float3(0,  0,  1)		// n5. back
		};

		const math::Float2 uvs[4] = {
			math::Float2(0.0f, 0.0f),	// t0. top-left
			math::Float2(1.0f, 0.0f),	// t1. top-right
			math::Float2(1.0f, 1.0f),	// t2. bottom-right
			math::Float2(0.0f, 1.0f)	// t3. bottom-left
		};

		std::vector<vk::ModelRenderState::Vertex> vertices = {
			// n4. front
			{ pos[0], normals[4], uvs[3], _color },
			{ pos[1], normals[4], uvs[0], _color },
			{ pos[2], normals[4], uvs[2], _color },
			{ pos[3], normals[4], uvs[0], _color },
			// n1. right
			{ pos[1], normals[1], uvs[3], _color },
			{ pos[5], normals[1], uvs[0], _color },
			{ pos[6], normals[1], uvs[2], _color },
			{ pos[2], normals[1], uvs[0], _color },
			// n5. top
			{ pos[3], normals[3], uvs[3], _color },
			{ pos[2], normals[3], uvs[0], _color },
			{ pos[6], normals[3], uvs[2], _color },
			{ pos[7], normals[3], uvs[0], _color },
			// n0. left
			{ pos[4], normals[0], uvs[3], _color },
			{ pos[0], normals[0], uvs[0], _color },
			{ pos[3], normals[0], uvs[2], _color },
			{ pos[7], normals[0], uvs[0], _color },
			// n2. bottom
			{ pos[4], normals[2], uvs[3], _color },
			{ pos[5], normals[2], uvs[0], _color },
			{ pos[1], normals[2], uvs[2], _color },
			{ pos[0], normals[2], uvs[0], _color },
			// n5. back
			{ pos[5], normals[5], uvs[3], _color },
			{ pos[4], normals[5], uvs[0], _color },
			{ pos[7], normals[5], uvs[2], _color },
			{ pos[6], normals[5], uvs[0], _color }
		};

		static std::vector<uint32_t> indices = {
			0,  1,  2,  2,  3,  0,
			4,  5,  6,  6,  7,  4,
			8,  9, 10, 10, 11,  8,
		   12, 13, 14, 14, 15, 12,
		   16, 17, 18, 18, 19, 16,
		   20, 21, 22, 22, 23, 20
		};

		// Create a new model-render-state template
		vk::ModelRenderState::InitData init_data;

		// Set vertices and indices
		init_data.gbo.vertices = vertices;
		init_data.gbo.indices = indices;

		// Static magenta texture
		static const uint32_t magenta_color = 0xFFFF00FF;
		init_data.tso.width = 1;
		init_data.tso.height = 1;
		init_data.tso.pixels = (const uint8_t*)(&magenta_color);

		rs_shaded_boxes_ = context_->createRenderState<vk::ModelRenderState>();
		CHECK_AND_REPORT(rs_shaded_boxes_, "Shaded-boxes render-state can't be created!");

		// Create device resources, buffers and texture images
		CHECK_AND_REPORT(rs_shaded_boxes_->init(init_data), "Cannot initialise the device-resources for the shadeed-boxs render-state!");
	}

	vk::ModelRenderState::UpdateData update_data;
	update_data.flags = vk::ModelRenderState::UpdateData::UDF_NONE;

	// Update instance buffer
	{
		const auto numOfInstances = _transforms.Count();
		update_data.ibo.transforms.resize(numOfInstances);
		for (auto iIt = 0; iIt < numOfInstances; ++iIt) {
			update_data.ibo.transforms[iIt] = getCorrectedModel(_transforms[iIt]);
		}

		// Trigger instance buffer update
		update_data.flags |= vk::ModelRenderState::UpdateData::UDF_INSTANCE_BUFFER;
	}

	// Update uniform buffer
	{
		// Uniform data
		update_data.ubo.model = math::Float4x4::identity();
		update_data.ubo.view = getCorrectedView(camera_->view());
		update_data.ubo.proj = getCorrectedProjection(camera_->projection());

		// Trigger uniform buffer update
		update_data.flags |= vk::ModelRenderState::UpdateData::UDF_UNIFORM_BUFFER;
	}

	rs_shaded_boxes_->update(update_data);
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
	// Update line batcher uniform data
	{
		vk::LineRenderState::UpdateData line_batcher_update_data;
		line_batcher_update_data.ubo.proj = getCorrectedProjection(camera_->projection());
		line_batcher_update_data.ubo.view = getCorrectedView(camera_->view());
		line_batcher_update_data.flags = vk::LineRenderState::UpdateData::UDF_UNIFORM_BUFFER;
		rs_line_batcher_->submit(line_batcher_update_data);
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
