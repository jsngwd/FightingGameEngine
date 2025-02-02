#include "Transform.h"

#include "../../libs/imgui/imgui.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../../DebugDrawManager.h"
#include "../../EngineSettings.h"
#include "../../Renderer/Asset/AssetInstance.h"

glm::vec3 Transform::worldLeft =	{ 1.0f, 0.0f, 0.0f };
glm::vec3 Transform::worldUp =		{ 0.0f, 1.0f, 0.0f };
glm::vec3 Transform::worldForward = { 0.0f, 0.0f, 1.0f };

Transform::Transform(float x, float y, float z) : 
	position_({x, y, z}),
	oldPos_({x-1, y-1, z-1}),
	rotation_({1.0f, 0.0f, 0.0f, 0.0f}),
	scale_({1.0f, 1.0f, 1.0f})
{}

Transform::Transform(glm::vec3 pos) :
	position_(pos),
	oldPos_(-pos),
	rotation_({1.0f, 0.0f, 0.0f, 0.0f}),
	scale_({1.0f, 1.0f, 1.0f})
{}

void Transform::drawDebugGui()
{
	if (drawDebugGui_)
	{
		UI::UIInterface& ui = UI::UIInterface::getSingleton();
		EngineSettings& engineSettings = EngineSettings::getSingleton();
		float windowWidth = engineSettings.windowWidth;
		float windowHeight = engineSettings.windowHeight;

		ui.beginWindow("Transform", windowWidth / 5, windowHeight / 5, { windowWidth/2, windowHeight/2 }, nullptr, false);
		std::string posStr = "Position: ";
		posStr += "x ";
		posStr += position_.x;
		posStr += "y ";
		posStr += position_.y;
		posStr += "z ";
		posStr += position_.z;
		ui.addText(posStr);
		ui.EndWindow();
	}
}

void Transform::setScale(float scale)
{
	scale_.x = scale;
	scale_.y = scale;
	scale_.z = scale;
}

void Transform::applyTransformToMesh(Renderable& mesh)
{
	finalTransform_ = calculateTransform();
	mesh.ubo_.model = finalTransform_;
	return;
}

void Transform::applyTransformToMesh(AssetInstance* assetInstance)
{
	finalTransform_ = calculateTransform();
	assetInstance->setModelMatrix(finalTransform_);
}

void Transform::drawDebug()
{
	DebugDrawManager& debugDrawManager = DebugDrawManager::getSingleton();
	debugDrawManager.drawLine(position_, position_ + forward(), { 255.0f, 0.0f, 255 });
	debugDrawManager.drawLine(position_, position_ + left(), { 0.0f, 255.0f, 0.0f});
	debugDrawManager.drawLine(position_, position_ + up(), { 0.0f, 0.0f, 255 });
}

glm::mat4 Transform::calculateTransform()
{
	if (calculateTransform_)
	{
		//if (position_ != oldPos_ || scale_ != oldScale_ || rotation_ != oldRot_)//only calculate new finalTransformation_ if anything has changed
		//{
			glm::mat4 scale = glm::scale(glm::mat4(1.0f), scale_);
			glm::mat4 rot = glm::toMat4(glm::normalize(rotation_));
			glm::mat4 trans = glm::translate(glm::mat4(1.0f), position_);

			//oldPos_ = position_;
			//oldScale_ = scale_;
			//oldRot_ = rotation_;

			finalTransform_ = trans * rot * scale;
		//}
	}
	return finalTransform_;
}

glm::vec3 Transform::left() const
{
	glm::vec3 leftVec{};
	for (int i = 0; i < 3; i++)
	{
		leftVec[i] = finalTransform_[0][i];
	}
	leftVec = glm::normalize(leftVec);
	return leftVec;
}

glm::vec3 Transform::up() const
{
	glm::vec3 upVec{};
	for (int i = 0; i < 3; i++)
	{
		upVec[i] = finalTransform_[1][i];
	}
	upVec = glm::normalize(upVec);
	return upVec;
}

glm::vec3 Transform::forward() const
{
	glm::vec3 forwardVec{};
	for (int i = 0; i < 3; i++)
	{
		forwardVec[i] = finalTransform_[2][i];
	}
	forwardVec = glm::normalize(forwardVec);
	return forwardVec;
}

void Transform::lookAt(glm::vec3 directionToLook)
{
	//https://stackoverflow.com/a/49824672
	// 2 catches to glm::quatLookAT
	//direction needs to be normalized
	//direction cant be parallel to up
	const float SIMILARITY_THRESHOLD = 0.0001f;
	glm::vec3 directionNormalized = glm::normalize(directionToLook);
	glm::vec3 upDir = up();
	float distance = glm::distance(directionNormalized, upDir);
	if (distance < SIMILARITY_THRESHOLD)//upDir and direction to look are parallel generate a new updir
	{
		std::cout << "ERROR: lookAT updir matched desired lookdirection" << std::endl;
		upDir = glm::cross(directionNormalized, left());
	}
	rotation_ = glm::quatLookAtLH(directionToLook, worldUp);
}

void Transform::rotateAround(float angleToRotateInDegrees, glm::vec3 axisToRotateAround)
{
	//glm::quat newRotation = glm::quat(angleToRotateInDegrees, axisToRotateAround);//axis angle constructor
	glm::quat applyRot = glm::angleAxis(glm::radians(angleToRotateInDegrees), axisToRotateAround);
	rotation_ *= applyRot;
	//rotation_ = glm::quat(glm::rotate(rotation_, glm::radians(angleToRotateInDegrees), axisToRotateAround));
	//rotation_ *= newRotation;
	calculateTransform();
}

void Transform::moveTowards(glm::vec3 finalPos, float maxMoveMag)
{
	if (finalPos == position_)//make sure that finalPos and position_ are very close to eachother.  glm::length will return nan if thats the case.
	{
		return;
	}
	glm::vec3 dir = finalPos - position_;
	float reqDist = glm::length(dir);
	dir = glm::normalize(dir);
	float mag;
	if (reqDist < maxMoveMag)
	{
		mag = reqDist;
	}
	else
	{
		mag = maxMoveMag;
	}
	position_ += dir * mag;
}

void Transform::rotateAround(glm::vec3 rotationInRadians)
{
	rotation_ *= glm::quat(glm::rotate(rotationInRadians.x, worldLeft));
	rotation_ *= glm::quat(glm::rotate(rotationInRadians.y, worldUp));
	rotation_ *= glm::quat(glm::rotate(rotationInRadians.z, worldForward));
}

float Transform::checkSide(glm::vec2 middle, glm::vec2 checkSide)
{
	glm::vec2 middleRotated90 = { middle.y, -middle.x };
	return glm::dot(middleRotated90, checkSide);
}

glm::mat4 Transform::calculateTransformNoScale()
{
	glm::mat4 localToWorldNoScale;
	glm::mat4 scale = glm::mat4(1.0f);
	glm::mat4 rot = glm::toMat4(glm::normalize(rotation_));
	glm::mat4 trans = glm::translate(glm::mat4(1.0f), position_);

	localToWorldNoScale = trans * rot * scale;
	return localToWorldNoScale;
}
