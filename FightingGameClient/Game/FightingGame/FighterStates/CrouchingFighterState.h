#pragma once
#include "BaseFighterState.h"

class CrouchingFighterState :
	public BaseFighterState
{
public:
	CrouchingFighterState(std::string animationName, std::vector<FrameInfo> frameData, AttackResources* attacks);
	BaseFighterState* update(Fighter* fighter) override;
	void enterState(Fighter* fighter) override;
	BaseFighterState* handleMovementInput(Fighter* fighter) override;
	BaseFighterState* handleAttackInput(Fighter* fighter) override;
	BaseFighterState* onHit(Fighter* fighter, OnHitType attack) override;
private:
	AttackResources* attacks_;
};

