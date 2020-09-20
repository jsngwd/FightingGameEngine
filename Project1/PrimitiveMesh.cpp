#include "PrimitiveMesh.h"
#include "BufferOperations.h"
#include <vector>
#include <iostream>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/*
[09/03/2020 Gwozdz]
what should be included in this class?
-- descriptor set layouts, descriptor pools, descripter sets
	Uniform buffers and samplers should be determined on a case by case basis for each derived mesh class.
	E.g: a mesh may want to also pass data through a uniform buffer into the fragment shader, in addition to 
	the usual vertex shader uniform buffer
	NOTE: need to decide whether a uniform buffer to the vertex shader will be an intrinsic property of a mesh.
	There may be a reason why uniforms/samplers may not be needed for some types of meshes.  Can't think of one right now

-- pipeline
	it may make sense to create a pipeline within a derived mesh object since there will be unique meshes that will
	need their own pipeline(E.g: skybox).  It will also be easier to set different shaders for each object.  Should add
	functionality that enables a meshes shader to be changed at run time.

[09/07/2020 Gwozdz]
--derived classes of Mesh will want to share a renderPass, frameBuffer, descriptorSetLayout, pipeline.  Ocasionally one or more
	instances within a group of dervied classes will want to change one of these.  Need to think of a good way to allow Mesh derived
	classes to share the componenets above while also having enough freedom to change a single instances components

solution: the above vulkan compoonenets can be encapsulated into a singleton mesh component held by the mesh class.  A derived meshes vulkan components can be determined
from a class derived from the singleton mesh class

[09/08/2020 Gwozdz]
--decided that the functionality above may be too hard for me to implement at the current time.
	Will instead focus on subset of problem.  Focus on implementing a way for shaders to be changed at runtime

-- will implement a static component called mesh_ShaderComponenet that will implement pipeline, descriptorPool, descriptorSetLayout, 
	and descriptorSet

-- Thought about creating pipeline object before mesh object is initalized and passing it in as an argument.  Don't think this would
	work well when being used in the actual game engine since more se
*/

//int totalMemory = 0;
//void* operator new(size_t bytes) {
//	totalMemory += bytes;
//	std::cout << "allocating " << totalMemory << "\n";
//	return malloc(bytes);
//}
//
//void operator delete(void* mem, size_t bytes) {
//	totalMemory -= bytes;
//	std::cout << "freeing " << totalMemory << "\n";
//	free(mem);
//}


/*
Constructor -
inputs:
	verticies: verticies of mesh

*/
PrimitiveMesh::PrimitiveMesh(std::vector<Vertex> verticies, 
	std::vector<uint32_t> indicies, 
	std::vector<VkCommandBuffer>& commandBuffers,
	VkDevice& logicalDevice, 
	std::vector<VkImage>& swapChainImages,  
	VkExtent2D& swapChainExtent, 
	VkPhysicalDevice& physicalDevice, 
	VkCommandPool& commandPool, 
	VkQueue& graphicsQueue) :
		Mesh(verticies, indicies, commandBuffers, logicalDevice, swapChainImages, swapChainExtent, physicalDevice, commandPool, graphicsQueue)
{
	createUniformBuffers();
	createDescriptorSetLayout();
	createDescriptorPool();
	createDescriptorSet();
	createRenderPass();
	createVertexBuffer();
	createIndexBuffer();
}

PrimitiveMesh::~PrimitiveMesh() {
	//index buffer
	vkDestroyBuffer(rm_logicalDevice, m_indexBuffer, nullptr);
	vkFreeMemory(rm_logicalDevice, m_indexBufferMemory, nullptr);
	//vertex buffer
	vkDestroyBuffer(rm_logicalDevice, m_vertexBuffer, nullptr);
	vkFreeMemory(rm_logicalDevice, m_vertexBufferMemory, nullptr);
	//renderpass
	vkDestroyRenderPass(rm_logicalDevice, m_renderPass, nullptr);
	//pipeline
	vkDestroyPipeline(rm_logicalDevice, m_pipeline, nullptr);
	vkDestroyPipelineLayout(rm_logicalDevice, m_pipelineLayout, nullptr);
	//uniform buffers
	for (size_t i = 0; i < rm_swapChainImages.size(); i++) {
		vkDestroyBuffer(rm_logicalDevice, m_uniformBuffers[i], nullptr);
		vkFreeMemory(rm_logicalDevice, m_uniformBuffersMemory[i], nullptr);
	}
	//descriptor pool/set
	vkDestroyDescriptorPool(rm_logicalDevice, m_descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(rm_logicalDevice, m_descriptorSetLayout, nullptr);
	std::cout << "deleteing PrimitiveMesh" << std::endl;
}


void PrimitiveMesh::createRenderPass() {
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = VK_FORMAT_B8G8R8A8_SRGB;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(rm_logicalDevice, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}


void PrimitiveMesh::updateUniformBuffer(uint32_t currentImage) {
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	m_ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	m_ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	m_ubo.proj = glm::perspective(glm::radians(45.0f), rm_swapChainExtent.width / (float)rm_swapChainExtent.height, 0.1f, 10.0f);
	m_ubo.proj[1][1] *= -1;

	void* data;
	vkMapMemory(rm_logicalDevice, m_uniformBuffersMemory[currentImage], 0, sizeof(m_ubo), 0, &data);
	memcpy(data, &m_ubo, sizeof(m_ubo));
	vkUnmapMemory(rm_logicalDevice, m_uniformBuffersMemory[currentImage]);
}

void PrimitiveMesh::createUniformBuffers() {
	VkDeviceSize bufferSize = sizeof(m_ubo);

	m_uniformBuffers.resize(rm_swapChainImages.size());
	m_uniformBuffersMemory.resize(rm_swapChainImages.size());

	for (size_t i = 0; i < rm_swapChainImages.size(); i++) {
		BufferOperations::createBuffer(rm_logicalDevice, rm_physicalDevice, rm_commandPool, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffers[i], m_uniformBuffersMemory[i]);
	}
}

void PrimitiveMesh::createDescriptorSetLayout() {
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorCount = 1; //number of elements in ubo array
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.pImmutableSamplers = nullptr;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	std::array<VkDescriptorSetLayoutBinding, 1> bindings = { uboLayoutBinding };

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(rm_logicalDevice, &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

void PrimitiveMesh::createDescriptorPool() {
	std::array<VkDescriptorPoolSize, 1> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(rm_swapChainImages.size());

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(rm_swapChainImages.size());

	if (vkCreateDescriptorPool(rm_logicalDevice, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void PrimitiveMesh::createDescriptorSet() {
	std::vector<VkDescriptorSetLayout> layouts(rm_swapChainImages.size(), m_descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(rm_swapChainImages.size());
	allocInfo.pSetLayouts = layouts.data();

	m_descriptorSets.resize(rm_swapChainImages.size());
	if (vkAllocateDescriptorSets(rm_logicalDevice, &allocInfo, m_descriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	for (size_t i = 0; i < rm_swapChainImages.size(); i++) {
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(m_ubo);

		std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = m_descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(rm_logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void PrimitiveMesh::createVertexBuffer() {
	VkDeviceSize bufferSize = sizeof(m_vertices[0]) * m_vertices.size();
	m_vertexBufferSize = bufferSize;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	BufferOperations::createBuffer(rm_logicalDevice, rm_physicalDevice, rm_commandPool, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(rm_logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, m_vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(rm_logicalDevice, stagingBufferMemory);

	BufferOperations::createBuffer(rm_logicalDevice, rm_physicalDevice, rm_commandPool, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertexBuffer, m_vertexBufferMemory);

	BufferOperations::copyBuffer(rm_logicalDevice, rm_commandPool, rm_graphicsQueue, stagingBuffer, m_vertexBuffer, bufferSize);

	vkDestroyBuffer(rm_logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(rm_logicalDevice, stagingBufferMemory, nullptr);
}

void PrimitiveMesh::createIndexBuffer() {
	VkDeviceSize bufferSize = sizeof(m_indicies[0]) * m_indicies.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	BufferOperations::createBuffer(rm_logicalDevice, rm_physicalDevice, rm_commandPool, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(rm_logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, m_indicies.data(), (size_t)bufferSize);
	vkUnmapMemory(rm_logicalDevice, stagingBufferMemory);

	BufferOperations::createBuffer(rm_logicalDevice, rm_physicalDevice, rm_commandPool, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_indexBuffer, m_indexBufferMemory);

	BufferOperations::copyBuffer(rm_logicalDevice, rm_commandPool, rm_graphicsQueue, stagingBuffer, m_indexBuffer, bufferSize);

	vkDestroyBuffer(rm_logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(rm_logicalDevice, stagingBufferMemory, nullptr);
}


void PrimitiveMesh::bindToCommandBuffers(std::vector<VkCommandBuffer>& commandBuffers, std::vector<VkFramebuffer>& frameBuffers) {
	for (size_t i = 0; i < rm_commandBuffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(rm_commandBuffers[i], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		std::array<VkClearValue, 1> clearValues{};
		VkDeviceSize offsets[1] = { 0 };

		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_renderPass;
		renderPassInfo.framebuffer = frameBuffers[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = rm_swapChainExtent;

		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

			vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &m_vertexBuffer, offsets);

			vkCmdBindIndexBuffer(commandBuffers[i], m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSets[i], 0, nullptr);

			vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(m_indicies.size()), 1, 0, 0, 0);

		vkCmdEndRenderPass(commandBuffers[i]);

		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}