#pragma once
#include "framework/EliteAI/EliteDecisionMaking/EliteFiniteStateMachine/EFiniteStateMachine.h"
#include "framework/EliteAI/EliteData/EBlackboard.h"
// ============
// STATES
// ============
using namespace Elite;
class PlayerWander final : public Elite::FSMState
{
public:
	virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
private:
};
class FollowSearchPatternState final : public Elite::FSMState
{
public:
	virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
	virtual void Update(Elite::Blackboard* pBlackboard, float deltaTime) override;
private:
};
class SeekState final : public Elite::FSMState
{
public:
	virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
	virtual void Update(Elite::Blackboard* pBlackboard, float deltaTime) override;
private:
};
class PickupPickupState final : public Elite::FSMState
{
public:
	virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
private:
};

// ============
// TRANSITIONS
// ============
class HaveNotAllCheckpointsBeenVisited final : public Elite::FSMTransition
{
public:
	virtual bool ToTransition(Elite::Blackboard* pBlackboard) const override;
private:
};
class AreTherePickupsInFOV final : public Elite::FSMTransition
{
public:
	virtual bool ToTransition(Elite::Blackboard* pBlackboard) const override;
private:
};
class IsAgentInPickupRange final : public Elite::FSMTransition
{
public:
	virtual bool ToTransition(Elite::Blackboard* pBlackboard) const override;
private:
};
class IsAlienInFOV final : public Elite::FSMTransition
{
public:
	virtual bool ToTransition(Elite::Blackboard* pBlackboard) const override;
private:
};