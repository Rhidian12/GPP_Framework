#pragma once
#include "stdafx.h"
#include <vector>
class MicroAIAgent;
class Blackboard;
class MacroAI
{
public:
	MacroAI() = default;
	~MacroAI() = default;

	void Update(const std::vector<Elite::Vector2>& pickupsInWorld, const int maxAmountOfPickups, Elite::Blackboard* pAlienBlackboard);

private:
	void CalculateInvestigationTarget(Elite::Blackboard* pAlienBlackboard) const;

};