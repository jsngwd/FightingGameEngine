#pragma once

#include <iostream>
#include "../Application.h"
#include "Hitbox.h"
#include "AnimationDataFileExporter.h"

class FighterCreatorTool : public Application
{
public:
	FighterCreatorTool();
	~FighterCreatorTool();
	void onUpdate(float deltaTime);
	void onStartup();
public:

private:
	void DrawMenuBar();
	bool LoadMesh(const std::string& filePath, std::string& error);
	Entity* loadEntity(const std::string& filepPath);
	void onEvent(Events::Event& event);
	bool lookAtCurrentEntity();
	void handleMouseScrolled(Events::MouseScrolledEvent& e);
	void handleMousePressedEvent(Events::MousePressedEvent& e);
	void handleMouseReleasedEvent(Events::MouseReleasedEvent& e);
	void handleMouseMovedEvent(Events::MouseMoveEvent& e);
	void rotateMeshRight();
	void drawHitboxes();
	Hitbox* getHitboxById(unsigned int id);
	int createHitboxId();
	void interpolateHitboxPosition(unsigned int uniqueHitboxId, unsigned int startFrame, unsigned int endFrame);
	void exportCurrentAnimationData();
private:
	const float CAMERA_DISTANCE_FROM_FIGHTER_DELTA = 5.0f;
	const float CAMERA_DISTANCE_FROM_FIGHTER_THRESHOLD = 1.0f;
	const float CAMERA_ROTATION_SPEED = 0.05f;
	const float MOVE_HITBOX_SENSITIVITY = 0.009f;

	UI::UIInterface* ui_;
	std::vector<std::string> fileNames_;
	std::vector<std::string> animations_;
	BaseCamera* baseCamera_;
	Entity* currentEntity_ = nullptr;
	float cameraDistanceFromFighter_ = 10.0f;
	float rightSideRotation_ = 0.0f;
	bool mouseHeld_ = false;
	int currentObjectSelectedId_ = -1;//-1 reserved for no object selected
	int previouslySelectedHitbox_ = -1;
	glm::vec2 lastMousePos_;

	
	//std::vector<Hitbox> currentHitboxes;

	struct {
		std::vector<Hitbox>& getCurrentHitboxData()
		{
			return hitboxData[currentFrame];
		}

		std::vector<Hitbox>& getPreviousFrameHitboxData()
		{
			return hitboxData[currentFrame - 1];
		}

		int getTotalAttackFrames()
		{
			return startup + active + recovery;
		}

		AnimationClip* animation;
		int currentFrame = 0;
		std::vector<std::vector<Hitbox>> hitboxData;//hitbox data for each frame in the animation
		InputKey input;
		int startup = 1;
		int active = 0;
		int recovery = 0;

	}CurrentAnimationData_;
};
