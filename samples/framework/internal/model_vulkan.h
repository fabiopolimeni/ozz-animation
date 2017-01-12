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

#ifndef OZZ_SAMPLES_FRAMEWORK_MODELRENDERSTATE_VULKAN_H_
#define OZZ_SAMPLES_FRAMEWORK_MODELRENDERSTATE_VULKAN_H_

#ifndef OZZ_INCLUDE_PRIVATE_HEADER
#error "This header is private, it cannot be included from public headers."
#endif  // OZZ_INCLUDE_PRIVATE_HEADER

#include <vector>

#include "ozz/base/maths/vec_float.h"
#include "ozz/base/maths/simd_math.h"

#include "framework/internal/renderstate_vulkan.h"
#include "framework/internal/tools_vulkan.h"
#include "framework/internal/context_vulkan.h"

namespace ozz {
	namespace sample {
		namespace vk {

			class ModelRenderState : public RenderState {

			public:
				
				struct Vertex {
					ozz::math::Float3	pos;
					ozz::math::Float3	normal;
					ozz::math::Float2	uv;
					struct {
						uint8_t r, g, b, a;
					} color;
				};

				struct UniformBufferObject {
					ozz::math::Float4x4 model;
					ozz::math::Float4x4 view;
					ozz::math::Float4x4 proj;
				};

				struct TextureSamplerObject {
					const uint8_t*	pixels;
					uint32_t		width;
					uint32_t		height;
				};

				struct UpdateData {
					enum UpdateFalgs {
						UDF_VERTEX_BUFFER = 0x01,
						UDF_INDEX_BUFFER = 0x02,
						UDF_UNIFORM_OBJECT = 0x04,
						UDF_TEXTURE_SAMPLER = 0x08,
						UDF_ALL = UDF_VERTEX_BUFFER | UDF_INDEX_BUFFER | UDF_UNIFORM_OBJECT | UDF_TEXTURE_SAMPLER
					};

					std::vector<Vertex>		vertices;	// Vertex buffer
					std::vector<uint32_t>	indices;	// Index buffer
					UniformBufferObject		ubo;		// MVP
					TextureSamplerObject	tex;		// Texture is meant to be RGBA
					uint32_t				flags;		// Used to optimize the state changes;
				};

			private:

				std::vector<Vertex> vertices;
				std::vector<uint32_t> indices;

				vk::deleter_ptr<VkDescriptorSetLayout> descriptorSetLayout{ renderContext->device, vkDestroyDescriptorSetLayout };
				vk::deleter_ptr<VkPipelineLayout> pipelineLayout{ renderContext->device, vkDestroyPipelineLayout };
				vk::deleter_ptr<VkPipeline> graphicsPipeline{ renderContext->device, vkDestroyPipeline };

				vk::deleter_ptr<VkImage> textureImage{ renderContext->device, vkDestroyImage };
				vk::deleter_ptr<VkDeviceMemory> textureImageMemory{ renderContext->device, vkFreeMemory };
				vk::deleter_ptr<VkImageView> textureImageView{ renderContext->device, vkDestroyImageView };
				vk::deleter_ptr<VkSampler> textureSampler{ renderContext->device, vkDestroySampler };

				vk::deleter_ptr<VkBuffer> vertexBuffer{ renderContext->device, vkDestroyBuffer };
				vk::deleter_ptr<VkDeviceMemory> vertexBufferMemory{ renderContext->device, vkFreeMemory };
				vk::deleter_ptr<VkBuffer> indexBuffer{ renderContext->device, vkDestroyBuffer };
				vk::deleter_ptr<VkDeviceMemory> indexBufferMemory{ renderContext->device, vkFreeMemory };

				vk::deleter_ptr<VkBuffer> uniformStagingBuffer{ renderContext->device, vkDestroyBuffer };
				vk::deleter_ptr<VkDeviceMemory> uniformStagingBufferMemory{ renderContext->device, vkFreeMemory };
				vk::deleter_ptr<VkBuffer> uniformBuffer{ renderContext->device, vkDestroyBuffer };
				vk::deleter_ptr<VkDeviceMemory> uniformBufferMemory{ renderContext->device, vkFreeMemory };

				vk::deleter_ptr<VkDescriptorPool> descriptorPool{ renderContext->device, vkDestroyDescriptorPool };
				VkDescriptorSet descriptorSet;

				VkVertexInputBindingDescription getVertexBindingDescription();
				std::array<VkVertexInputAttributeDescription, 4> getVertexAttributeDescriptions();

				void createDescriptorSetLayout();
				void createGraphicsPipeline();
				void createTextureImage(const uint8_t* pixels, uint32_t width, uint32_t height);
				void createTextureImageView();
				void createTextureSampler();
				void createVertexBuffer(const std::vector<Vertex>& vertex_buffer);
				void createIndexBuffer(const std::vector<uint32_t>& index_buffer);
				void createUniformBuffer();
				void updateUniformBuffer(const UniformBufferObject& ubo);
				void createDescriptorPool();
				void createDescriptorSet();

			public:

				// Gives a chance to create the render resources
				virtual bool onInitResources(internal::ContextVulkan* context) override;

				// Gives a chance to release all the owned resources
				virtual void onReleaseResources() override;

				// This function is called when command buffers are recorded,
				// between vkCmdBeginRenderPass() and vkCmdEndRenderPass.
				virtual bool onRegisterRenderPass() override;

				// Update render state according to the flags
				bool update(const UpdateData& updateData);

			};

		}
	}
}

#endif //OZZ_SAMPLES_FRAMEWORK_MODELRENDERSTATE_VULKAN_H_