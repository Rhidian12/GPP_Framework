#pragma once
#include "framework/EliteAI/EliteDecisionMaking/EliteFiniteStateMachine/EFiniteStateMachine.h"
#include "framework/EliteAI/EliteData/EBlackboard.h"
// ============
// STATES
// ============
using namespace Elite;
class WanderingState final : public Elite::FSMState
{
public:
	virtual void OnEnter(Blackboard* pBlackboard) override;
private:
};
class FollowSearchPatternState final : public Elite::FSMState
{
public:
	virtual void OnEnter(Blackboard* pBlackboard) override;
	virtual void Update(Blackboard* pBlackboard, float deltaTime) override;
private:
};

// ============
// TRANSITIONS
// ============
class HaveNotAllCheckpointsBeenVisited final : public Elite::FSMTransition
{
public:
	virtual bool ToTransition(Blackboard* pBlackboard) const override;
private:
};