#pragma once
#include "Mesh.h"

class PrimitiveMesh :
	public Mesh
{
public:

	PrimitiveMesh(std::vector<Vertex> verticies, std::vector<uint32_t> indicies, std::vector<VkCommandBuffer>& commandBuffers, VkDevice& logicalDevice, std::vector<VkImage>& swapChainImages, VkExtent2D& swapChainExtent, VkPhysicalDevice& physicalDevice, VkCommandPool& commandPool, VkQueue& graphicsQueue);

	~PrimitiveMesh();

	virtual void updateUniformBuffer(uint32_t currentImage);


private:
		
	/*
	*****************
	Methods
	*****************
	*/
	virtual void createDescriptorSetLayout();

	virtual void createDescriptorPool();

	virtual void createDescriptorSet();

	virtual void createVertexBuffer();
	
	virtual void createIndexBuffer();

	virtual void createUniformBuffers();
};


