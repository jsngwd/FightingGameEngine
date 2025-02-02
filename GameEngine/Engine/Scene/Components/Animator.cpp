#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_interpolation.hpp>
#include <glm/gtx/quaternion.hpp>
#include "Animator.h"
#include "../../ResourceManager.h"
#include "../../libs/imgui/imgui.h"
#include "assimp/scene.h"
#include <stb_image.h>
#include "../../../Engine/Renderer/Asset/AssetInstance.h"

extern const int MAX_BONES;

Animator::Animator(std::vector<AnimationClip> animations, int boneStructureIndex) :
	animations_(animations), boneStructureIndex_(boneStructureIndex), resourceManager_(ResourceManager::getSingleton())
{
	globalInverseTransform_ = resourceManager_.boneStructures_[boneStructureIndex_].globalInverseTransform_;
;
}

//CONSTRUCTOR for testing interpolation
Animator::Animator(std::vector<AnimationClip> animations ):
	animations_(animations), resourceManager_(ResourceManager::getSingleton())
{
}

Animator::Animator(const Animator& other) :
	currentAnimation_(other.currentAnimation_),
	animations_(other.animations_),
	globalInverseTransform_(other.globalInverseTransform_),
	localTime_(other.localTime_),
	boneTransforms_(other.boneTransforms_),
	resourceManager_(other.resourceManager_)
{
	std::cout << "Animator: Copy constructor" << std::endl;
	boneStructureIndex_ = other.boneStructureIndex_;
}

Animator::Animator(Animator&& other) :
	currentAnimation_(other.currentAnimation_),
	boneTransforms_(other.boneTransforms_),
	animations_(other.animations_),
	globalInverseTransform_(other.globalInverseTransform_),
	localTime_(other.localTime_),
	resourceManager_(other.resourceManager_)
{
	std::cout << "Animator: Move constructor" << std::endl;
	boneStructureIndex_ = other.boneStructureIndex_;
}

void Animator::setAnimation(int animationIndex)
{
	dontUpdate_ = false;
	int size = animations_.size();
	if (animationIndex < size && animationIndex >= -1)
	{
		currentAnimation_ = animationIndex;
		lastIndex_ = 0;
		//startTime_ = std::clock();
		startTime_ = 0;
		localTime_ = 0;
	}
	else
	{
		std::cout << "WARNING: animationIndex: " << animationIndex << " not found" << std::endl;
	}
}

void Animator::setAnimation(std::string name)
{
	dontUpdate_ = false;
	for (int i = 0; i < animations_.size(); i++)
	{
		if (animations_[i].name_ == name)
		{
			currentAnimation_ = i;
			lastIndex_ = 0;
			startTime_ = 0;
			localTime_ = 0;
			currentFrameIndex_ = 0;
			return;
		}
	}
	_ASSERT_EXPR(false, ("Error: animation not found"));
}

void Animator::setAnimationTime(float timeInSeconds)
{
	if (currentAnimation_ != -1)
	{
		const AnimationClip& clip = animations_[currentAnimation_];
		assert(timeInSeconds <= clip.durationInSeconds_);
		localTime_ = timeInSeconds;
		timeSet_ = true;
	}
	else
	{
		std::cout << "Error tried to setAnimationTime when there is no animation currently set" << std::endl;
	}
}

void Animator::getAnimationPoseByFrame(const AnimationClip& clip, unsigned int frameNumber, AssetInstance* assetInstance)
{
	if (frameNumber > clip.frameCount_)
	{
		std::cout << "Error: frame number out of bounds" << std::endl;
		return;
	}

	const BoneStructure* boneStructure = assetInstance->getSkeleton();

	globalTransforms_.clear();

	glm::mat4 globalTransform = boneStructure->boneInfo_[0].invBindPose_;
	glm::mat4 transform;
	std::vector<glm::mat4> finalTransform;

	if (boneStructure->boneInfo_[0].animated_)
	{
		globalTransform = 
			glm::translate(glm::mat4(1.0f), clip.positions_[1][frameNumber]) *
			glm::toMat4(clip.rotations_[1][frameNumber]) * 
			glm::scale(glm::mat4(1.0f), glm::vec3(clip.scale_[1][frameNumber]));
	}

	globalTransforms_.push_back(globalTransform);
	finalTransform.push_back(globalInverseTransform_ * globalTransform * boneStructure->boneInfo_[0].offset_);

	for (int i = 1; i < boneStructure->boneInfo_.size() - 1; i++)
	{
		glm::mat4 transform = boneStructure->boneInfo_[i].invBindPose_;
		if (boneStructure->boneInfo_[i].animated_)
		{
			transform =
				glm::translate(glm::mat4(1.0f), clip.positions_[i][frameNumber]) *
				glm::toMat4(clip.rotations_[i][frameNumber]) *
				glm::scale(glm::mat4(1.0f), glm::vec3(clip.scale_[i][frameNumber]));
		}

		glm::mat4 parent(1.0f);
		int parentIndex = boneStructure->boneInfo_[i].parent_;
		if (parentIndex < finalTransform.size())
		{
			parent = globalTransforms_[parentIndex];
		}
		else
		{
			parent = glm::mat4(1.0f);
			std::cout << "ERROR: parent joint not found in final transform matrix" << std::endl;
		}

		globalTransform = parent * transform;
		globalTransforms_.push_back(globalTransform);

		finalTransform.push_back(globalInverseTransform_ * globalTransform * boneStructure->boneInfo_[i].offset_);
	}

	assetInstance->setPose(finalTransform);
	dontUpdate_ = true;
}

Animator::~Animator()
{
	//delete boneStructure_;
}

void outputMatrixToFile(std::vector<aiMatrix4x4> boneTransforms)
{
	std::string output;
	for (uint32_t i = 0; i < boneTransforms.size(); i++)
	{
		for (int j = 0; j < 4; j++)
		{
			for (int k = 0; k < 4; k++)
			{
				output += std::to_string((boneTransforms[i])[j][k]);
			}
		}
	}
	std::cout << output << std::endl;
}

void Animator::setPose(const std::vector<glm::mat4>& pose, Renderable& renderable)
{
	uint32_t bonesToSet;
	for (bonesToSet = 0; bonesToSet < pose.size(); bonesToSet++)
	{
		renderable.ubo_.bones[bonesToSet] =  pose[bonesToSet];
	}
	for (size_t i = bonesToSet; i < MAX_BONES; i++)
	{
		renderable.ubo_.bones[i] = glm::mat4(1.0f);
	}
}

void Animator::update(float deltaTime, AssetInstance* assetInstance)
{
	if (dontUpdate_)
	{
		return;
	}

	std::vector<glm::mat4> finalTransform;

	globalTransforms_.clear();

	const BoneStructure& boneStructure = resourceManager_.boneStructures_[boneStructureIndex_];

	glm::mat4 globalTransform = boneStructure.boneInfo_[0].invBindPose_;

	if (currentAnimation_ != -1)
	{
		const AnimationClip& clip = animations_[currentAnimation_];

		//was time set by setAnimationTime
		if (!timeSet_)
		{
			localTime_ += deltaTime * 0.001 * clip.playbackRate_;//in seconds
		}
		timeSet_ = false;

		if (clip.isLooping_)
		{
			localTime_ = fmod(((localTime_)), clip.durationInSeconds_);
		}
		else
		{
			localTime_ = std::clamp(((localTime_)), 0.0f, clip.durationInSeconds_);
		}

		for (unsigned int i = 0; i < clip.frameCount_-1; i++)
		{
			if (clip.times_[i] >= localTime_)
			{
				currentFrameIndex_ = i;
				break;
			}
		}

		if (boneStructure.boneInfo_[0].animated_)
			globalTransform = interpolateTransforms(1, clip, currentFrameIndex_);
	}

#define DEBUG 0
#if DEBUG == 1
	if (ImGui::Begin("Animator Debug"))
	{
		debugScrollingBuffer_.add( currentTime, localTime_ );

		ImPlot::SetNextPlotLimitsX(currentTime - 3000.0f, currentTime, ImGuiCond_Always);
		if (ImPlot::BeginPlot("AnimatorTiminig", "time", "LocalTime"))
		{
			ImPlot::PlotLine("localTime", debugScrollingBuffer_.bufferX.data(), debugScrollingBuffer_.bufferY.data(), debugScrollingBuffer_.bufferX.size());
			ImPlot::EndPlot();
		}
		ImGui::End();
	}
#endif

	globalTransforms_.push_back(globalTransform);
	
	finalTransform.push_back(globalInverseTransform_ * globalTransform * boneStructure.boneInfo_[0].offset_); //model -> joint space

	glm::mat4 parent(1.0f);
	//get global positions of each joint
	for (int i = 1; i < boneStructure.boneInfo_.size()-1; i++)
	{
		glm::mat4 currentPose(boneStructure.boneInfo_[i].invBindPose_ );
		if (currentAnimation_ != -1 && boneStructure.boneInfo_[i].animated_)
			currentPose = interpolateTransforms(i, animations_[currentAnimation_], currentFrameIndex_);

		int parentIndex = boneStructure.boneInfo_[i].parent_;
		if (parentIndex < finalTransform.size())
		{
			parent = globalTransforms_[parentIndex];
		}
		else
		{
			parent = glm::mat4(1.0f);
			std::cout << "ERROR: parent joint not found in final transform matrix" << std::endl;
		}

		globalTransform = parent * currentPose;

		globalTransforms_.push_back(globalTransform);

		finalTransform.push_back(globalInverseTransform_ * globalTransform * boneStructure.boneInfo_[i].offset_);
	}

	assetInstance->setPose(finalTransform);
}

glm::mat4 Animator::interpolateTransforms(int jointIndex, const AnimationClip& clip, int frameIndex)
{
	glm::mat4 pos;
	glm::mat4 rotation;
	glm::mat4 scale;

	glm::vec3 startPos = clip.positions_[jointIndex][frameIndex];
	glm::vec3 endPos = clip.positions_[jointIndex][frameIndex + 1];

	float startScale = clip.scale_[jointIndex][frameIndex];
	float endScale = clip.scale_[jointIndex][frameIndex+1];

	glm::quat startRot = clip.rotations_[jointIndex][frameIndex];
	glm::quat endRot = clip.rotations_[jointIndex][frameIndex+1];
	
	float normalizedTime = (localTime_ - clip.times_[frameIndex]) / (clip.times_[frameIndex] - clip.times_[frameIndex+1]);

	glm::mat4 output(1.0f);
	float x = startPos.x + normalizedTime*(endPos.x - startPos.x);
	float y = startPos.y + normalizedTime*(endPos.y - startPos.y);
	float z = startPos.z + normalizedTime*(endPos.z - startPos.z);
	output = glm::translate(output,{x, y, z});

	glm::quat rotationQuat = glm::normalize(glm::slerp(startRot, endRot, normalizedTime));
	output *= glm::toMat4(rotationQuat);

	float s = startScale + normalizedTime*(endScale - startScale);
	output = glm::scale(output, { s, s, s });

	return output;
}

int Animator::findAnimationIndexByName(const std::string& animationName)
{
	for (int i = 0; i < animations_.size(); i++)
	{
		if (animations_[i].name_ == animationName)
		{
			return i;
		}
	}
	return -1;
}

AnimationClip* Animator::getAnimationClipByName(const std::string& animationName)
{
	AnimationClip* clip = nullptr;
	int index = findAnimationIndexByName(animationName);
	if (index != -1)
	{
		clip = &animations_[index];
	}
	return clip;
}
