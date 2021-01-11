#include "stdafx.h"
#include "AlienBehaviours.h"
#include "MicroAIAgent.h"
#include "Alien.h"
#include "Job.h"
#include "Jobs.h"
#include <deque>
const Elite::Vector2 CalculateNewInvestigationArea(Elite::Blackboard* pBlackboard)
{
	Alien* pAlien{};
	if (!pBlackboard->GetData("alien", pAlien))
		return Elite::Vector2{};

	const float distanceToGo{ 150.f };
	Elite::Vector2 randomPosition{};
	do
	{
		randomPosition = Elite::randomVector2(-85.f, 85.f);
	} while (Elite::DistanceSquared(randomPosition, pAlien->GetPosition()) >= (distanceToGo * distanceToGo));
	return randomPosition;
}

// -------- WANDER --------
Elite::BehaviorState WanderBehaviour(Elite::Blackboard* pBlackboard)
{
	Alien* pAlien{};
	if (!pBlackboard->GetData("alien", pAlien))
		return Elite::BehaviorState::Failure;

	pAlien->SetToWander();
	return Elite::BehaviorState::Success;
}


// -------- IS PLAYER IN FOV --------
bool IsPlayerInFOV(Elite::Blackboard* pBlackboard)
{
	bool isPlayerInFOV{};
	if (!pBlackboard->GetData("isPlayerInFOV", isPlayerInFOV))
		return false;

	return isPlayerInFOV;
}


// -------- IS PURSUIT AVAILABLE --------
bool IsPursuitAvailable(Elite::Blackboard* pBlackboard)
{
	bool IsPursuitAvailable{};
	if (!pBlackboard->GetData("isPursuitAvailable", IsPursuitAvailable))
		return false;

	return IsPursuitAvailable;
}


// -------- PURSUIT PLAYER --------
Elite::BehaviorState PursuitPlayer(Elite::Blackboard* pBlackboard)
{
	Alien* pAlien{};
	MicroAIAgent* pPlayer{};
	if (!pBlackboard->GetData("alien", pAlien) || !pBlackboard->GetData("agent", pPlayer))
		return Elite::BehaviorState::Failure;

	pAlien->SetToPursuit(pPlayer);
	return Elite::BehaviorState::Success;
}


// -------- CHASE PLAYER --------
Elite::BehaviorState ChasePlayer(Elite::Blackboard* pBlackboard)
{
	Alien* pAlien{};
	MicroAIAgent* pPlayer{};
	if (!pBlackboard->GetData("alien", pAlien) || !pBlackboard->GetData("agent", pPlayer))
		return Elite::BehaviorState::Failure;

	pAlien->SetToSeek(pPlayer->GetPosition());
	return Elite::BehaviorState::Success;
}


// -------- IS THERE A CURRENT JOB --------
bool IsThereACurrentJob(Elite::Blackboard* pBlackboard)
{
	std::deque<Job*>* pJobs{};
	if (!pBlackboard->GetData("jobs", pJobs))
		return false;

	return !pJobs->empty();
}


// -------- EXECUTE CURRENT JOB --------
Elite::BehaviorState ExecuteFirstJob(Elite::Blackboard* pBlackboard)
{
	std::deque<Job*>* pJobs{};
	Alien* pAlien{};
	MicroAIAgent* pPlayer{};
	if (!pBlackboard->GetData("jobs", pJobs) || !pBlackboard->GetData("alien", pAlien) || !pBlackboard->GetData("agent", pPlayer))
		return Elite::BehaviorState::Failure;

	const JobState currentJobState{ (*pJobs)[0]->ExecuteJob(pBlackboard) };

	switch (currentJobState)
	{
	case JobState::COMPLETE:
		switch ((*pJobs)[0]->GetJobType())
		{
			SAFE_DELETE((*pJobs)[0]);
			pJobs->pop_front();

		case JobType::INVESTIGATE:
		case JobType::COOLDOWN:
			if (pJobs->empty())
			{
				const Elite::Vector2 randomPosition{ CalculateNewInvestigationArea(pBlackboard) };
				pBlackboard->ChangeData("investigationTarget", randomPosition);
				pAlien->AddJob(new Job{ InvestigateArea,JobPriority::NORMAL,JobType::INVESTIGATE });
			}
			return Elite::BehaviorState::Success;
			break;
		default:
			break;
		}
		break;
	case JobState::RUNNING:
		return Elite::BehaviorState::Success;
		break;
	case JobState::FAILURE:
#ifdef _DEBUG || DEBUG
		assert(currentJobState != JobState::FAILURE);
#endif
		return Elite::BehaviorState::Failure;
		break;
	default:
		break;
	}
	return Elite::BehaviorState::Failure;
}


// -------- MAKE A JOB --------
Elite::BehaviorState MakeJob(Elite::Blackboard* pBlackboard)
{
	std::deque<Job*>* pJobs{};
	Alien* pAlien{};
	if (!pBlackboard->GetData("jobs", pJobs) || !pBlackboard->GetData("alien", pAlien))
		return Elite::BehaviorState::Failure;

	const float distanceToGo{ 50.f };
	Elite::Vector2 randomPosition{};
	do
	{
		randomPosition = Elite::randomVector2(-85.f, 85.f);
	} while (Elite::DistanceSquared(randomPosition, pAlien->GetPosition()) >= (distanceToGo * distanceToGo));
	pBlackboard->ChangeData("investigationTarget", randomPosition);
	pJobs->push_back(new Job{ InvestigateArea,JobPriority::NORMAL,JobType::INVESTIGATE });
	return Elite::BehaviorState::Success;
}