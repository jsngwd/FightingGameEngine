#pragma once
#include "Scene/Entity.h"
#include "Scene/Scene.h"
#include "../InputHandler.h"
#include "EngineSettings.h"
#include "Hitbox.h"
#include "Attack.h"
#include "../FighterStates/BaseFighterState.h"
#include "Scene/Components/Behavior.h"
#include "../FighterSandbox/AttackBase.h"
#include "../FighterSandbox/AttackTypes.h"

#include "../FighterStates/IdleFighterState.h"
#include "../FighterStates/AttackingFighterState.h"
#include "../FighterStates/WalkingFighterState.h"
#include "../FighterStates/HitFighterState.h"
#include "../FighterStates/JumpingFighterState.h"
#include "../FighterStates/CrouchingFighterState.h"
#include "../FighterStates/BlockingFighterState.h"
#include "../FighterStates/JumpingAttackFighterState.h"

class DebugDrawManager;

struct AttackInput
{
	AttackInput(std::vector<uint8_t> attackInputIn, int attackIndex) :
		attackInput(attackInputIn)
	{};

	std::vector<uint8_t> attackInput;
	int attackIndex = 0;
	double dtBetweenAttacks; // IN MILLISECONDS
	double lastCheckTime = 0;
	float currentInput = 0;
};

//enum FighterState
//{
//	idle,
//	walkingForward,
//	walkingBackward,
//	jumping,
//	attacking,
//	crouching,
//	hitstun,
//	blocking
//};

enum FighterSide
{
	left = 0,
	right = 1
};

struct FighterStateData 
{
	std::string animationName;
	std::vector<FrameInfo> frameData;
};

struct AttackResources
{
	std::vector<AttackBase*> newAttacks_;
	std::vector<AttackInput> inputs_;
};


class Fighter : public BehaviorImplementationBase
{
public:

	Fighter(Entity* entity, InputHandler& inputHandler, FighterStateData idleStateData, FighterStateData walkingStateData, FighterStateData crouchStatedata, FighterStateData jumpStateData, FighterStateData hitStateData, FighterStateData blockStateData, AttackResources attacks, BoxCollider basePushBox, FighterSide side = right);
	~Fighter();

	static Fighter* getFighterComp(Entity* entity);

	void setPosition(glm::vec3 pos);
	void flipSide();
	bool onAttackHit(HitEffect hitEffect);
	glm::vec3 getPosition() const;
	void setXSpeed(float speedX);
	void setYSpeed(float speedY);
	void takeDamage(int damage);
	void handleWallCollision(bool collidedWithLeftWall);// called by arenaWallBehavior when this fighter collides with a wall. argument is true when the fighter hits the left, false when they hit the right
	void handleFloorCollision();// called by arenaFloorBehavior when this fighter collides with the floor

	//Inherited from BehaviorImplementationBase
	virtual void onCollision(Entity* otherEnt, class BoxCollider* thisCollider, class BoxCollider* otherCollider);
	virtual void whileColliding(Entity* otherEnt, class BoxCollider* thisCollider, class BoxCollider* otherCollider);
	virtual void onExitCollision(Entity* otherEnt, class BoxCollider* thisCollider, class BoxCollider* otherCollider);
public:
	float baseSpeed_ = 5.0f;
	float jumpSpeed_ = 25.0f;
	float airDashSpeed_ = 15.0f;

	glm::vec3 velocityWorldSpace_;
	glm::vec3 oldPos_ = glm::vec3(0.0f);

	BoxCollider basePushBox_;

	BaseFighterState* lastState_;
	AttackBase* currentAttack_ = nullptr;
	FighterSide side_;
	int maxHealth_ = 100;
	bool flipSide_ = false;
	bool applyGravity_ = false;
	bool controllable_ = false;
	bool collidingWithWall = false;
	bool colldingWithFighter = false;

	IdleFighterState* idleFighterState_ = nullptr;
	WalkingFighterState* walkingFighterState_ = nullptr;
	JumpingFighterState* jumpingFighterState_ = nullptr;
	JumpingAttackFighterState* jumpingAttackFighterState_ = nullptr;
	AttackingFighterState* attackingFighterState_ = nullptr;
	CrouchingFighterState* crouchingFighterState_ = nullptr;
	HitFighterState* hitFighterState_ = nullptr;
	BlockingFighterState* blockedFighterState_ = nullptr;

	std::map<int, std::vector<int>>  cancelAttackMap_;
	InputHandler& inputHandler_;
	int health_ = 100;

private:
	friend class GameStateManager;
	//Inherited from BehaviorImplementationBase
	virtual void update();
	void updateTransform();
	void handleStateTransition(BaseFighterState* transitionToState);

private:
	AttackResources standingAttacks_;
	BaseFighterState* state_;
	glm::vec3 gravity_ = { 0.0f, -0.5f, 0.0f };
	float terminalYSpeed_ = 0.25f;
	float movedThisFrame_;
	float deltaTime_;
};

