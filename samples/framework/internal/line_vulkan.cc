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
#include "framework/internal/line_vulkan.h"

std::array<VkVertexInputBindingDescription, 1> ozz::sample::vk::LineRenderState::getVertexBindingDescriptions()
{
	VkVertexInputBindingDescription vertexBindingDesc = { 0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX };

	return{ vertexBindingDesc };
}

std::array<VkVertexInputAttributeDescription, 2> ozz::sample::vk::LineRenderState::getVertexAttributeDescriptions()
{
	std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

	// Geometry
	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, pos);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R8G8B8A8_UNORM;
	attributeDescriptions[1].offset = offsetof(Vertex, color);

	return attributeDescriptions;

}

void ozz::sample::vk::LineRenderState::setupDeleterPtrs()
{
	descriptorSetLayout = { renderContext->device, vkDestroyDescriptorSetLayout };
	pipelineLayout = { renderContext->device, vkDestroyPipelineLayout };
	graphicsPipeline = { renderContext->device, vkDestroyPipeline };
	vertexBuffer = { renderContext->device, vkDestroyBuffer };
	vertexBufferMemory = { renderContext->device, vkFreeMemory };
	uniformStagingBuffer = { renderContext->device, vkDestroyBuffer };
	uniformStagingBufferMemory = { renderContext->device, vkFreeMemory };
	uniformBuffer = { renderContext->device, vkDestroyBuffer };
	uniformBufferMemory = { renderContext->device, vkFreeMemory };
	descriptorPool = { renderContext->device, vkDestroyDescriptorPool };
}

void ozz::sample::vk::LineRenderState::createDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.pImmutableSamplers = nullptr;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	std::array<VkDescriptorSetLayoutBinding, 1> bindings = { uboLayoutBinding };
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	CHECK_VK_RESULT(vkCreateDescriptorSetLayout(renderContext->device, &layoutInfo, nullptr, descriptorSetLayout.replace()));
}

void ozz::sample::vk::LineRenderState::createGraphicsPipeline()
{
	std::vector<char> vertShaderCode;
	std::vector<char> fragShaderCode;
	bool validShaders =
		   vk::readFile("../shaders/line_vert.spv", vertShaderCode)
		&& vk::readFile("../shaders/line_frag.spv", fragShaderCode);

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
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
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
	depthStencil.depthWriteEnable = VK_FALSE;
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

void ozz::sample::vk::LineRenderState::createUniformBuffer()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	createBuffer(renderContext->physicalDevice, renderContext->device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformStagingBuffer, uniformStagingBufferMemory);
	createBuffer(renderContext->physicalDevice, renderContext->device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, uniformBuffer, uniformBufferMemory);
	
	setDirty(true);
}

void ozz::sample::vk::LineRenderState::createDescriptorPool()
{
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

void ozz::sample::vk::LineRenderState::createVertexBuffer(const std::vector<Vertex>& vertices)
{
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

void ozz::sample::vk::LineRenderState::createDescriptorSet()
{
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

	std::array<VkWriteDescriptorSet, 1> descriptorWrites = {};

	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = descriptorSet;
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pBufferInfo = &bufferInfo;

	vkUpdateDescriptorSets(renderContext->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	setDirty(true);
}

void ozz::sample::vk::LineRenderState::updateVertexBuffer(const std::vector<Vertex>& vertices)
{
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

bool ozz::sample::vk::LineRenderState::updateVertexBufferObject(const VertexBufferObject& vbo)
{
	CHECK_AND_REPORT(vbo.vertices.size(), "Vertex buffer needs to contain valid data");
	if (vbo.vertices.size() != numOfVertices) {
		createVertexBuffer(vbo.vertices);
		return true;
	}
	else {
		updateVertexBuffer(vbo.vertices);
		return false;
	}
}

bool ozz::sample::vk::LineRenderState::updateUniformBufferObject(const UniformBufferObject& ubo)
{
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

void ozz::sample::vk::LineRenderState::setDirty(bool bDirty)
{
	dirty = bDirty;
}

ozz::sample::vk::LineRenderState::LineRenderState()
	: numOfVertices(0), dirty(false)
{

}

ozz::sample::vk::LineRenderState::~LineRenderState()
{

}

bool ozz::sample::vk::LineRenderState::onInitResources(internal::ContextVulkan* context)
{
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

void ozz::sample::vk::LineRenderState::onReleaseResources()
{
	RenderState::onReleaseResources();
}

bool ozz::sample::vk::LineRenderState::onRegisterRenderPass(size_t commandIndex)
{
	VkCommandBuffer commandBuffer = renderContext->commandBuffers[commandIndex];

	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, offsets);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
	vkCmdDraw(commandBuffer, numOfVertices, 1, 0, 0);

	return true;
}

void ozz::sample::vk::LineRenderState::onRenderPassesComplete()
{
	setDirty(false);
}

bool ozz::sample::vk::LineRenderState::onSwapChainChange()
{
	createGraphicsPipeline();
	return true;
}

bool ozz::sample::vk::LineRenderState::isDirty()
{
	return dirty;
}

bool ozz::sample::vk::LineRenderState::update(const UpdateData& updateData, uint32_t updateFlags)
{
	bool b_input_assambly_modified = false;
	bool b_recreate_descriptor_set = false;

	// Vertex buffer
	if (updateFlags & (UDF_VERTEX_BUFFER)) {
		b_input_assambly_modified |= updateVertexBufferObject(updateData.vbo);
	}

	// Uniform buffer
	if (updateFlags & (UDF_UNIFORM_BUFFER)) {
		b_recreate_descriptor_set |= updateUniformBufferObject(updateData.ubo);
	}

	if (b_recreate_descriptor_set || descriptorSet == VK_NULL_HANDLE) {
		createDescriptorSet();
	}

	return true;
}
