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
					ozz::math::Float3 pos;
					ozz::math::Float3 color;
					ozz::math::Float3 normal;
					ozz::math::Float2 uv;
				};

				struct UniformBufferObject {
					ozz::math::Float4x4 model;
					ozz::math::Float4x4 view;
					ozz::math::Float4x4 proj;
				};

				struct UpdateData
				{
					std::vector<Vertex> vertices;
					std::vector<uint32_t> indices;
					UniformBufferObject ubo;
				};

				struct InitData
				{
					std::string textureFilename;
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

				void createDescriptorSetLayout();
				void createGraphicsPipeline();
				void createTextureImage();
				void createTextureImageView();
				void createTextureSampler();
				void createVertexBuffer();
				void createIndexBuffer();
				void createUniformBuffer();
				void createDescriptorPool();
				void createDescriptorSet();

			public:

				ModelRenderState(const InitData& initData);

				// Gives a chance to create the render resources
				virtual bool onInitResources(internal::ContextVulkan* context) override;

				// Gives a chance to release all the owned resources
				virtual void onReleaseResources() override;

				// This function is called when command buffers are recorded,
				// between vkCmdBeginRenderPass() and vkCmdEndRenderPass.
				virtual bool onRegisterRenderPass() override;

				bool update(const UpdateData& updateData);

			};

		}
	}
}

#endif //OZZ_SAMPLES_FRAMEWORK_MODELRENDERSTATE_VULKAN_H_