#include "stdafx.h"
#include "Behaviours.h"
#include "MicroAIAgent.h"
#include "Structs.h"
#include <limits>
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

// ============ SEEK ============
void SeekState::OnEnter(Blackboard* pBlackboard)
{
	MicroAIAgent* pMicroAI{};
	std::vector<Elite::Vector2> pickupsInFOV{};
	if (!pBlackboard->GetData("microAI", pMicroAI) || !pBlackboard->GetData("pickupsInFOV", pickupsInFOV))
		return;

	if (pickupsInFOV.size() == 1)
	{
		pMicroAI->SetToSeek(pickupsInFOV.front());
	}
	else
	{
		float shortestDistance{ std::numeric_limits<float>::max() }; // same as FLT_MAX, but C++11
		const Elite::Vector2 agentPosition{ pMicroAI->GetPosition() };
		for (const auto& pickup : pickupsInFOV)
		{
			const float distance{ Elite::DistanceSquared(agentPosition, pickup) };
			if (distance < shortestDistance)
			{
				shortestDistance = distance;
				pMicroAI->SetToSeek(pickup);
			}
		}
	}
}
void SeekState::Update(Blackboard* pBlackboard, float deltaTime)
{
	MicroAIAgent* pMicroAI{};
	std::vector<Elite::Vector2> pickupsInFOV{};
	if (!pBlackboard->GetData("microAI", pMicroAI) || !pBlackboard->GetData("pickupsInFOV", pickupsInFOV))
		return;

	if (pickupsInFOV.size() == 1)
	{
		pMicroAI->SetToSeek(pickupsInFOV.front());
	}
	else
	{
		float shortestDistance{ std::numeric_limits<float>::max() }; // same as FLT_MAX, but C++11
		const Elite::Vector2 agentPosition{ pMicroAI->GetPosition() };
		for (const auto& pickup : pickupsInFOV)
		{
			const float distance{ Elite::DistanceSquared(agentPosition, pickup) };
			if (distance < shortestDistance)
			{
				shortestDistance = distance;
				pMicroAI->SetToSeek(pickup);
			}
		}
	}
}


// ============ PICK UP PICKUP ============
void PickupPickupState::OnEnter(Blackboard* pBlackboard)
{
	MicroAIAgent* pMicroAI{};
	std::vector<Elite::Vector2> pickupsInFOV{};
	std::vector<Elite::Vector2>* pickups{};
	float grabRange{};
	if (!pBlackboard->GetData("microAI", pMicroAI) || !pBlackboard->GetData("pickupsInFOV", pickupsInFOV) || !pBlackboard->GetData("pickups", pickups)
		|| !pBlackboard->GetData("grabRange", grabRange))
		return;

	const Elite::Vector2 microAIPosition{ pMicroAI->GetPosition() };
	for (const auto& pickup : pickupsInFOV)
	{
		if (Elite::DistanceSquared(microAIPosition, pickup) <= (grabRange * grabRange))
		{
			auto it = std::find_if(pickups->begin(), pickups->end(), [&pickup](const Elite::Vector2& a)->bool
				{
					return pickup == a;
				});
			if (it != pickups->end())
			{
				pickups->erase(it);
				pBlackboard->ChangeData("pickups", pickups);
				pBlackboard->ChangeData("wasPickupSeen", false);
				return;
			}
		}
	}
}


// ============ CHECK SEARCH PATTERN ============
bool HaveNotAllCheckpointsBeenVisited::ToTransition(Blackboard* pBlackboard) const
{
	std::vector<Checkpoint>* pCheckpoints{};
	bool wasPickupSeen{};
	if (!pBlackboard->GetData("checkpoints", pCheckpoints) || !pBlackboard->GetData("wasPickupSeen", wasPickupSeen))
		return false;

	if (wasPickupSeen)
		return false;

	for (const auto& checkpoint : *pCheckpoints)
	{
		if (!checkpoint.hasBeenVisited)
			return true;
	}
	return false;
}


// ============ CHECK IF THERE ARE PICKUPS IN THE FOV ============
bool AreTherePickupsInFOV::ToTransition(Blackboard* pBlackboard) const
{
	std::vector<Elite::Vector2> pickupsInFOV{};
	if (!pBlackboard->GetData("pickupsInFOV", pickupsInFOV))
		return false;

	
	if (!pickupsInFOV.empty())
	{
		return pBlackboard->ChangeData("wasPickupSeen", true);
	}
	return false;
}


// ============ CHECK IF THERE ARE PICKUPS IN THE FOV THAT ARE CLOSE ENOUGH TO BE PICKED UP ============
bool IsAgentInPickupRange::ToTransition(Blackboard* pBlackboard) const
{
	MicroAIAgent* pMicroAI{};
	float grabRange{};
	std::vector<Elite::Vector2> pickupsInFOV{};
	if (!pBlackboard->GetData("microAI", pMicroAI) || !pBlackboard->GetData("grabRange", grabRange) || !pBlackboard->GetData("pickupsInFOV", pickupsInFOV))
		return false;

	const Elite::Vector2 microAIPosition{ pMicroAI->GetPosition() };
	for (const auto& pickup : pickupsInFOV)
	{
		if (Elite::DistanceSquared(microAIPosition, pickup) <= (grabRange * grabRange))
			return true;
	}
	return false;
}


// ============ CHECK IF THE ALIEN IS IN THE FOV ============
bool IsAlienInFOV::ToTransition(Blackboard* pBlackboard) const
{
	return false;
}