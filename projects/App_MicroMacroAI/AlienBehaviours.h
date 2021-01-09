#pragma once
#include "framework/EliteAI/EliteDecisionMaking/EDecisionMaking.h"
#include "framework/EliteAI/EliteData/EBlackboard.h"
Elite::BehaviorState WanderBehaviour(Elite::Blackboard* pBlackboard);
bool IsPlayerInFOV(Elite::Blackboard* pBlackboard);
bool IsPursuitAvailable(Elite::Blackboard* pBlackboard);
Elite::BehaviorState PursuitPlayer(Elite::Blackboard* pBlackboard);
Elite::BehaviorState ChasePlayer(Elite::Blackboard* pBlackboard);
bool IsThereACurrentJob(Elite::Blackboard* pBlackboard);
Elite::BehaviorState ExecuteFirstJob(Elite::Blackboard* pBlackboard);
Elite::BehaviorState MakeJob(Elite::Blackboard* pBlackboard);