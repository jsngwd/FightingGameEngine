#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include "../libs/VulkanMemoryAllocator/vk_mem_alloc.h"

#include "RenderSubsystemInterface.h"
#include "VkTypes.h"
#include "PipelineBuilder.h"

#define MAX_VERTEX_BUFFER_SIZE 1000


struct ModelReturnVals;

class SkyBoxRenderSubsystem : public RenderSubsystemInterface
{
public:
	SkyBoxRenderSubsystem(VkDevice& logicalDevice, VkRenderPass& renderPass, VmaAllocator& allocator, VkDescriptorPool& descriptorPool, VkRenderer& renderer);
	~SkyBoxRenderSubsystem();

	void renderFrame(VkCommandBuffer commandBuffer, uint32_t currentSwapChainIndex);
	void onResize(VkCommandBuffer commandBuffer);
	bool setSkyboxTexture(const std::string& path);//true if file exists and skybox loading is succesful

private:
	void createDescriptors();
	void createPipeline();
	void createUniformBuffers();
	void loadCube(ModelReturnVals* modelVals);
	bool createTextureResources(int baseHeight, int baseWidth, int numChannels, std::vector<unsigned char>& totalPixels);
	int getStringIndex(const std::string& filename);
	void updateUniformBuffer();

private:
	struct 
	{
		alignas(16)glm::mat4 model;
		alignas(16)glm::mat4 view;
		alignas(16)glm::mat4 proj;
	} skyboxBufferObject_;

	std::string skyboxFaceFileNames_[6] =
	{
		"right",
		"left",
		"top",
		"bottom",
		"front",
		"back"
	};

	const std::string VERT_SHADER_ = "C:\\Users\\jsngw\\source\\repos\\FightingGame\\GameEngine\\shaders\\skybox.vert.spv";
	const std::string FRAG_SHADER_ = "C:\\Users\\jsngw\\source\\repos\\FightingGame\\GameEngine\\shaders\\skybox.frag.spv";
	const std::string CUBE_MODEL_PATH_ = "C:\\Users\\jsngw\\source\\repos\\FightingGame\\FightingGameClient\\Models\\cube.obj";
	bool skyboxSet_ = false;

	class Scene* scene_;
	int numSkyboxModelIndicies_;
	TextureResources skyboxTexture_;
	VkDescriptorSetLayout skyboxDescriptorSetLayout_;
	VkDescriptorSet skyboxDescriptorSet_;
	VulkanBuffer vertexBuffer_;
	VulkanBuffer indexBuffer_;
	VulkanBuffer uniformBuffer_;
	PipelineResources* skyboxPipeline_;
};
