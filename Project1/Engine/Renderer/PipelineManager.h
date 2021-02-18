#pragma once
#include <vector>
#include <array>
#include "GraphicsPipeline.h"
#include "../Singleton.h"

//enum PipelineTypes 
//{
//	BASIC_PIPELINE,
//	LINE_PIPELINE,
//	DEBUG_PIPELINE,
//	ANIMATION_PIPELINE,
//	NUM_PIPELINE_TYPES = 4
//};

//class PipelineManager : Singleton<PipelineManager>
//{
//public:
//	PipelineManager();
//	~PipelineManager();
//	void init();
//	GraphicsPipeline* createOrGetPipeline(PipelineTypes type, VkDescriptorSetLayout& layout);
//	static PipelineManager& getSingleton();
//	static PipelineManager* getSingletonPtr();
//private:
//	std::vector<uint32_t> pipelinesToDelete;
//	std::array<GraphicsPipeline*, NUM_PIPELINE_TYPES> graphicsPipelines;
//	std::array<VkDescriptorSetLayout, NUM_PIPELINE_TYPES> descriptorLayouts;
//	std::array<VkDescriptorSet, NUM_PIPELINE_TYPES> descriptorSets;
//};
//
