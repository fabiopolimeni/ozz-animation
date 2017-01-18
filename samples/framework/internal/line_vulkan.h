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

#ifndef OZZ_SAMPLES_FRAMEWORK_LINERENDERSTATE_VULKAN_H_
#define OZZ_SAMPLES_FRAMEWORK_LINERENDERSTATE_VULKAN_H_

#ifndef OZZ_INCLUDE_PRIVATE_HEADER
#error "This header is private, it cannot be included from public headers."
#endif  // OZZ_INCLUDE_PRIVATE_HEADER

#include <vector>

#include "ozz/base/maths/vec_float.h"
#include "ozz/base/maths/simd_math.h"

#include "framework/renderer.h"
#include "framework/internal/renderstate_vulkan.h"
#include "framework/internal/tools_vulkan.h"
#include "framework/internal/context_vulkan.h"

namespace ozz {
	namespace sample {
		namespace vk {

			class LineRenderState : public RenderState {

			public:

				struct Vertex {
					ozz::math::Float3	pos;
					Renderer::Color		color;
				};

				struct UniformBufferObject {
					ozz::math::Float4x4 view;
					ozz::math::Float4x4 proj;
				};

				struct UpdateData {
					enum UpdateFalgs {
						UDF_NONE = 0x00,
						UDF_UNIFORM_BUFFER = 0x08,
						UDF_ALL = UDF_UNIFORM_BUFFER
					};

					UniformBufferObject		ubo;		// Uniform buffer (MVP) object
					uint32_t				flags;		// Used to filter state changes;
				};

			private:

				struct GeometryBuffersObject {
					std::vector<Vertex>		vertices;	// Vertex buffer
				};

				GeometryBuffersObject gbo;

				// Store the minimum information needed to determine
				// whether a buffer needs to be re-instantiated or not.
				uint32_t numOfVertices;
				bool dirty;

				vk::deleter_ptr<VkDescriptorSetLayout> descriptorSetLayout;
				vk::deleter_ptr<VkPipelineLayout> pipelineLayout;
				vk::deleter_ptr<VkPipeline> graphicsPipeline;

				vk::deleter_ptr<VkBuffer> vertexBuffer;
				vk::deleter_ptr<VkDeviceMemory> vertexBufferMemory;
				
				vk::deleter_ptr<VkBuffer> uniformStagingBuffer;
				vk::deleter_ptr<VkDeviceMemory> uniformStagingBufferMemory;
				vk::deleter_ptr<VkBuffer> uniformBuffer;
				vk::deleter_ptr<VkDeviceMemory> uniformBufferMemory;

				vk::deleter_ptr<VkDescriptorPool> descriptorPool;
				VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

				std::array<VkVertexInputBindingDescription, 1> getVertexBindingDescriptions();
				std::array<VkVertexInputAttributeDescription, 2> getVertexAttributeDescriptions();

				void setupDeleterPtrs();

				void createDescriptorSetLayout();
				void createGraphicsPipeline();
				void createUniformBuffer();
				void createDescriptorPool();

				void createVertexBuffer(const std::vector<Vertex>& vertices);
				void createDescriptorSet();

				void updateVertexBuffer(const std::vector<Vertex>& vertices);
				
				void updateGeometryBufferObject();
				
				// These series of functions return whether or not the respective
				// buffer needed to be re-created as a result of the update.
				bool updateUniformBufferObject(const UniformBufferObject& ubo);

				void setDirty(bool bDirty);

			public:

				// Ctor
				LineRenderState();
				virtual ~LineRenderState();

				// Gives a chance to create the render resources
				virtual bool onInitResources(internal::ContextVulkan* context) override;

				// Gives a chance to release all the owned resources
				virtual void onReleaseResources() override;

				// This function is called when command buffers are recorded,
				// between vkCmdBeginRenderPass() and vkCmdEndRenderPass().
				virtual bool onRegisterRenderPass(size_t commandIndex) override;

				// Because we can have multiple render passes,
				// this callback specify when the render context
				// has finished to register them all.
				virtual void onRenderPassesComplete() override;

				// Notified when the swap chain has changed, e.g. resized.
				virtual bool onSwapChainChange() override;

				// Made dirty whenever we have to recreate any of the buffers,
				// as these have to be re-bound to the render pass.
				virtual bool isDirty() override;

				// Add a segment
				bool add(ozz::math::Float3 p0, ozz::math::Float3 p1,
					Renderer::Color color, ozz::math::Float4x4 transform);

				// Once all the segments have been accumulated,
				// submit them all in once just before rendering.
				bool submit(const UpdateData& updateData);

			};

		}
	}
}

#endif //OZZ_SAMPLES_FRAMEWORK_LINERENDERSTATE_VULKAN_H_