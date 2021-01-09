#include "stdafx.h"
#include "MacroAI.h"
#include "framework/EliteAI/EliteData/EBlackboard.h"
#include "Alien.h"
#include "MicroAIAgent.h"
#include "ExceptionHandler.h"
#include "Jobs.h"
#include "Job.h"
void MacroAI::Update(const std::vector<Elite::Vector2>& pickupsInWorld, const int maxAmountOfPickups, Elite::Blackboard* pAlienBlackboard)
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

	if (pAlien->GetJobs()[0]->GetJobPriority() != JobPriority::PRIORITY)
	{
		const float maxDistanceBetweenAgents{ 150.f };
		if (Elite::DistanceSquared(pPlayer->GetPosition(), pAlien->GetPosition()) >= (maxDistanceBetweenAgents * maxDistanceBetweenAgents))
		{
			CalculateInvestigationTarget(pAlienBlackboard);
			pAlien->AddJob(new Job{ InvestigateArea,JobPriority::PRIORITY,JobType::INVESTIGATE });
			std::cout << "PRIORITY INVESTIGATION ADDED" << std::endl;
		}
	}
}

void MacroAI::CalculateInvestigationTarget(Elite::Blackboard* pAlienBlackboard) const
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