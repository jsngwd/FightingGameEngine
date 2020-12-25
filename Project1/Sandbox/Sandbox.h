#pragma once
#include "../Application.h"
#include "../Engine/Scene/Scene.h"

class Sandbox : public Application
{
public:
	virtual ~Sandbox();
	virtual void onUpdate(float deltaTime);
	virtual void onStartup();
private:
	BaseCamera* baseCamera;

	void initScene();
	void onEvent(Events::Event& e);
	void handleMouseClick(Events::MousePressedEvent& e);
	void handleMouseMove(Events::MouseMoveEvent& e);
	void transformVel(entt::entity e, entt::registry& r, ...);
	Scene* scene_;
};

