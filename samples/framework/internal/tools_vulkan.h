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

#ifndef OZZ_SAMPLES_FRAMEWORK_TOOLS_VULKAN_H_
#define OZZ_SAMPLES_FRAMEWORK_TOOLS_VULKAN_H_

#ifndef OZZ_INCLUDE_PRIVATE_HEADER
#error "This header is private, it cannot be included from public headers."
#endif  // OZZ_INCLUDE_PRIVATE_HEADER

#include <vulkan/vulkan.h>

#include <functional>
#include <vector>	
#include <string>

namespace ozz {
	namespace sample {
		namespace vk {

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

			std::string getErrorString(VkResult errorCode);
			
			bool checkValidationLayerSupport(const std::vector<const char*>& validationLayers);
			
			VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
				VkDebugReportFlagsEXT flags,
				VkDebugReportObjectTypeEXT /*objType*/,
				uint64_t /*obj*/,
				size_t /*location*/,
				int32_t /*code*/,
				const char* layerPrefix,
				const char* msg,
				void* /*userData*/);

			struct QueueFamilyIndices {
				int graphicsFamily = -1;
				int presentFamily = -1;
			};

			bool isQueryComplete(const QueueFamilyIndices& queryIndices);

			QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

			bool checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice, const std::vector<const char*>& deviceExtensions);

			struct SwapChainSupportDetails {
				VkSurfaceCapabilitiesKHR capabilities;
				std::vector<VkSurfaceFormatKHR> formats;
				std::vector<VkPresentModeKHR> presentModes;
			};

			SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

			bool isDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions);

			VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

			VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);

			VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, int32_t width, int32_t height);

			VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates,
				VkImageTiling tiling, VkFormatFeatureFlags features);

			VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);

			bool readFile(const std::string& filename, std::vector<char>& buffer);

			void createShaderModule(VkDevice device, const std::vector<char>& code, ozz::sample::vk::deleter_ptr<VkShaderModule>& shaderModule);

			VkCommandBuffer beginSingleTimeCommand(VkDevice device, VkCommandPool commandPool);

			void endSingleTimeCommand(VkDevice device, VkQueue commandQueue, VkCommandPool commandPool, VkCommandBuffer commandBuffer);

			bool hasStencilComponent(VkFormat format);

			bool findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties, uint32_t& memoryType);

			void createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
				ozz::sample::vk::deleter_ptr<VkImageView>& imageView);

			void createImage(VkPhysicalDevice physicalDevice, VkDevice device,
				uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
				VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
				ozz::sample::vk::deleter_ptr<VkImage>& image,
				ozz::sample::vk::deleter_ptr<VkDeviceMemory>& imageMemory);

			bool transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandBuffer commandBuffer);

			void copyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height, VkCommandBuffer commandBuffer);

			void createBuffer(VkPhysicalDevice physicalDevice, VkDevice device,
				VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
				deleter_ptr<VkBuffer>& buffer, deleter_ptr<VkDeviceMemory>& bufferMemory);

			void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandBuffer commandBuffer);

			void reportFail(const char* funcCall, VkResult result, const char* file, int32_t line);

		} // namespace vk
	} // namespace sample
} // namespace ozz

// Macro to check invalid statement
#define CHECK_AND_REPORT(cond, msg) if (!cond) ozz::sample::vk::reportFail(msg, VK_SUCCESS, __FILE__, __LINE__)

// Macro to check Vulkan return results. We can do the follow because VK_SUCCESS == 0
#define CHECK_VK_RESULT(func) if (VkResult res = (func)) ozz::sample::vk::reportFail(#func, res, __FILE__, __LINE__)

#endif // OZZ_SAMPLES_FRAMEWORK_INTERNAL_CONTEXT_VULKAN_H_