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

#include "framework/renderer.h"
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
					Renderer::Color		color;
				};

				struct GeometryBuffersObject {
					std::vector<Vertex>		vertices;	// Vertex buffer
					std::vector<uint32_t>	indices;	// Index buffer
				};

				struct InstancesBufferObject {
					std::vector<ozz::math::Float4x4>	transforms;	// instance transformation matrices
				};

				struct TextureSamplerObject {
					const uint8_t*	pixels;
					uint32_t		width;
					uint32_t		height;
				};

				struct UniformBufferObject {
					ozz::math::Float4x4 model;
					ozz::math::Float4x4 view;
					ozz::math::Float4x4 proj;
				};

				struct InitData {
					GeometryBuffersObject	gbo;		// Geometry buffers (vertices and indices) object
					InstancesBufferObject	ibo;		// Instances buffer (transformation matrix) object
					TextureSamplerObject	tso;		// Texture sampler object
				};

				struct UpdateData {
					GeometryBuffersObject	gbo;		// Geometry buffers (vertices and indices) object
					InstancesBufferObject	ibo;		// Instances buffer (transformation matrix) object
					TextureSamplerObject	tso;		// Texture sampler object
					UniformBufferObject		ubo;		// Uniform buffer (MVP matrix) object
				};

				enum UpdateFalgs {
					UDF_NONE = 0x00,
					UDF_VERTEX_BUFFER = 0x01,
					UDF_INDEX_BUFFER = 0x02,
					UDF_INSTANCE_BUFFER = 0x04,
					UDF_UNIFORM_BUFFER = 0x08,
					UDF_TEXTURE_IMAGE_BUFFER = 0x10,
					UDF_GEOMETRY_BUFFER = UDF_VERTEX_BUFFER
					| UDF_INDEX_BUFFER,
					UDF_ALL = UDF_GEOMETRY_BUFFER
					| UDF_INSTANCE_BUFFER
					| UDF_UNIFORM_BUFFER
					| UDF_TEXTURE_IMAGE_BUFFER
				};

			private:

				// Store the minimum information needed to determine
				// whether a buffer needs to be re-instantiated or not.
				uint32_t numOfVertices;
				uint32_t numOfIndices;
				uint32_t numOfInstances;
				uint32_t texWidth;
				uint32_t texHeight;
				bool dirty;

				vk::deleter_ptr<VkDescriptorSetLayout> descriptorSetLayout;
				vk::deleter_ptr<VkPipelineLayout> pipelineLayout;
				vk::deleter_ptr<VkPipeline> graphicsPipeline;

				vk::deleter_ptr<VkImage> textureImage;
				vk::deleter_ptr<VkDeviceMemory> textureImageMemory;
				vk::deleter_ptr<VkImageView> textureImageView;
				vk::deleter_ptr<VkSampler> textureSampler;

				vk::deleter_ptr<VkBuffer> vertexBuffer;
				vk::deleter_ptr<VkDeviceMemory> vertexBufferMemory;
				vk::deleter_ptr<VkBuffer> indexBuffer;
				vk::deleter_ptr<VkDeviceMemory> indexBufferMemory;
				vk::deleter_ptr<VkBuffer> instanceBuffer;
				vk::deleter_ptr<VkDeviceMemory> instanceBufferMemory;

				vk::deleter_ptr<VkBuffer> uniformStagingBuffer;
				vk::deleter_ptr<VkDeviceMemory> uniformStagingBufferMemory;
				vk::deleter_ptr<VkBuffer> uniformBuffer;
				vk::deleter_ptr<VkDeviceMemory> uniformBufferMemory;

				vk::deleter_ptr<VkDescriptorPool> descriptorPool;
				VkDescriptorSet descriptorSet;

				std::array<VkVertexInputBindingDescription, 2> getVertexBindingDescriptions();
				std::array<VkVertexInputAttributeDescription, 8> getVertexAttributeDescriptions();

				void setupDeleterPtrs();

				void createDescriptorSetLayout();
				void createGraphicsPipeline();
				void createUniformBuffer();
				void createDescriptorPool();

				void createTextureImage(const uint8_t* pixels, uint32_t width, uint32_t height);
				void createTextureImageView();
				void createTextureSampler();
				void createVertexBuffer(const std::vector<Vertex>& vertices);
				void createIndexBuffer(const std::vector<uint32_t>& indices);
				void createInstanceBuffer(const std::vector<ozz::math::Float4x4>& transforms);
				void createDescriptorSet();

				void updateVertexBuffer(const std::vector<Vertex>& vertices);
				void updateIndexBuffer(const std::vector<uint32_t>& indices);
				void updateInstanceBuffer(const std::vector<ozz::math::Float4x4>& transforms);
				void updateTextureImage(const uint8_t* pixels, uint32_t width, uint32_t height);

				// These series of function return whether or not the respective
				// buffer needed to be re-created as a result of the update.
				bool updateGeometryBufferObject(const GeometryBuffersObject& gbo);
				bool updateInstanceBufferObject(const InstancesBufferObject& ibo);
				bool updateTextureImageBufferObject(const TextureSamplerObject& tso);
				bool updateUniformBufferObject(const UniformBufferObject& ubo);
				
				void setDirty(bool bDirty);

			protected:

				// Gives a chance to create the render resources
				virtual bool onInitResources(internal::ContextVulkan* context) override;

				// Gives a chance to release all the owned resources
				virtual void onReleaseResources() override;

				// This function is called when command buffers are recorded,
				// between vkCmdBeginRenderPass() and vkCmdEndRenderPass()
				virtual bool onRegisterRenderPass(size_t commandIndex) override;

				// Because we can have multiple render passes,
				// this callback specify when the render context
				// has finished to register them all.
				virtual void onRenderPassesComplete() override;

				// Notified when the swap chain has changed, e.g. resized.
				virtual bool onSwapChainChange() override;

				// Made dirty whenever we have to recreate any of the buffers,
				// as these have to be re-bound to the render pass
				virtual bool isDirty() override;

			public:

				// Ctor
				ModelRenderState();
				virtual ~ModelRenderState();

				// Update render state according to the flags
				bool update(const UpdateData& updateData, uint32_t updateFlags);
			};

		}
	}
}

#endif //OZZ_SAMPLES_FRAMEWORK_MODELRENDERSTATE_VULKAN_H_