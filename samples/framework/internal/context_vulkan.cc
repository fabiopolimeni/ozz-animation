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

#include "framework/application.h"
#include "framework/internal/context_vulkan.h"
#include "framework/internal/renderstate_vulkan.h"

#include "glfw/glfw3.h"

#include <set>

extern GLFWwindow* g_glfwWindow;

#ifdef _DEBUG
bool g_enableValidationLayers = true;
#else
bool g_enableValidationLayers = false;
#endif

static const std::vector<const char*> g_validationLayers = {
	"VK_LAYER_LUNARG_standard_validation"
};

static const std::vector<const char*> g_deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

VkResult ozz::sample::internal::CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) {
	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void ozz::sample::internal::DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	if (func != nullptr) {
		func(instance, callback, pAllocator);
	}
}

namespace {
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
}

bool ozz::sample::internal::ContextVulkan::createInstance() {
	if (g_enableValidationLayers && !vk::checkValidationLayerSupport(g_validationLayers)) {
		ozz::log::Err() << "Validation layers requested, but not available!" << std::endl;
		return false;
	}

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = ozz::sample::Application::GetInstance()->GetTile().c_str();
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName =  "Ozz Sample Framework";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extensions = getRequiredExtensions();
	if (g_enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	if (g_enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(g_validationLayers.size());
		createInfo.ppEnabledLayerNames = g_validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateInstance(&createInfo, nullptr, instance.replace()) != VK_SUCCESS) {
		ozz::log::Err() << "Failed to create Vulkan instance!" << std::endl;
		return false;
	}

	return true;
}

bool ozz::sample::internal::ContextVulkan::setupDebugCallback() {
	if (!g_enableValidationLayers)
		return true;

	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	createInfo.pfnCallback = vk::debugCallback;

	if (CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, callback.replace()) != VK_SUCCESS) {
		ozz::log::Err() << "Failed to set up debug callback!" << std::endl;
		return false;
	}

	return true;
}

bool ozz::sample::internal::ContextVulkan::createSurface() {
	if (glfwCreateWindowSurface(instance, g_glfwWindow, nullptr, surface.replace()) != VK_SUCCESS) {
		ozz::log::Err() << "Failed to create window surface!" << std::endl;
		return false;
	}

	return true;
}

bool ozz::sample::internal::ContextVulkan::pickPhysicalDevice() {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		ozz::log::Err() << "Failed to find GPUs with Vulkan support!!" << std::endl;
		return false;
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	for (const auto& phys_device : devices) {
		if (vk::isDeviceSuitable(phys_device, surface, g_deviceExtensions)) {
			physicalDevice = phys_device;
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE) {
		ozz::log::Err() << "Failed to find a suitable GPU!" << std::endl;
		return false;
	}

	return true;
}

bool ozz::sample::internal::ContextVulkan::createLogicalDevice() {
	vk::QueueFamilyIndices indices = vk::findQueueFamilies(physicalDevice, surface);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<int> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

	float queuePriority = 1.0f;
	for (int queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(g_deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = g_deviceExtensions.data();

	if (g_enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(g_validationLayers.size());
		createInfo.ppEnabledLayerNames = g_validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, device.replace()) != VK_SUCCESS) {
		ozz::log::Err() << "Failed to find a suitable GPU!" << std::endl;
		return false;
	}

	vkGetDeviceQueue(device, indices.graphicsFamily, 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily, 0, &presentQueue);

	return true;
}

bool ozz::sample::internal::ContextVulkan::createSwapChain(int32_t width, int32_t height) {
	vk::SwapChainSupportDetails swapChainSupport = vk::querySwapChainSupport(physicalDevice, surface);

	VkSurfaceFormatKHR surfaceFormat = vk::chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = vk::chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = vk::chooseSwapExtent(swapChainSupport.capabilities, width, height);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	vk::QueueFamilyIndices indices = vk::findQueueFamilies(physicalDevice, surface);
	uint32_t queueFamilyIndices[] = { (uint32_t)indices.graphicsFamily, (uint32_t)indices.presentFamily };

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	VkSwapchainKHR oldSwapChain = swapChain;
	createInfo.oldSwapchain = oldSwapChain;

	VkSwapchainKHR newSwapChain;
	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &newSwapChain) != VK_SUCCESS) {
		ozz::log::Err() << "Failed to create a swap chain!" << std::endl;
		return false;
	}

	swapChain = newSwapChain;

	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;

	return true;
}

bool ozz::sample::internal::ContextVulkan::createSwapChainImageViews() {
	swapChainImageViews.resize(swapChainImages.size(), ozz::sample::vk::deleter_ptr<VkImageView>{device, vkDestroyImageView});

	for (uint32_t i = 0; i < swapChainImages.size(); i++) {
		vk::createImageView(device, swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, swapChainImageViews[i]);
	}

	return true;
}

bool ozz::sample::internal::ContextVulkan::createRenderPass()
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = vk::findDepthFormat(physicalDevice);
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	CHECK_VK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, renderPass.replace()));
	return true;
}

bool ozz::sample::internal::ContextVulkan::createCommandPool()
{
	vk::QueueFamilyIndices queueFamilyIndices = vk::findQueueFamilies(physicalDevice, surface);

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

	CHECK_VK_RESULT(vkCreateCommandPool(device, &poolInfo, nullptr, commandPool.replace()));
	return true;
}

bool ozz::sample::internal::ContextVulkan::createDepthResources()
{
	VkFormat depthFormat = vk::findDepthFormat(physicalDevice);

	vk::createImage(physicalDevice, device, swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
	vk::createImageView(device, depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, depthImageView);

	VkCommandBuffer commandBuffer = vk::beginSingleTimeCommand(device, commandPool);
	bool success = vk::transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, commandBuffer);
	vk::endSingleTimeCommand(device, graphicsQueue, commandPool, commandBuffer);

	return success;
}

bool ozz::sample::internal::ContextVulkan::createFramebuffers()
{
	swapChainFramebuffers.resize(swapChainImageViews.size(), vk::deleter_ptr<VkFramebuffer>{device, vkDestroyFramebuffer});

	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
		std::array<VkImageView, 2> attachments = {
			swapChainImageViews[i],
			depthImageView
		};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		CHECK_VK_RESULT(vkCreateFramebuffer(device, &framebufferInfo, nullptr, swapChainFramebuffers[i].replace()));
	}

	return true;
}

bool ozz::sample::internal::ContextVulkan::createCommandBuffers()
{
	if (commandBuffers.size() > 0) {
		vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
	}

	commandBuffers.resize(swapChainFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

	CHECK_VK_RESULT(vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()));

	for (size_t i = 0; i < commandBuffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		CHECK_VK_RESULT(vkBeginCommandBuffer(commandBuffers[i], &beginInfo)); {
			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderPass;
			renderPassInfo.framebuffer = swapChainFramebuffers[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = swapChainExtent;

			std::array<VkClearValue, 2> clearValues = {};
			clearValues[0].color = { .4f, .42f, .38f, 1.0f };
			clearValues[1].depthStencil = { 1.0f, 0 };

			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE); {

				// Iterate through all the render states and
				// register their binding and drawing operations.
				for (auto* rs : renderStates) {
					rs->onRegisterRenderPass(i);
				}

				vkCmdEndRenderPass(commandBuffers[i]);
			}

			CHECK_VK_RESULT(vkEndCommandBuffer(commandBuffers[i]));
		}
	}

	return true;
}

bool ozz::sample::internal::ContextVulkan::createSemaphores()
{
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	CHECK_VK_RESULT(vkCreateSemaphore(device, &semaphoreInfo, nullptr, imageAvailableSemaphore.replace()));
	CHECK_VK_RESULT(vkCreateSemaphore(device, &semaphoreInfo, nullptr, renderFinishedSemaphore.replace()));
	return true;
}

bool ozz::sample::internal::ContextVulkan::initialize()
{
	int32_t width, height;
	glfwGetFramebufferSize(g_glfwWindow, &width, &height);

	// This will guarantee that each following function will
	// be called only if every previous one has succeeded.
	return createInstance()
		&& setupDebugCallback()
		&& createSurface()
		&& pickPhysicalDevice()
		&& createLogicalDevice()
		&& createSwapChain(width, height)
		&& createSwapChainImageViews()
		&& createRenderPass()
		&& createCommandPool()
		&& createDepthResources()
		&& createFramebuffers()
		&& createCommandBuffers()
		&& createSemaphores()
		;
}

void ozz::sample::internal::ContextVulkan::shutdown()
{
	vkDeviceWaitIdle(device);
}

bool ozz::sample::internal::ContextVulkan::drawFrame()
{
	// It might happen that we have re-created a render state
	// since last time we issued a render command, therefore,
	// we eventually need to register its render-pass commands
	// to the render command buffer.
	bool b_recreate_coommand_buffers = false;
	for (auto* rs : renderStates) {
		b_recreate_coommand_buffers |= rs->isDirty();
	}

	if (b_recreate_coommand_buffers) {
		createCommandBuffers();
	}

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(device, swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapChain();
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		ozz::log::Err() << "Failed to acquire swap chain image!" << std::endl;
		return false;
	}

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
		ozz::log::Err() << "Failed to submit draw command buffer!" << std::endl;
		return false;
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		recreateSwapChain();
	}
	else if (result != VK_SUCCESS) {
		ozz::log::Err() << "Failed to present swap chain image!" << std::endl;
		return false;
	}

	return true;
}

bool ozz::sample::internal::ContextVulkan::recreateSwapChain()
{
	int32_t width, height;
	glfwGetFramebufferSize(g_glfwWindow, &width, &height);

	vkDeviceWaitIdle(device);
	bool result =
		   createSwapChain(width, height)
		&& createSwapChainImageViews()
		&& createRenderPass()
		&& createDepthResources()
		&& createFramebuffers()
		&& createCommandBuffers();

	// Iterate through all the render states
	// to notify swap chain has changed.
	for (auto* rs : renderStates) {
		result |= rs->onSwapChainChange();
	}
	
	return result;
}

bool ozz::sample::internal::ContextVulkan::registerRenderState(vk::RenderState* renderState)
{
	vkDeviceWaitIdle(device);
	if (renderState->onInitResources(this)) {
		renderStates.push_back(renderState);
		return true;
	}

	return false;
}

void ozz::sample::internal::ContextVulkan::destroyRenderState(vk::RenderState* renderState)
{
	vkDeviceWaitIdle(device);
	auto rsIt = std::find(renderStates.cbegin(), renderStates.cend(), renderState);
	if (rsIt != std::end(renderStates)) {
		renderState->onReleaseResources();
		renderStates.erase(rsIt);
	}

	memory::default_allocator()->Delete(renderState);
}
