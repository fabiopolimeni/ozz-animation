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

#include "framework/internal/tools_vulkan.h"
#include "ozz/base/log.h"

#include "assert.h"

#include <cstdio>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <set>

#include "glfw/glfw3.h"

extern GLFWwindow* g_glfwWindow;

namespace ozz {
	namespace sample {
		namespace vk {

			std::string getErrorString(VkResult errorCode)
			{
				switch (errorCode)
				{
#define STR(r) case VK_ ##r: return #r

					STR(NOT_READY);
					STR(TIMEOUT);
					STR(EVENT_SET);
					STR(EVENT_RESET);
					STR(INCOMPLETE);
					STR(ERROR_OUT_OF_HOST_MEMORY);
					STR(ERROR_OUT_OF_DEVICE_MEMORY);
					STR(ERROR_INITIALIZATION_FAILED);
					STR(ERROR_DEVICE_LOST);
					STR(ERROR_MEMORY_MAP_FAILED);
					STR(ERROR_LAYER_NOT_PRESENT);
					STR(ERROR_EXTENSION_NOT_PRESENT);
					STR(ERROR_FEATURE_NOT_PRESENT);
					STR(ERROR_INCOMPATIBLE_DRIVER);
					STR(ERROR_TOO_MANY_OBJECTS);
					STR(ERROR_FORMAT_NOT_SUPPORTED);
					STR(ERROR_SURFACE_LOST_KHR);
					STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
					STR(SUBOPTIMAL_KHR);
					STR(ERROR_OUT_OF_DATE_KHR);
					STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
					STR(ERROR_VALIDATION_FAILED_EXT);
					STR(ERROR_INVALID_SHADER_NV);
#undef STR

				default:
					return "UNKNOWN_ERROR";
				}
			}

			bool checkValidationLayerSupport(const std::vector<const char*>& validationLayers) {
				uint32_t layerCount;
				vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

				std::vector<VkLayerProperties> availableLayers(layerCount);
				vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

				for (const char* layerName : validationLayers) {
					for (const auto& layerProperties : availableLayers) {
						if (strcmp(layerName, layerProperties.layerName) == 0) {
							return true;
						}
					}
				}

				return false;
			}

			std::vector<const char*> getRequiredExtensions() {
				std::vector<const char*> extensions;

				unsigned int glfwExtensionCount = 0;
				const char** glfwExtensions;
				glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

				for (unsigned int i = 0; i < glfwExtensionCount; i++) {
					extensions.push_back(glfwExtensions[i]);
				}

				extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
				return extensions;
			}

			VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
				VkDebugReportFlagsEXT flags,
				VkDebugReportObjectTypeEXT /*objType*/,
				uint64_t /*obj*/,
				size_t /*location*/,
				int32_t /*code*/,
				const char* layerPrefix,
				const char* msg,
				void* /*userData*/) {

				std::ostringstream stream;
				stream << "VKDBG: ";
				if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
					stream << "INFO | ";
				}
				if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
					stream << "WARNING | ";
				}
				if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
					stream << "PERFORMANCE | ";
				}
				if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
					stream << "ERROR |";
				}
				if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
					stream << "DEBUG | ";
				}

				stream << "@[" << layerPrefix << "]: ";
				stream << msg << std::endl;
				ozz::log::Log() << stream.str();

				if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
					assert(!(flags & VK_DEBUG_REPORT_ERROR_BIT_EXT));
					std::exit(EXIT_FAILURE);
				}

				return VK_FALSE;
			}

			bool isQueryComplete(const QueueFamilyIndices& queryIndices)
			{
				return queryIndices.graphicsFamily >= 0 && queryIndices.presentFamily >= 0;
			}

			QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
				QueueFamilyIndices indices;

				uint32_t queueFamilyCount = 0;
				vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

				std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
				vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

				int i = 0;
				for (const auto& queueFamily : queueFamilies) {
					if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
						indices.graphicsFamily = i;
					}

					VkBool32 presentSupport = false;
					vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

					if (queueFamily.queueCount > 0 && presentSupport) {
						indices.presentFamily = i;
					}

					if (vk::isQueryComplete(indices)) {
						break;
					}

					i++;
				}

				return indices;
			}

			bool checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice, const std::vector<const char*>& deviceExtensions) {
				uint32_t extensionCount;
				vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

				std::vector<VkExtensionProperties> availableExtensions(extensionCount);
				vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

				std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

				for (const auto& extension : availableExtensions) {
					requiredExtensions.erase(extension.extensionName);
				}

				return requiredExtensions.empty();
			}

			SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
				SwapChainSupportDetails details;

				vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

				uint32_t formatCount;
				vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

				if (formatCount != 0) {
					details.formats.resize(formatCount);
					vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
				}

				uint32_t presentModeCount;
				vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

				if (presentModeCount != 0) {
					details.presentModes.resize(presentModeCount);
					vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentModes.data());
				}

				return details;
			}

			bool isDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions) {
				QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

				bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice, deviceExtensions);

				bool swapChainAdequate = false;
				if (extensionsSupported) {
					SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);
					swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
				}

				return vk::isQueryComplete(indices) && extensionsSupported && swapChainAdequate;
			}

			VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
				if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
					return{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
				}

				for (const auto& availableFormat : availableFormats) {
					if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
						return availableFormat;
					}
				}

				return availableFormats[0];
			}

			VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes) {
				for (const auto& availablePresentMode : availablePresentModes) {
					if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
						return availablePresentMode;
					}
				}

				// FIFO is guaranteed to be available
				return VK_PRESENT_MODE_FIFO_KHR;
			}

			VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
				if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
					return capabilities.currentExtent;
				}
				else {
					int32_t width_, height_;
					glfwGetFramebufferSize(g_glfwWindow, &width_, &height_);

					VkExtent2D actualExtent = { static_cast<uint32_t>(width_), static_cast<uint32_t>(height_) };
					actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
					actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

					return actualExtent;
				}
			}

			VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
				for (VkFormat format : candidates) {
					VkFormatProperties props;
					vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

					if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
						return format;
					}
					else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
						return format;
					}
				}

				return VK_FORMAT_UNDEFINED;
			}

			VkFormat findDepthFormat(VkPhysicalDevice physicalDevice) {
				return findSupportedFormat(physicalDevice,
				{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
					VK_IMAGE_TILING_OPTIMAL,
					VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
				);
			}

			bool readFile(const std::string& filename, std::vector<char>& buffer) {
				std::ifstream file(filename, std::ios::ate | std::ios::binary);

				if (!file.is_open()) {
					return false;
				}

				size_t fileSize = (size_t)file.tellg();
				buffer.resize(fileSize);

				file.seekg(0);
				file.read(buffer.data(), fileSize);

				file.close();

				return true;
			}

			void createShaderModule(VkDevice device, const std::vector<char>& code, ozz::sample::vk::deleter_ptr<VkShaderModule>& shaderModule) {
				VkShaderModuleCreateInfo createInfo = {};
				createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				createInfo.codeSize = code.size();
				createInfo.pCode = (uint32_t*)code.data();

				VK_CHECK_RESULT(vkCreateShaderModule(device, &createInfo, nullptr, shaderModule.replace()));
			}

			VkCommandBuffer beginSingleTimeCommand(VkDevice device, VkCommandPool commandPool) {
				VkCommandBufferAllocateInfo allocInfo = {};
				allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
				allocInfo.commandPool = commandPool;
				allocInfo.commandBufferCount = 1;

				VkCommandBuffer commandBuffer;
				vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

				VkCommandBufferBeginInfo beginInfo = {};
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

				vkBeginCommandBuffer(commandBuffer, &beginInfo);

				return commandBuffer;
			}

			void endSingleTimeCommand(VkDevice device, VkQueue commandQueue, VkCommandPool commandPool, VkCommandBuffer commandBuffer) {
				vkEndCommandBuffer(commandBuffer);

				VkSubmitInfo submitInfo = {};
				submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				submitInfo.commandBufferCount = 1;
				submitInfo.pCommandBuffers = &commandBuffer;

				vkQueueSubmit(commandQueue, 1, &submitInfo, VK_NULL_HANDLE);
				vkQueueWaitIdle(commandQueue);

				vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
			}

			bool hasStencilComponent(VkFormat format) {
				return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
			}

			bool findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties, uint32_t& memoryType) {
				VkPhysicalDeviceMemoryProperties memProperties;
				vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

				for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
					if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
						memoryType = i;
						return true;
					}
				}

				return false;
			}

			void createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
				ozz::sample::vk::deleter_ptr<VkImageView>& imageView) {
				VkImageViewCreateInfo viewInfo = {};
				viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				viewInfo.image = image;
				viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				viewInfo.format = format;
				viewInfo.subresourceRange.aspectMask = aspectFlags;
				viewInfo.subresourceRange.baseMipLevel = 0;
				viewInfo.subresourceRange.levelCount = 1;
				viewInfo.subresourceRange.baseArrayLayer = 0;
				viewInfo.subresourceRange.layerCount = 1;

				VK_CHECK_RESULT(vkCreateImageView(device, &viewInfo, nullptr, imageView.replace()));
			}

			void createImage(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
				VkMemoryPropertyFlags properties, ozz::sample::vk::deleter_ptr<VkImage>& image, ozz::sample::vk::deleter_ptr<VkDeviceMemory>& imageMemory) {
				VkImageCreateInfo imageInfo = {};
				imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
				imageInfo.imageType = VK_IMAGE_TYPE_2D;
				imageInfo.extent.width = width;
				imageInfo.extent.height = height;
				imageInfo.extent.depth = 1;
				imageInfo.mipLevels = 1;
				imageInfo.arrayLayers = 1;
				imageInfo.format = format;
				imageInfo.tiling = tiling;
				imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
				imageInfo.usage = usage;
				imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
				imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

				VK_CHECK_RESULT(vkCreateImage(device, &imageInfo, nullptr, image.replace()));

				VkMemoryRequirements memRequirements;
				vkGetImageMemoryRequirements(device, image, &memRequirements);

				VkMemoryAllocateInfo allocInfo = {};
				allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				allocInfo.allocationSize = memRequirements.size;

				findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties, allocInfo.memoryTypeIndex);

				VK_CHECK_RESULT(vkAllocateMemory(device, &allocInfo, nullptr, imageMemory.replace()));
				VK_CHECK_RESULT(vkBindImageMemory(device, image, imageMemory, 0));
			}

			bool transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandBuffer commandBuffer) {
				VkImageMemoryBarrier barrier = {};
				barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrier.oldLayout = oldLayout;
				barrier.newLayout = newLayout;
				barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.image = image;

				if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
					barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

					if (hasStencilComponent(format)) {
						barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
					}
				}
				else {
					barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				}

				barrier.subresourceRange.baseMipLevel = 0;
				barrier.subresourceRange.levelCount = 1;
				barrier.subresourceRange.baseArrayLayer = 0;
				barrier.subresourceRange.layerCount = 1;

				if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
					barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
					barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				}
				else if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
					barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
					barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				}
				else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
					barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				}
				else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
					barrier.srcAccessMask = 0;
					barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				}
				else {
					return false;
				}

				vkCmdPipelineBarrier(
					commandBuffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
					0,
					0, nullptr,
					0, nullptr,
					1, &barrier
				);

				return true;
			}

			void copyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height, VkCommandBuffer commandBuffer) {
				VkImageSubresourceLayers subResource = {};
				subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				subResource.baseArrayLayer = 0;
				subResource.mipLevel = 0;
				subResource.layerCount = 1;

				VkImageCopy region = {};
				region.srcSubresource = subResource;
				region.dstSubresource = subResource;
				region.srcOffset = { 0, 0, 0 };
				region.dstOffset = { 0, 0, 0 };
				region.extent.width = width;
				region.extent.height = height;
				region.extent.depth = 1;

				vkCmdCopyImage(
					commandBuffer,
					srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1, &region
				);
			}
		} // namespace vk
	} // namespace sample
} // namespace ozz