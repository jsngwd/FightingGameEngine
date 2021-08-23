#include "CameraController.h"
#include <iostream>

CameraController::CameraController(BaseCamera& camera) :
	camera_(camera)
{}

void CameraController::handleKeyPressed(Events::KeyPressedEvent& e)
{
	if (controllable_)
	{
		if (e.KeyCode == 87)
		{
			forwardHeld_ = true;
		}
		else if (e.KeyCode == 83)
		{
			backwardHeld_ = true;
		}
		else if (e.KeyCode == 65)//a
		{
			strafeLeftHeld_ = true;
		}
		else if (e.KeyCode == 68)
		{
			strafeRightHeld_ = true;
		}
	}
}

void CameraController::handleKeyReleased(Events::KeyReleasedEvent& e)
{
	if (controllable_)
	{
		if (e.KeyCode == 87)
		{
			forwardHeld_ = false;
		}
		else if (e.KeyCode == 83)
		{
			backwardHeld_ = false;
		}
		else if (e.KeyCode == 65)//a
		{
			strafeLeftHeld_ = false;
		}
		else if (e.KeyCode == 68)
		{
			strafeRightHeld_ = false;
		}
	}
}

void CameraController::handleMouseMoved(Events::MouseMoveEvent& e)
{
	if (controllable_)
	{
		camera_.updateMouse({ e.mouseX, e.mouseY });
	}
}

void CameraController::onUpdate(float deltaTime)
{
	if (forwardHeld_) 
		camera_.moveForward(deltaTime);
	if (backwardHeld_) 
		camera_.moveBackward(deltaTime);
	if (strafeLeftHeld_) 
		camera_.strafeLeft(deltaTime);
	if (strafeRightHeld_) 
		camera_.strafeRight(deltaTime);
}
