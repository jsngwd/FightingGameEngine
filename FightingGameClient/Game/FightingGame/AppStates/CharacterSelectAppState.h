#pragma once
#include "AppState.h"

#include <vector>

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include "Events.h"
#include <Scene/Components/Behavior.h>
#include <Scene/Components/Transform.h>

class InputHandler;
class Entity;
class Scene;
class ResourceManager;
class FighterFactory;
class BaseCamera;
class DebugDrawManager;
class CharacterSelectCameraBehavior;
namespace UI
{
	class UIInterface;
}

class CharacterSelectAppState :
	public AppState
{
public:
	CharacterSelectAppState(std::vector<std::string> fighterFiles, InputHandler* leftSideInputs, InputHandler* rightSideInputs, DebugDrawManager* debugDrawManager, class GameBase* gameBase);
	virtual void enterState();
	virtual AppState* update(float deltaTime);
	std::vector<Entity*> getCurrentlySelectedFighters();
private:
	enum SelectionState {
		idle = 0,
		entering,
		selected
	};

	void drawFighterSelectGrid();
	Entity* loadFighter(const std::string& fighterFile);
	void updateCursor(int fighterIndex);	
	void checkIfFighterCursorMoved(int fighterIndex);
	void updateFighter(int fighterIndex, float deltaTime);
	void fighterEntering(int fighterIndex, float deltaTime);
	void fighterIdle(int fighterIndex, float deltaTime);
	void fighterSelected(int fighterIndex, float deltaTime);
	void flipFighter(Entity* entity);
	void transitionState(int fighterIndex, SelectionState state);
	void setAnimation(int fighterIndex, std::string animation);
	bool checkFighterSelected(int fighterIndex);
	void setFinalFighterPos(int fighterIndex);
	void setupInput();

private:
	void initalizeCamera(GameBase* gameBas);
#define NUM_FIGHTERS 2
#define LEFT_SIDE 0
#define RIGHT_SIDE 1
	int MAX_GRID_COLS = 4;
	int cursorPos_[NUM_FIGHTERS];
	int prevCursorPos_[NUM_FIGHTERS];
	std::string selectedFighters_[NUM_FIGHTERS];
	InputHandler* inputs_[NUM_FIGHTERS];
	std::vector<std::string> fighterFiles_;
	std::vector<Entity*> fighters_[NUM_FIGHTERS];
	bool bothFighterSelected_ = false;

	Scene* scene_;
	ResourceManager* resourceManager_;
	UI::UIInterface* ui_;
	DebugDrawManager* debugManager_;
	Entity* camera_;
	
	const std::vector<std::string> HorizontalInputs =
	{
		"Horizontal",
		"Horizontal_P2"
	};

	const std::vector<std::string> VerticalInputs =
	{
		"Vertical",
		"Vertical_P2"
	};

	class Input* input_;


	SelectionState fighterState_[NUM_FIGHTERS];

	struct EnteringStateData
	{
		float currTime = 0;
		std::vector<std::pair<glm::vec3, float>> leftFighterPickedTranslation = {
			{{-3.0f, -2.0f, 6.0f}, 0.0f},
			{{-5.0f, -2.0f, 2.0f}, 750.0f}
		};
		std::vector<std::pair<glm::vec3, float>> rightFighterPickedTranslation = 
		{
			{{-3.0f, -2.0f, -6.0f}, 0.0f},
			{{-5.0f, -2.0f, -2.0f}, 1000.0f}
		};
	};

	struct SelectedStateData 
	{
		uint32_t currentFrame = 0;
	};
	
	EnteringStateData enteringStateData_[NUM_FIGHTERS];
	SelectedStateData selectedStateData_[NUM_FIGHTERS];

	class CharacterSelectCameraBehavior : public BehaviorImplementationBase 
	{
	public:
		CharacterSelectCameraBehavior(Entity* entity, CharacterSelectAppState* currentState) :
			characterSelectAppState_(currentState),
			BehaviorImplementationBase(entity)
		{};
		~CharacterSelectCameraBehavior() = default;
		
	private:
		void update()
		{
			glm::vec3 midPoint = calculateMidPoint();
			midPoint.y += 2.0f;
			Transform& transform = entity_->getComponent<Transform>();
			transform.lookAt(midPoint - transform.position_);
		}

		glm::vec3 calculateMidPoint()
		{
			std::vector<Entity*> fighters = characterSelectAppState_->getCurrentlySelectedFighters();
			std::vector<glm::vec3> positions;
			for (Entity* entity : fighters)
			{
				positions.push_back(entity->getComponent<Transform>().position_);
			}
			glm::vec3 midPoint = positions[0] + positions[1];
			midPoint *= 0.5f;
			return(midPoint);
		}

	private:
		CharacterSelectAppState* characterSelectAppState_;
	};
};


