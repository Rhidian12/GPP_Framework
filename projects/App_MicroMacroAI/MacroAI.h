#pragma once
#include "stdafx.h"
#include <vector>
#include "Job.h"
class MicroAIAgent;
class Blackboard;
class MacroAI
{
public:
	MacroAI() = default;
	~MacroAI() = default;

	void Update(const std::vector<Elite::Vector2>& pickupsInWorld, const int maxAmountOfPickups, Elite::Blackboard* pAlienBlackboard, const Elite::Rect& worldBounds);

private:
	void CalculateInvestigationTargetNearPlayer(Elite::Blackboard* pAlienBlackboard) const;
	const bool IsPlayerInACornerOfTheWorld(const Elite::Vector2& playerPosition, const Elite::Rect& worldBounds, const float cornerSize) const;

	const float m_MaxDistanceBetweenAgents{ 150.f };
	JobType m_LastPriorityJobAdded;
};