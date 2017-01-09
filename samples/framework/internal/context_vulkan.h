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

#ifndef OZZ_SAMPLES_FRAMEWORK_INTERNAL_CONTEXT_VULKAN_H_
#define OZZ_SAMPLES_FRAMEWORK_INTERNAL_CONTEXT_VULKAN_H_

#ifndef OZZ_INCLUDE_PRIVATE_HEADER
#error "This header is private, it cannot be included from public headers."
#endif  // OZZ_INCLUDE_PRIVATE_HEADER

//#define VK_NO_PROTOTYPES            // do not declared prototypes, so I can load dynamically!
#include <vulkan/vulkan.h>

#include <functional>
#include <vector>
#include <array>

#include "ozz/base/maths/simd_math.h"
#include "ozz/base/maths/vec_float.h"

namespace ozz {
	namespace sample {
		namespace internal {

			// The majority of this code is taken from the tutorials provided at https://vulkan-tutorial.com/

			VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);
			void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator);

			template <typename T>
			class deleter_ptr {
			public:
				deleter_ptr() : deleter_ptr([](T, VkAllocationCallbacks*) {}) {}

				deleter_ptr(std::function<void(T, VkAllocationCallbacks*)> deletef) {
					delete_func = [=](T obj) { deletef(obj, nullptr); };
				}

				deleter_ptr(const deleter_ptr<VkInstance>& instance, std::function<void(VkInstance, T, VkAllocationCallbacks*)> deletef) {
					delete_func = [&instance, deletef](T obj) { deletef(instance, obj, nullptr); };
				}

				deleter_ptr(const deleter_ptr<VkDevice>& device, std::function<void(VkDevice, T, VkAllocationCallbacks*)> deletef) {
					delete_func = [&device, deletef](T obj) { deletef(device, obj, nullptr); };
				}

				~deleter_ptr() {
					cleanup();
				}

				const T* operator &() const {
					return &object_handle;
				}

				T* replace() {
					cleanup();
					return &object_handle;
				}

				operator T() const {
					return object_handle;
				}

				void operator=(T rhs) {
					if (rhs != object_handle) {
						cleanup();
						object_handle = rhs;
					}
				}

				template<typename V>
				bool operator==(V rhs) {
					return object_handle == T(rhs);
				}

			private:
				T object_handle{ VK_NULL_HANDLE };
				std::function<void(T)> delete_func;

				void cleanup() {
					if (object_handle != VK_NULL_HANDLE) {
						delete_func(object_handle);
					}
					object_handle = VK_NULL_HANDLE;
				}
			};

			struct Vertex {
				math::Float3 pos;
				math::Float3 color;
				math::Float2 texCoord;

				static VkVertexInputBindingDescription getBindingDescription() {
					VkVertexInputBindingDescription bindingDescription = {};
					bindingDescription.binding = 0;
					bindingDescription.stride = sizeof(Vertex);
					bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

					return bindingDescription;
				}

				static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
					std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};

					attributeDescriptions[0].binding = 0;
					attributeDescriptions[0].location = 0;
					attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
					attributeDescriptions[0].offset = offsetof(Vertex, pos);

					attributeDescriptions[1].binding = 0;
					attributeDescriptions[1].location = 1;
					attributeDescriptions[1].format = VK_FORMAT_R8G8B8_UNORM;
					attributeDescriptions[1].offset = offsetof(Vertex, color);

					attributeDescriptions[2].binding = 0;
					attributeDescriptions[2].location = 2;
					attributeDescriptions[2].format = VK_FORMAT_R8G8_UNORM;
					attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

					return attributeDescriptions;
				}

				bool operator==(const Vertex& other) const {
					return pos == other.pos && color == other.color && texCoord == other.texCoord;
				}
			};

			struct UniformBufferObject {
				math::Float4x4 model;
				math::Float4x4 view;
				math::Float4x4 proj;
			};

			// Manages all the boilerplate needed to handle Vulkan
			class ContextVulkan {
			private:

				deleter_ptr<VkInstance> instance{ vkDestroyInstance };
				deleter_ptr<VkDebugReportCallbackEXT> callback{ instance, DestroyDebugReportCallbackEXT };
				deleter_ptr<VkSurfaceKHR> surface{ instance, vkDestroySurfaceKHR };

				VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
				deleter_ptr<VkDevice> device{ vkDestroyDevice };

				VkQueue graphicsQueue;
				VkQueue presentQueue;

				deleter_ptr<VkSwapchainKHR> swapChain{ device, vkDestroySwapchainKHR };
				std::vector<VkImage> swapChainImages;
				VkFormat swapChainImageFormat;
				VkExtent2D swapChainExtent;
				std::vector<deleter_ptr<VkImageView>> swapChainImageViews;
				std::vector<deleter_ptr<VkFramebuffer>> swapChainFramebuffers;

				deleter_ptr<VkRenderPass> renderPass{ device, vkDestroyRenderPass };
				deleter_ptr<VkDescriptorSetLayout> descriptorSetLayout{ device, vkDestroyDescriptorSetLayout };
				deleter_ptr<VkPipelineLayout> pipelineLayout{ device, vkDestroyPipelineLayout };
				deleter_ptr<VkPipeline> graphicsPipeline{ device, vkDestroyPipeline };

				deleter_ptr<VkCommandPool> commandPool{ device, vkDestroyCommandPool };

				deleter_ptr<VkImage> depthImage{ device, vkDestroyImage };
				deleter_ptr<VkDeviceMemory> depthImageMemory{ device, vkFreeMemory };
				deleter_ptr<VkImageView> depthImageView{ device, vkDestroyImageView };

				bool createInstance();
				bool setupDebugCallback();
				bool createSurface();
				bool pickPhysicalDevice();
				bool createLogicalDevice();
				bool createSwapChain();
				bool createSwapChainImageViews();
				bool createRenderPass();
				bool createDescriptorSetLayout();
				bool createGraphicsPipeline();

			public:
				bool initialize();
				bool drawFrame();
			};

		}
	}
}

#endif // OZZ_SAMPLES_FRAMEWORK_INTERNAL_CONTEXT_VULKAN_H_
