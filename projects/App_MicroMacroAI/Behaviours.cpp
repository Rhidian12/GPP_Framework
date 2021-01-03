#include "stdafx.h"
#include "Behaviours.h"
#include "MicroAIAgent.h"
#include "Structs.h"
// ============ WANDER ============
void WanderingState::OnEnter(Blackboard* pBlackboard)
{
	MicroAIAgent* pMicroAI{};
	if (!pBlackboard->GetData("microAI", pMicroAI))
		return;

	pMicroAI->SetToWander();
}


// ============ FOLLOW SEARCH PATTERN ============
void FollowSearchPatternState::OnEnter(Blackboard* pBlackboard)
{
	MicroAIAgent* pMicroAI{};
	std::vector<Checkpoint>* pCheckpoints{};
	if (!pBlackboard->GetData("microAI", pMicroAI) || !pBlackboard->GetData("checkpoints", pCheckpoints))
		return;

	for (const auto& checkpoint : *pCheckpoints)
	{
		if (!checkpoint.hasBeenVisited)
		{
			pMicroAI->SetToSeek(checkpoint.position);
			pBlackboard->ChangeData("target", checkpoint.position);
			return;
		}
	}
}
void FollowSearchPatternState::Update(Blackboard* pBlackboard, float deltaTime)
{
	MicroAIAgent* pMicroAI{};
	std::vector<Checkpoint>* pCheckpoints{};
	if (!pBlackboard->GetData("microAI", pMicroAI) || !pBlackboard->GetData("checkpoints", pCheckpoints))
		return;

	for (const auto& checkpoint : *pCheckpoints)
	{
		if (!checkpoint.hasBeenVisited)
		{
			pMicroAI->SetToSeek(checkpoint.position);
			pBlackboard->ChangeData("target", checkpoint.position);
			return;
		}
	}
}


// ============ CHECK SEARCH PATTERN ============
bool HaveNotAllCheckpointsBeenVisited::ToTransition(Blackboard* pBlackboard) const
{
	std::vector<Checkpoint>* pCheckpoints{};
	if (!pBlackboard->GetData("checkpoints", pCheckpoints))
		return false;

	for (const auto& checkpoint : *pCheckpoints)
	{
		if (!checkpoint.hasBeenVisited)
			return true;
	}
	return false;
}
