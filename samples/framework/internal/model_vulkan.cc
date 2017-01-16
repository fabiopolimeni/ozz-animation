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
#include "framework/internal/model_vulkan.h"

std::array<VkVertexInputBindingDescription, 2> ozz::sample::vk::ModelRenderState::getVertexBindingDescriptions() {
	VkVertexInputBindingDescription geometryBindingDesc = { 0, sizeof(ozz::sample::vk::ModelRenderState::Vertex), VK_VERTEX_INPUT_RATE_VERTEX };
	VkVertexInputBindingDescription instanceBindingDesc = { 1, sizeof(ozz::math::Float4x4), VK_VERTEX_INPUT_RATE_INSTANCE };

	return { geometryBindingDesc, instanceBindingDesc };
}

std::array<VkVertexInputAttributeDescription, 8> ozz::sample::vk::ModelRenderState::getVertexAttributeDescriptions() {
	std::array<VkVertexInputAttributeDescription, 8> attributeDescriptions = {};

	// Geometry
	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, pos);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, normal);

	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(Vertex, uv);

	attributeDescriptions[3].binding = 0;
	attributeDescriptions[3].location = 3;
	attributeDescriptions[3].format = VK_FORMAT_R8G8B8A8_UNORM;
	attributeDescriptions[3].offset = offsetof(Vertex, color);

	// Instance
	attributeDescriptions[4].binding = 1;
	attributeDescriptions[4].location = 4;
	attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[4].offset = offsetof(math::Float4x4, cols[0]);

	attributeDescriptions[5].binding = 1;
	attributeDescriptions[5].location = 5;
	attributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[5].offset = offsetof(math::Float4x4, cols[1]);

	attributeDescriptions[6].binding = 1;
	attributeDescriptions[6].location = 6;
	attributeDescriptions[6].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[6].offset = offsetof(math::Float4x4, cols[2]);

	attributeDescriptions[7].binding = 1;
	attributeDescriptions[7].location = 7;
	attributeDescriptions[7].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[7].offset = offsetof(math::Float4x4, cols[3]);

	return attributeDescriptions;
}

void ozz::sample::vk::ModelRenderState::createDescriptorSetLayout() {
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.pImmutableSamplers = nullptr;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	CHECK_VK_RESULT(vkCreateDescriptorSetLayout(renderContext->device, &layoutInfo, nullptr, descriptorSetLayout.replace()));
}

void ozz::sample::vk::ModelRenderState::createGraphicsPipeline() {
	std::vector<char> vertShaderCode;
	std::vector<char> fragShaderCode;
	bool validShaders = vk::readFile("../shaders/vert.spv", vertShaderCode) && vk::readFile("../shaders/frag.spv", fragShaderCode);

	CHECK_AND_REPORT(validShaders, "Failed to load shader code. The graphics pipeline cannot be created");

	deleter_ptr<VkShaderModule> vertShaderModule{ renderContext->device, vkDestroyShaderModule };
	deleter_ptr<VkShaderModule> fragShaderModule{ renderContext->device, vkDestroyShaderModule };
	createShaderModule(renderContext->device, vertShaderCode, vertShaderModule);
	createShaderModule(renderContext->device, fragShaderCode, fragShaderModule);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	auto bindingDescriptions = getVertexBindingDescriptions();
	auto attributeDescriptions = getVertexAttributeDescriptions();

	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)renderContext->swapChainExtent.width;
	viewport.height = (float)renderContext->swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = renderContext->swapChainExtent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.stencilTestEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkDescriptorSetLayout setLayouts[] = { descriptorSetLayout };
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = setLayouts;

	CHECK_VK_RESULT(vkCreatePipelineLayout(renderContext->device, &pipelineLayoutInfo, nullptr, pipelineLayout.replace()));

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderContext->renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	CHECK_VK_RESULT(vkCreateGraphicsPipelines(renderContext->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, graphicsPipeline.replace()));
	setDirty(true);
}

void ozz::sample::vk::ModelRenderState::createTextureImage(const uint8_t* pixels, uint32_t width, uint32_t height) {
	// Store the texture's width and height, that is,
	// we can decide later whether the buffer requires
	// to be re-created when and update is requested.
	texWidth = width;
	texHeight = height;

	createImage(renderContext->physicalDevice, renderContext->device, width, height,
		VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

	updateTextureImage(pixels, width, height);
	setDirty(true);
}

void ozz::sample::vk::ModelRenderState::createTextureImageView() {
	createImageView(renderContext->device, textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, textureImageView);
	setDirty(true);
}

void ozz::sample::vk::ModelRenderState::createTextureSampler() {
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	CHECK_VK_RESULT(vkCreateSampler(renderContext->device, &samplerInfo, nullptr, textureSampler.replace()));
	setDirty(true);
}

void ozz::sample::vk::ModelRenderState::createVertexBuffer(const std::vector<Vertex>& vertices) {
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	// Store the number of vertices, that is, we can decide
	// later whether or not a buffer update requires to
	// re-create the vertex-buffer.
	numOfVertices = static_cast<uint32_t>(vertices.size());
	createBuffer(renderContext->physicalDevice, renderContext->device, bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
	
	updateVertexBuffer(vertices);	
	setDirty(true);
}

void ozz::sample::vk::ModelRenderState::createIndexBuffer(const std::vector<uint32_t>& indices) {
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	// Store the number of indices, that is, we can decide
	// later whether or not a buffer update requires to
	// re-create the index-buffer.
	numOfIndices = static_cast<uint32_t>(indices.size());
	createBuffer(renderContext->physicalDevice, renderContext->device, bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

	updateIndexBuffer(indices);
	setDirty(true);
}

void ozz::sample::vk::ModelRenderState::createInstanceBuffer(const std::vector<ozz::math::Float4x4>& transforms) {
	VkDeviceSize bufferSize = sizeof(transforms[0]) * transforms.size();

	// Store the number of instances, that is, we can decide
	// later whether or not a buffer update requires to
	// re-create the instance-buffer.
	numOfInstances = static_cast<uint32_t>(transforms.size());
	createBuffer(renderContext->physicalDevice, renderContext->device, bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, instanceBuffer, instanceBufferMemory);

	updateInstanceBuffer(transforms);
	setDirty(true);
}

void ozz::sample::vk::ModelRenderState::createUniformBuffer() {
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	createBuffer(renderContext->physicalDevice, renderContext->device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformStagingBuffer, uniformStagingBufferMemory);
	createBuffer(renderContext->physicalDevice, renderContext->device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, uniformBuffer, uniformBufferMemory);
	setDirty(true);
}

void ozz::sample::vk::ModelRenderState::createDescriptorPool() {
	std::array<VkDescriptorPoolSize, 2> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = 1;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = 1;

	CHECK_VK_RESULT(vkCreateDescriptorPool(renderContext->device, &poolInfo, nullptr, descriptorPool.replace()));
}

void ozz::sample::vk::ModelRenderState::createDescriptorSet() {
	VkDescriptorSetLayout layouts[] = { descriptorSetLayout };
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts;

	CHECK_VK_RESULT(vkAllocateDescriptorSets(renderContext->device, &allocInfo, &descriptorSet));

	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = uniformBuffer;
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof(UniformBufferObject);

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = textureImageView;
	imageInfo.sampler = textureSampler;

	std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};

	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = descriptorSet;
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pBufferInfo = &bufferInfo;

	descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[1].dstSet = descriptorSet;
	descriptorWrites[1].dstBinding = 1;
	descriptorWrites[1].dstArrayElement = 0;
	descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[1].descriptorCount = 1;
	descriptorWrites[1].pImageInfo = &imageInfo;

	vkUpdateDescriptorSets(renderContext->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	setDirty(true);
}

void ozz::sample::vk::ModelRenderState::updateVertexBuffer(const std::vector<Vertex>& vertices) {
	CHECK_AND_REPORT(vertexBuffer && vertexBufferMemory, "vertexBuffer and vertexBufferMemory have to be valid handles");
	CHECK_AND_REPORT(vertices.size() == numOfVertices, "To update the vertex buffer, the number of vertices have to match");

	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
	deleter_ptr<VkBuffer> stagingBuffer{ renderContext->device, vkDestroyBuffer };
	deleter_ptr<VkDeviceMemory> stagingBufferMemory{ renderContext->device, vkFreeMemory };
	createBuffer(renderContext->physicalDevice, renderContext->device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(renderContext->device, stagingBufferMemory, 0, bufferSize, 0, &data);
	{
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(renderContext->device, stagingBufferMemory);
	}

	VkCommandBuffer commandBuffer = beginSingleTimeCommand(renderContext->device, renderContext->commandPool);
	{
		copyBuffer(stagingBuffer, vertexBuffer, bufferSize, commandBuffer);
		endSingleTimeCommand(renderContext->device, renderContext->graphicsQueue, renderContext->commandPool, commandBuffer);
	}
}

void ozz::sample::vk::ModelRenderState::updateIndexBuffer(const std::vector<uint32_t>& indices) {
	CHECK_AND_REPORT(indexBuffer && indexBufferMemory, "indexBuffer and indexBufferMemory have to be valid handles");
	CHECK_AND_REPORT(indices.size() == numOfIndices, "To update the index buffer, the number of indices have to match");
	
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
	deleter_ptr<VkBuffer> stagingBuffer{ renderContext->device, vkDestroyBuffer };
	deleter_ptr<VkDeviceMemory> stagingBufferMemory{ renderContext->device, vkFreeMemory };
	createBuffer(renderContext->physicalDevice, renderContext->device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(renderContext->device, stagingBufferMemory, 0, bufferSize, 0, &data);
	{
		memcpy(data, indices.data(), (size_t)bufferSize);
		vkUnmapMemory(renderContext->device, stagingBufferMemory);
	}

	VkCommandBuffer commandBuffer = beginSingleTimeCommand(renderContext->device, renderContext->commandPool);
	{
		copyBuffer(stagingBuffer, indexBuffer, bufferSize, commandBuffer);
		endSingleTimeCommand(renderContext->device, renderContext->graphicsQueue, renderContext->commandPool, commandBuffer);
	}
}

void ozz::sample::vk::ModelRenderState::updateInstanceBuffer(const std::vector<ozz::math::Float4x4>& transforms) {
	CHECK_AND_REPORT(instanceBuffer && instanceBufferMemory, "instanceBuffer and instanceBufferMemory have to be valid handles");
	CHECK_AND_REPORT(transforms.size() == numOfInstances, "To update the instance buffer, the number of instances have to match");

	VkDeviceSize bufferSize = sizeof(transforms[0]) * transforms.size();
	deleter_ptr<VkBuffer> stagingBuffer{ renderContext->device, vkDestroyBuffer };
	deleter_ptr<VkDeviceMemory> stagingBufferMemory{ renderContext->device, vkFreeMemory };
	createBuffer(renderContext->physicalDevice, renderContext->device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(renderContext->device, stagingBufferMemory, 0, bufferSize, 0, &data);
	{
		memcpy(data, transforms.data(), (size_t)bufferSize);
		vkUnmapMemory(renderContext->device, stagingBufferMemory);
	}

	VkCommandBuffer commandBuffer = beginSingleTimeCommand(renderContext->device, renderContext->commandPool);
	{
		copyBuffer(stagingBuffer, instanceBuffer, bufferSize, commandBuffer);
		endSingleTimeCommand(renderContext->device, renderContext->graphicsQueue, renderContext->commandPool, commandBuffer);
	}
}
void ozz::sample::vk::ModelRenderState::updateTextureImage(const uint8_t* pixels, uint32_t width, uint32_t height) {
	CHECK_AND_REPORT(pixels, "Texture's pixels need to point to valid memory");
	CHECK_AND_REPORT(texWidth == width && texHeight == height, "To update texture image data, texture dimensions have to match");
	
	deleter_ptr<VkImage> stagingImage{ renderContext->device, vkDestroyImage };
	deleter_ptr<VkDeviceMemory> stagingImageMemory{ renderContext->device, vkFreeMemory };
	createImage(renderContext->physicalDevice, renderContext->device, width, height,
		VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingImage, stagingImageMemory);

	VkImageSubresource subresource = {};
	subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresource.mipLevel = 0;
	subresource.arrayLayer = 0;

	VkSubresourceLayout stagingImageLayout;
	vkGetImageSubresourceLayout(renderContext->device, stagingImage, &subresource, &stagingImageLayout);

	void* data;
	VkDeviceSize imageSize = width * height * 4;
	vkMapMemory(renderContext->device, stagingImageMemory, 0, imageSize, 0, &data);
	{
		if (stagingImageLayout.rowPitch == width * 4) {
			memcpy(data, pixels, (size_t)imageSize);
		}
		else {
			uint8_t* dataBytes = reinterpret_cast<uint8_t*>(data);

			for (uint32_t y = 0; y < height; y++) {
				memcpy(&dataBytes[y * stagingImageLayout.rowPitch], &pixels[y * width * 4], width * 4);
			}
		}

		vkUnmapMemory(renderContext->device, stagingImageMemory);
	}

	VkCommandBuffer commandBuffer = beginSingleTimeCommand(renderContext->device, renderContext->commandPool);
	{
		transitionImageLayout(stagingImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, commandBuffer);
		transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, commandBuffer);
		copyImage(stagingImage, textureImage, width, height, commandBuffer);

		transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, commandBuffer);

		endSingleTimeCommand(renderContext->device, renderContext->graphicsQueue, renderContext->commandPool, commandBuffer);
	}
}

bool ozz::sample::vk::ModelRenderState::updateGeometryBufferObject(const GeometryBuffersObject& gbo) {
	bool b_recreate = false;

	CHECK_AND_REPORT(gbo.vertices.size(), "Vertex buffer needs to contain valid data");
	if (gbo.vertices.size() != numOfVertices) {
		createVertexBuffer(gbo.vertices);
		b_recreate = true;
	}
	else {
		updateVertexBuffer(gbo.vertices);
	}

	CHECK_AND_REPORT(gbo.indices.size(), "Index buffer needs to contain valid data");
	if (gbo.indices.size() != numOfIndices) {
		createIndexBuffer(gbo.indices);
		b_recreate = true;
	}
	else {
		updateIndexBuffer(gbo.indices);
	}

	return b_recreate;
}

bool ozz::sample::vk::ModelRenderState::updateInstanceBufferObject(const InstancesBufferObject& ibo) {
	CHECK_AND_REPORT(ibo.transforms.size(), "Vertex buffer needs to contain valid data");
	if (ibo.transforms.size() != numOfInstances) {
		createInstanceBuffer(ibo.transforms);
		return true;
	}
	else {
		updateInstanceBuffer(ibo.transforms);
		return false;
	}
}

bool ozz::sample::vk::ModelRenderState::updateTextureImageBufferObject(const TextureSamplerObject& tso) {
	CHECK_AND_REPORT(tso.pixels, "Texture image object needs to point to valid data");
	CHECK_AND_REPORT(tso.width, "Texture image width can't be 0");
	CHECK_AND_REPORT(tso.height, "Texture image height can't be 0");

	if ((texWidth != tso.width) || (texHeight != tso.height)) {
		createTextureImage(tso.pixels, tso.width, tso.height);
		createTextureImageView();
		createTextureSampler();
		return true;
	}
	else {
		updateTextureImage(tso.pixels, tso.width, tso.height);
		return false;
	}
}

bool ozz::sample::vk::ModelRenderState::updateUniformBufferObject(const UniformBufferObject& ubo) {
	void* data;
	vkMapMemory(renderContext->device, uniformStagingBufferMemory, 0, sizeof(ubo), 0, &data);
	{
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(renderContext->device, uniformStagingBufferMemory);
	}

	VkCommandBuffer commandBuffer = beginSingleTimeCommand(renderContext->device, renderContext->commandPool);
	{
		copyBuffer(uniformStagingBuffer, uniformBuffer, sizeof(ubo), commandBuffer);
		endSingleTimeCommand(renderContext->device, renderContext->graphicsQueue, renderContext->commandPool, commandBuffer);
	}

	// NOTE: Can't change the uniform buffer size at runtime
	return false;
}

ozz::sample::vk::ModelRenderState::ModelRenderState()
	: numOfVertices(0), numOfIndices(0), numOfInstances(0)
	, texWidth(0), texHeight(0), dirty(false) {

}

ozz::sample::vk::ModelRenderState::~ModelRenderState() {

}

void ozz::sample::vk::ModelRenderState::setupDeleterPtrs() {
	descriptorSetLayout = { renderContext->device, vkDestroyDescriptorSetLayout };
	pipelineLayout = { renderContext->device, vkDestroyPipelineLayout };
	graphicsPipeline = { renderContext->device, vkDestroyPipeline };
	textureImage = { renderContext->device, vkDestroyImage };
	textureImageMemory = { renderContext->device, vkFreeMemory };
	textureImageView = { renderContext->device, vkDestroyImageView };
	textureSampler = { renderContext->device, vkDestroySampler };
	vertexBuffer = { renderContext->device, vkDestroyBuffer };
	vertexBufferMemory = { renderContext->device, vkFreeMemory };
	indexBuffer = { renderContext->device, vkDestroyBuffer };
	indexBufferMemory = { renderContext->device, vkFreeMemory };
	uniformStagingBuffer = { renderContext->device, vkDestroyBuffer };
	uniformStagingBufferMemory = { renderContext->device, vkFreeMemory };
	uniformBuffer = { renderContext->device, vkDestroyBuffer };
	uniformBufferMemory = { renderContext->device, vkFreeMemory };
	descriptorPool = { renderContext->device, vkDestroyDescriptorPool };
}

bool ozz::sample::vk::ModelRenderState::onInitResources(internal::ContextVulkan* context) {
	if (RenderState::onInitResources(context)) {
		setupDeleterPtrs();

		createDescriptorSetLayout();
		createGraphicsPipeline();
		createUniformBuffer();
		createDescriptorPool();
		return true;
	}

	return false;
}

void ozz::sample::vk::ModelRenderState::onReleaseResources() {
	RenderState::onReleaseResources();
}

bool ozz::sample::vk::ModelRenderState::onRegisterRenderPass(size_t commandIndex) {	
	VkCommandBuffer commandBuffer = renderContext->commandBuffers[commandIndex];
	
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, offsets);
	vkCmdBindVertexBuffers(commandBuffer, 1, 1, &instanceBuffer, offsets);
	vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
	vkCmdDrawIndexed(commandBuffer, numOfIndices, 1, 0, 0, 0);

	return true;
}

void ozz::sample::vk::ModelRenderState::onRenderPassesComplete() {
	setDirty(false);
}

bool ozz::sample::vk::ModelRenderState::onSwapChainChange() {
	createGraphicsPipeline();
	return true;
}

bool ozz::sample::vk::ModelRenderState::isDirty() {
	return dirty;
}

void ozz::sample::vk::ModelRenderState::setDirty(bool bDirty) {
	dirty = bDirty;
}

bool ozz::sample::vk::ModelRenderState::init(const InitData& initData) {
	// Geometry
	createVertexBuffer(initData.gbo.vertices);
	createIndexBuffer(initData.gbo.indices);

	// Texture
	createTextureImage(initData.tso.pixels, initData.tso.width, initData.tso.height);
	createTextureImageView();
	createTextureSampler();

	// The descriptor set depends on the above buffers
	createDescriptorSet();

	return true;
}

// Filter what to update based on the set flags
bool ozz::sample::vk::ModelRenderState::update(const UpdateData& updateData) {
	bool b_input_assambly_modified = false;
	bool b_recreate_descriptor_set = false;
	
	// Geometry buffers
	if (updateData.flags & (UpdateData::UDF_VERTEX_BUFFER | UpdateData::UDF_INDEX_BUFFER)) {
		b_input_assambly_modified |= updateGeometryBufferObject(updateData.gbo);
	}

	// Instances buffer
	if (updateData.flags & (UpdateData::UDF_INSTANCE_BUFFER)) {
		b_input_assambly_modified |= updateInstanceBufferObject(updateData.ibo);
	}

	// Texture buffer
	if (updateData.flags & (UpdateData::UDF_TEXTURE_IMAGE_BUFFER)) {
		b_recreate_descriptor_set |= updateTextureImageBufferObject(updateData.tso);
	}

	// Uniform buffer
	if (updateData.flags & (UpdateData::UDF_UNIFORM_BUFFER)) {
		b_recreate_descriptor_set |= updateUniformBufferObject(updateData.ubo);
	}

	if (b_recreate_descriptor_set) {
		createDescriptorSet();
	}

	return true;
}
