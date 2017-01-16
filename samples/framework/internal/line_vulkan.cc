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
#include "framework/internal/line_vulkan.h"

std::array<VkVertexInputBindingDescription, 1> ozz::sample::vk::LineRenderState::getVertexBindingDescriptions()
{
	VkVertexInputBindingDescription vertexBindingDesc = { 0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX };

	return{ vertexBindingDesc };
}

std::array<VkVertexInputAttributeDescription, 2> ozz::sample::vk::LineRenderState::getVertexAttributeDescriptions()
{
	std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

	// Geometry
	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, pos);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R8G8B8A8_UNORM;
	attributeDescriptions[1].offset = offsetof(Vertex, color);

	return attributeDescriptions;

}

void ozz::sample::vk::LineRenderState::setupDeleterPtrs()
{

}

void ozz::sample::vk::LineRenderState::createDescriptorSetLayout()
{

}

void ozz::sample::vk::LineRenderState::createGraphicsPipeline()
{

}

void ozz::sample::vk::LineRenderState::createUniformBuffer()
{

}

void ozz::sample::vk::LineRenderState::createDescriptorPool()
{

}

void ozz::sample::vk::LineRenderState::createVertexBuffer(const std::vector<Vertex>& /*vertices*/)
{

}

void ozz::sample::vk::LineRenderState::createDescriptorSet()
{

}

void ozz::sample::vk::LineRenderState::updateVertexBuffer(const std::vector<Vertex>& /*vertices*/)
{

}

bool ozz::sample::vk::LineRenderState::updateGeometryBufferObject(const GeometryBuffersObject& /*gbo*/)
{
	return true;
}

bool ozz::sample::vk::LineRenderState::updateUniformBufferObject(const UniformBufferObject& /*ubo*/)
{
	return true;
}

void ozz::sample::vk::LineRenderState::setDirty(bool /*bDirty*/)
{

}

ozz::sample::vk::LineRenderState::LineRenderState()
{

}

ozz::sample::vk::LineRenderState::~LineRenderState()
{

}

bool ozz::sample::vk::LineRenderState::onInitResources(internal::ContextVulkan* /*context*/)
{
	return true;
}

void ozz::sample::vk::LineRenderState::onReleaseResources()
{

}

bool ozz::sample::vk::LineRenderState::onRegisterRenderPass(size_t /*commandIndex*/)
{
	return true;
}

void ozz::sample::vk::LineRenderState::onRenderPassesComplete()
{

}

bool ozz::sample::vk::LineRenderState::onSwapChainChange()
{
	return true;
}

bool ozz::sample::vk::LineRenderState::isDirty()
{
	return true;
}

bool ozz::sample::vk::LineRenderState::add(ozz::math::Float3 /*p0*/, ozz::math::Float3 /*p1*/,
	Renderer::Color /*color*/, ozz::math::Float4x4 /*transform*/)
{
	return true;
}

bool ozz::sample::vk::LineRenderState::submit(const UpdateData& /*updateData*/)
{
	return true;
}
