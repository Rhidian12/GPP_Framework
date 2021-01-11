/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// Behaviors.h: Implementation of certain reusable behaviors for the BT version of the Agario Game
/*=============================================================================*/
#ifndef ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
#define ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteMath/EMath.h"
#include "framework/EliteAI/EliteDecisionMaking/EliteBehaviorTree/EBehaviorTree.h"
#include "../Shared/Agario/AgarioAgent.h"
#include "../Shared/Agario/AgarioFood.h"
#include "../App_Steering/SteeringBehaviors.h"

//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------
bool IsCloseToFood(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	std::vector<AgarioFood*>* foodVec = nullptr;

	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) &&
		pBlackboard->GetData("FoodVec", foodVec);

	if (!pAgent || !foodVec)
		return false;

	//TODO: Check for food closeby and set target accordingly
	const float rangeToCheckForFood{ 20.f };
	auto foodIt{ std::find_if(foodVec->begin(),foodVec->end(),[&pAgent,&rangeToCheckForFood](AgarioFood* pFood)
		{
			// true if close to agent
			return Elite::DistanceSquared(pAgent->GetPosition(),pFood->GetPosition()) <= (rangeToCheckForFood * rangeToCheckForFood);
		}) };

	if (foodIt != foodVec->end())
	{
		pBlackboard->ChangeData("Target", (*foodIt)->GetPosition());
		return true;
	}

	return false;
}
bool IsLargerEnemyClose(Elite::Blackboard* pBlackboard)
{
#pragma region Blackboard
	AgarioAgent* pSmartAgent = nullptr;
	std::vector<AgarioAgent*>* pDumbAgents{};
	float radiusToCheckForLargerEnemies{};

	bool dataAvailable{ pBlackboard->GetData("smartAgent", pSmartAgent) && pBlackboard->GetData("dumbAgents",pDumbAgents)
				&& pBlackboard->GetData("radiusToCheckForLargerEnemies",radiusToCheckForLargerEnemies) };

	if (!dataAvailable || pSmartAgent == nullptr || pDumbAgents->empty()) // == Safety Checks ==
		return false;
#pragma endregion

#pragma region CheckForLargerEnemies
	const float radiusOfSmartAgent{ pSmartAgent->GetRadius() };
	const Elite::Vector2 positionOfSmartAgent{ pSmartAgent->GetPosition() };
	radiusToCheckForLargerEnemies += radiusOfSmartAgent; // == Make Sure We Take Our Own Radius Into Account ==
	AgarioAgent* pLargerEnemyToRunAwayFrom{};
	float shortestDistanceSoFar{ FLT_MAX }; // == Start At FLT_MAX, So We Can Decrease It With Distance Checks ==
	bool isALargerEnemyClose{};

	for (AgarioAgent* pDumbAgent : *pDumbAgents)
	{
		if (pDumbAgent->CanBeDestroyed())
			continue; // if the agent is marked for destruction, continue to the next agent

		const Elite::Vector2 positionOfDumbAgent{ pDumbAgent->GetPosition() };
		const float radiusOfDumbAgent{ pDumbAgent->GetRadius() };

		for (int i{}; i < 360; i += 5)
		{
			// We create a line from the center of the dumb agent to the edge of its radius
			// we then check if that line intersects with a circle, origin at the center of the smart agent
			// and that circle has a radius of smart agent radius + radius we want to check
			// we then rotate that edgeOfRadius 360 degrees around and check everytime if it hits the circle of the smart agent
			const float s{ sinf(float(i)) };
			const float c{ cosf(float(i)) };

			const Elite::Vector2 edgeOfRadius{ positionOfDumbAgent.x + radiusOfDumbAgent * c, positionOfDumbAgent.y + radiusOfDumbAgent * s };
			if (Elite::IsSegmentIntersectingWithCircle(positionOfDumbAgent, edgeOfRadius, positionOfSmartAgent, radiusToCheckForLargerEnemies))
			{
				if (radiusOfDumbAgent > radiusOfSmartAgent + 1)
				{
					isALargerEnemyClose = true;

					const float distanceToLargeAgent{ Elite::DistanceSquared(positionOfSmartAgent,positionOfDumbAgent) };
					if (distanceToLargeAgent < shortestDistanceSoFar)
					{
						// we try to find the largest agent which is also the closest to the player
						shortestDistanceSoFar = distanceToLargeAgent;
						pLargerEnemyToRunAwayFrom = pDumbAgent;
					}
				}
			}
		}
	}

	if (isALargerEnemyClose)
	{
		pBlackboard->ChangeData("fleeTarget", pLargerEnemyToRunAwayFrom);
		return true;
	}
	else
		return false;
#pragma endregion
}
bool IsMinRadiusReached(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pSmartAgent{};
	float minimumRadius{};

	auto dataAvailable{ pBlackboard->GetData("smartAgent",pSmartAgent) && pBlackboard->GetData("minimumRadius",minimumRadius) };

	if (!dataAvailable || pSmartAgent == nullptr)
		return false;

	if (pSmartAgent->GetRadius() >= minimumRadius)
		return true;
	else
		return false;
}
bool IsSmallerEnemyClose(Elite::Blackboard* pBlackboard)
{
#pragma region Getting Information From Blackboard
	AgarioAgent* pSmartAgent{};
	bool dataAvailable{ pBlackboard->GetData("smartAgent",pSmartAgent) };

	if (!dataAvailable || pSmartAgent == nullptr)
		return false; // safety checks

	std::vector<AgarioAgent*>* pDumbAgents{};
	dataAvailable = pBlackboard->GetData("dumbAgents", pDumbAgents);

	if (!dataAvailable || pDumbAgents->empty())
		return false; // safety check + there can't be smaller enemies if there are no enemies

	float radiusToCheckForSmallerEnemies{};
	dataAvailable = pBlackboard->GetData("radiusToCheckForSmallerEnemies", radiusToCheckForSmallerEnemies);

	if (!dataAvailable)
		return false; // safety checks
#pragma endregion

#pragma region CheckForSmallerEnemies
	const float radiusOfSmartAgent{ pSmartAgent->GetRadius() };
	radiusToCheckForSmallerEnemies += radiusOfSmartAgent; // take our own radius into account
	const Elite::Vector2 positionOfSmartAgent{ pSmartAgent->GetPosition() };
	AgarioAgent* pClosestAgentToChase{}; // this is the agent that will be passed along
	float shortestDistanceSoFar{ FLT_MAX }; // set to FLT_MAX so we can compare distances to it
	bool isASmallerEnemyClose{};

	for (AgarioAgent* pDumbAgent : *pDumbAgents)
	{
		if (pDumbAgent->CanBeDestroyed())
			continue; // if agent is marked for destruction, skip it, because it would lead to nullptr errors

		const Elite::Vector2 positionOfDumbAgent{ pDumbAgent->GetPosition() };
		const float radiusOfDumbAgent{ pDumbAgent->GetRadius() };

		for (int i{}; i < 360; i += 5)
		{
			// We create a line from the center of the dumb agent to the edge of its radius
			// we then check if that line intersects with a circle, origin at the center of the smart agent
			// and that circle has a radius of smart agent radius + radius we want to check
			// we then rotate that edgeOfRadius 360 degrees around and check everytime if it hits the circle of the smart agent
			const float s{ sinf(float(i)) };
			const float c{ cosf(float(i)) };

			const Elite::Vector2 edgeOfRadius{ positionOfDumbAgent.x + radiusOfDumbAgent * c, positionOfDumbAgent.y + radiusOfDumbAgent * s };
			if (Elite::IsSegmentIntersectingWithCircle(positionOfDumbAgent, edgeOfRadius, positionOfSmartAgent, radiusToCheckForSmallerEnemies))
			{
				// the agent is at least 1/3 of the smart agent
				const float minimumSizeOfEnemy{ radiusOfSmartAgent * 0.333333f };
				if (radiusOfDumbAgent > minimumSizeOfEnemy)
				{
					if (radiusOfDumbAgent + 1 < radiusOfSmartAgent)
					{
						isASmallerEnemyClose = true;

						const float distanceToLargeAgent{ Elite::DistanceSquared(positionOfSmartAgent,positionOfDumbAgent) };
						if (distanceToLargeAgent < shortestDistanceSoFar)
						{
							// we try to find the largest agent which is also the closest to the player
							shortestDistanceSoFar = distanceToLargeAgent;
							pClosestAgentToChase = pDumbAgent;
						}
					}
				}
			}
		}
	}
#pragma endregion

	if (isASmallerEnemyClose)
	{
		pBlackboard->ChangeData("chaseTarget", pClosestAgentToChase);
		return true;
	}
	else
		return false;
}

BehaviorState ChaseSmallerEnemy(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pSmartAgent{};
	AgarioAgent* pAgentToChase{};

	auto dataAvailable{ pBlackboard->GetData("smartAgent",pSmartAgent) && pBlackboard->GetData("chaseTarget",pAgentToChase) };

	if (!dataAvailable || pSmartAgent == nullptr || pAgentToChase == nullptr)
		return Failure;

	pSmartAgent->SetToSeek(pAgentToChase->GetPosition());

	std::cout << "Chasing Smaller Enemy" << std::endl;

	return Success;
}
BehaviorState ChangeToFlee(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pSmartAgent{};
	AgarioAgent* pFleeAgent{};

	auto dataAvailable{ pBlackboard->GetData("smartAgent",pSmartAgent) && pBlackboard->GetData("fleeTarget",pFleeAgent) };

	if (!dataAvailable || pSmartAgent == nullptr || pFleeAgent == nullptr)
		return Failure;

	pSmartAgent->SetToFlee(pFleeAgent->GetPosition());

	std::cout << "Fleeing From Larger Enemy" << std::endl;

	return Success;
}
BehaviorState ChangeToWander(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	auto dataAvailable = pBlackboard->GetData("Agent", pAgent);

	if (!pAgent)
		return Failure;

	pAgent->SetToWander();

	return Success;
}

BehaviorState ChangeToSeek(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	Elite::Vector2 seekTarget{};
	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) &&
		pBlackboard->GetData("Target", seekTarget);

	if (!pAgent)
		return Failure;

	//TODO: Implement Change to seek (Target)
	pAgent->SetToSeek(seekTarget);

	std::cout << "Seeking Food" << std::endl;

	return Success;
}

#endif