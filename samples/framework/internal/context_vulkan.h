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

#include <functional>
#include <vector>
#include <array>
#include <type_traits>

#include "ozz/base/maths/simd_math.h"
#include "ozz/base/maths/vec_float.h"

#include "framework/internal/tools_vulkan.h"

namespace ozz {
	namespace sample {

		namespace vk {
			class RenderState;
			class ModelRenderState;
		}

		namespace internal {

			// The majority of this code is taken from the tutorials provided at https://vulkan-tutorial.com/

			VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);
			void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator);

			// Manages all the boilerplate needed to handle Vulkan
			class ContextVulkan {
			private:

				friend class vk::RenderState;
				friend class vk::ModelRenderState;

				vk::deleter_ptr<VkInstance> instance{ vkDestroyInstance };
				vk::deleter_ptr<VkDebugReportCallbackEXT> callback{ instance, DestroyDebugReportCallbackEXT };
				vk::deleter_ptr<VkSurfaceKHR> surface{ instance, vkDestroySurfaceKHR };

				VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
				vk::deleter_ptr<VkDevice> device{ vkDestroyDevice };

				VkQueue graphicsQueue;
				VkQueue presentQueue;

				vk::deleter_ptr<VkSwapchainKHR> swapChain{ device, vkDestroySwapchainKHR };
				std::vector<VkImage> swapChainImages;
				VkFormat swapChainImageFormat;
				VkExtent2D swapChainExtent;
				std::vector<vk::deleter_ptr<VkImageView>> swapChainImageViews;
				std::vector<vk::deleter_ptr<VkFramebuffer>> swapChainFramebuffers;

				vk::deleter_ptr<VkRenderPass> renderPass{ device, vkDestroyRenderPass };
				vk::deleter_ptr<VkCommandPool> commandPool{ device, vkDestroyCommandPool };

				vk::deleter_ptr<VkImage> depthImage{ device, vkDestroyImage };
				vk::deleter_ptr<VkDeviceMemory> depthImageMemory{ device, vkFreeMemory };
				vk::deleter_ptr<VkImageView> depthImageView{ device, vkDestroyImageView };

				vk::deleter_ptr<VkDescriptorPool> descriptorPool{ device, vkDestroyDescriptorPool };
				VkDescriptorSet descriptorSet;

				std::vector<VkCommandBuffer> commandBuffers;

				vk::deleter_ptr<VkSemaphore> imageAvailableSemaphore{ device, vkDestroySemaphore };
				vk::deleter_ptr<VkSemaphore> renderFinishedSemaphore{ device, vkDestroySemaphore };

				std::vector<vk::RenderState*> renderStates;

				bool createInstance();
				bool setupDebugCallback();
				bool createSurface();
				bool pickPhysicalDevice();
				bool createLogicalDevice();
				bool createSwapChain(int32_t width, int32_t height);
				bool createSwapChainImageViews();
				bool createRenderPass();
				bool createCommandPool();
				bool createDepthResources();
				bool createFramebuffers();
				bool createCommandBuffers();
				bool createSemaphores();
				bool registerRenderState(vk::RenderState* renderState);

			public:

				bool initialize();
				void shutdown();
				bool drawFrame();
				bool recreateSwapChain();

				template<typename T, class... Args>
				vk::RenderState* createRenderState(Args&&... args);
				void destroyRenderState(vk::RenderState* renderState);
			};

			template<typename T, class... Args>
			vk::RenderState* ozz::sample::internal::ContextVulkan::createRenderState(Args&&... args)
			{
				vk::RenderState* renderState = memory::default_allocator()->New<T>(std::forward<Args>(args)...);
				if (!registerRenderState(renderState)) {
					memory::default_allocator()->Delete(renderState);
					renderState = nullptr;
				}

				return renderState;
			}

		}
	}
}

#endif // OZZ_SAMPLES_FRAMEWORK_INTERNAL_CONTEXT_VULKAN_H_
