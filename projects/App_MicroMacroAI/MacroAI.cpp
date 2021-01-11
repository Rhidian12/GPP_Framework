#include "stdafx.h"
#include "MacroAI.h"
#include "framework/EliteAI/EliteData/EBlackboard.h"
#include "Alien.h"
#include "MicroAIAgent.h"
#include "ExceptionHandler.h"
#include "Jobs.h"
#include "Job.h"
#include "CollisionFunctions.h"
void MacroAI::Update(const std::vector<Elite::Vector2>& pickupsInWorld, const int maxAmountOfPickups, Elite::Blackboard* pAlienBlackboard, const Elite::Rect& worldBounds)
{
	Alien* pAlien{};
	MicroAIAgent* pPlayer{};
	if (!pAlienBlackboard->GetData("alien", pAlien) || !pAlienBlackboard->GetData("agent", pPlayer))
	{
		ExceptionHandler e{};
		e.ProcessException();
	}

	if (int(pickupsInWorld.size()) <= maxAmountOfPickups - maxAmountOfPickups / 3) // 1/3 of pickups have been picked up
	{
		pAlienBlackboard->ChangeData("isHearingEnabled", true);
	}
	if (int(pickupsInWorld.size()) <= maxAmountOfPickups / 2) // half of the pickups have been picked up
	{
		pAlienBlackboard->ChangeData("isPursuitAvailable", true);
	}


	// Alien can only have 1 priority job
	if (pAlien->GetJobs()[0]->GetJobPriority() != JobPriority::PRIORITY)
	{
		const float cornersOfWorldSize{ 50.f };
		const Elite::Vector2 playerPosition{ pPlayer->GetPosition() };
		if(IsPlayerInACornerOfTheWorld(playerPosition,worldBounds,cornersOfWorldSize))
		{
			// player is in a corner of the world
			if (m_LastPriorityJobAdded != JobType::COOLDOWN) // we don't want the alien to go on cooldown multiple times
			{
				if (Elite::DistanceSquared(playerPosition, pAlien->GetPosition()) <= (cornersOfWorldSize * cornersOfWorldSize))
				{
					pAlien->AddJob(new Job{ Cooldown,JobPriority::PRIORITY,JobType::COOLDOWN });
					m_LastPriorityJobAdded = JobType::COOLDOWN;
					std::cout << "PRIORITY COOLDOWN ADDED" << std::endl;
				}
			}
		}
		else if (Elite::DistanceSquared(playerPosition, pAlien->GetPosition()) >= (m_MaxDistanceBetweenAgents * m_MaxDistanceBetweenAgents))
		{
			CalculateInvestigationTargetNearPlayer(pAlienBlackboard);
			pAlien->AddJob(new Job{ InvestigateArea,JobPriority::PRIORITY,JobType::INVESTIGATE });
			m_LastPriorityJobAdded = JobType::INVESTIGATE;
			std::cout << "PRIORITY INVESTIGATION ADDED" << std::endl;
		}
	}
}

void MacroAI::CalculateInvestigationTargetNearPlayer(Elite::Blackboard* pAlienBlackboard) const
{
	Alien* pAlien{};
	MicroAIAgent* pPlayer{};
	if (!pAlienBlackboard->GetData("alien", pAlien) || !pAlienBlackboard->GetData("agent", pPlayer))
	{
		ExceptionHandler e{};
		e.ProcessException();
	}

	const float distanceToGo{ 50.f };
	Elite::Vector2 randomPosition{};
	do
	{
		randomPosition = Elite::randomVector2(-85.f, 85.f);
	} while (Elite::DistanceSquared(randomPosition, pPlayer->GetPosition()) <= (distanceToGo * distanceToGo));

	pAlienBlackboard->ChangeData("investigationTarget", randomPosition);
}

const bool MacroAI::IsPlayerInACornerOfTheWorld(const Elite::Vector2& playerPosition, const Elite::Rect& worldBounds, const float cornerSize) const
{
	return (Collisions::IsPointInRect(playerPosition, Elite::Rect{ worldBounds.bottomLeft,cornerSize,cornerSize }) ||
		Collisions::IsPointInRect(playerPosition, Elite::Rect{ {worldBounds.bottomLeft.x + worldBounds.width - cornerSize,worldBounds.bottomLeft.y},cornerSize,cornerSize }) ||
		Collisions::IsPointInRect(playerPosition, Elite::Rect{ {worldBounds.bottomLeft.x + worldBounds.width - cornerSize,worldBounds.bottomLeft.y + worldBounds.height - cornerSize},cornerSize,cornerSize }) ||
		Collisions::IsPointInRect(playerPosition, Elite::Rect{ {worldBounds.bottomLeft.x,worldBounds.bottomLeft.y + worldBounds.height - cornerSize},cornerSize,cornerSize }));
}
