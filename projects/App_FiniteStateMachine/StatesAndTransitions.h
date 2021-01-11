/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// StatesAndTransitions.h: Implementation of the state/transition classes
/*=============================================================================*/
#ifndef ELITE_APPLICATION_FSM_STATES_TRANSITIONS
#define ELITE_APPLICATION_FSM_STATES_TRANSITIONS

#include "../Shared/Agario/AgarioAgent.h"
#include "../Shared/Agario/AgarioFood.h"
#include "../App_Steering/SteeringBehaviors.h"

// ============
// STATES
// ============
class WanderState final : public Elite::FSMState
{
public:
	virtual void OnEnter(Blackboard* pBlackboard) override
	{
#pragma region Getting Information From Blackboard
		bool isSmartAgent{};
		bool dataAvailable{ pBlackboard->GetData("isSmartAgent",isSmartAgent) };

		if (!dataAvailable)
			return; // safety check
#pragma endregion

		// Check to see which type of agent is calling the function: the smart agent or a dumb agent
		if (isSmartAgent)
		{
			AgarioAgent* pSmartAgent{};
			dataAvailable = pBlackboard->GetData("SmartAgent", pSmartAgent);

			if (!dataAvailable || pSmartAgent == nullptr)
				return; // safety checks

			pSmartAgent->SetToWander();
		}
		else
		{
			AgarioAgent* pAgent{};
			bool dataAvailable{ pBlackboard->GetData("Agent",pAgent) };

			if (!dataAvailable || pAgent == nullptr)
				return; // safety checks

			pAgent->SetToWander();
		}
	}

private:

};
class SeekFood final : public Elite::FSMState
{
public:
	virtual void OnEnter(Blackboard* pBlackboard) override
	{
#pragma region Getting Information From Blackboard
		AgarioAgent* pSmartAgent{};
		bool dataAvailable{ pBlackboard->GetData("SmartAgent",pSmartAgent) };

		if (!dataAvailable || pSmartAgent == nullptr)
			return;

		std::vector<AgarioFood*>* pMeals{};
		dataAvailable = pBlackboard->GetData("FoodVector", pMeals);

		if (!dataAvailable || pMeals->empty())
			return; // if there is no food, we can return

		float radiusToCheckForFood{};
		dataAvailable = pBlackboard->GetData("RadiusToCheckForFood", radiusToCheckForFood);

		if (!dataAvailable)
			return; // safety check, if radius == 0.f, we won't find much food
		radiusToCheckForFood += pSmartAgent->GetRadius(); // take our own radius into account
#pragma endregion

#pragma region Finding Nearest Food
		const float radiusToCheckForFoodSquared{ radiusToCheckForFood * radiusToCheckForFood };
		const Elite::Vector2 smartAgentPosition{ pSmartAgent->GetPosition() };
		Elite::Vector2 positionOfClosestFood{};
		float squaredDistanceToClosestFood{ FLT_MAX }; // initialize it as max, so we can do < checks with the distance
		for (AgarioFood* pFood : *pMeals)
		{
			if (pFood->CanBeDestroyed())
				continue; // if the food is marked for destruction, don't use it, because it would cause nullptr errors

			const Elite::Vector2 foodPosition{ pFood->GetPosition() };
			// use squared distance since it is a cheaper function than Distance()
			const float distanceToFoodSquared{ Elite::DistanceSquared(smartAgentPosition,foodPosition) };
			if (distanceToFoodSquared <= radiusToCheckForFoodSquared)
			{
				if (distanceToFoodSquared < squaredDistanceToClosestFood)
				{
					positionOfClosestFood = foodPosition;
					squaredDistanceToClosestFood = distanceToFoodSquared;
				}
			}
		}
#pragma endregion

		pSmartAgent->SetToSeek(positionOfClosestFood);
	}
	virtual void Update(Blackboard* pBlackboard, float deltaTime) override
	{
#pragma region Getting Information From Blackboard
		AgarioAgent* pSmartAgent{};
		bool dataAvailable{ pBlackboard->GetData("SmartAgent",pSmartAgent) };

		if (!dataAvailable || pSmartAgent == nullptr)
			return;

		std::vector<AgarioFood*>* pMeals{};
		dataAvailable = pBlackboard->GetData("FoodVector", pMeals);

		if (!dataAvailable || pMeals->empty())
			return; // if there is no food, we can return

		float radiusToCheckForFood{};
		dataAvailable = pBlackboard->GetData("RadiusToCheckForFood", radiusToCheckForFood);

		if (!dataAvailable)
			return; // safety check, if radius == 0.f, we won't find much food
		radiusToCheckForFood += pSmartAgent->GetRadius(); // take our own radius into account
#pragma endregion

#pragma region Finding Nearest Food
		const float radiusToCheckForFoodSquared{ radiusToCheckForFood * radiusToCheckForFood };
		const Elite::Vector2 smartAgentPosition{ pSmartAgent->GetPosition() };
		Elite::Vector2 positionOfClosestFood{};
		float squaredDistanceToClosestFood{ FLT_MAX }; // initialize it as max, so we can do < checks with the distance
		for (AgarioFood* pFood : *pMeals)
		{
			if (pFood->CanBeDestroyed())
				continue; // if the food is marked for destruction, don't use it, because it would cause nullptr errors

			const Elite::Vector2 foodPosition{ pFood->GetPosition() };
			// use squared distance since it is a cheaper function than Distance()
			const float distanceToFoodSquared{ Elite::DistanceSquared(smartAgentPosition,foodPosition) };
			if (distanceToFoodSquared <= radiusToCheckForFoodSquared)
			{
				if (distanceToFoodSquared < squaredDistanceToClosestFood)
				{
					positionOfClosestFood = foodPosition;
					squaredDistanceToClosestFood = distanceToFoodSquared;
				}
			}
		}
#pragma endregion

		pSmartAgent->SetToSeek(positionOfClosestFood);
	}

private:
};
class EvadeLargerEnemy final : public Elite::FSMState
{
public:
	virtual void OnEnter(Blackboard* pBlackboard) override
	{
#pragma region Initializing Blackboard
		AgarioAgent* pSmartAgent{};
		bool dataAvailable{ pBlackboard->GetData("SmartAgent",pSmartAgent) };

		if (!dataAvailable || pSmartAgent == nullptr)
			return; // safety check for nullptr

		std::vector<AgarioAgent*>* pDumbAgents{};
		dataAvailable = pBlackboard->GetData("DumbAgents", pDumbAgents);

		if (!dataAvailable || pDumbAgents->empty())
			return; // safety check + if there are no agents, there can't be any larger enemies

		float radiusToCheckForLargerEnemies{};
		dataAvailable = pBlackboard->GetData("radiusToCheckForLargerEnemies", radiusToCheckForLargerEnemies);

		if (!dataAvailable)
			return; // safety check + if radius is 0.f, we won't find many enemies
		radiusToCheckForLargerEnemies += pSmartAgent->GetRadius(); // take our own radius into account
#pragma endregion

#pragma region Checking For Enemies
		const Elite::Vector2 positionOfSmartAgent{ pSmartAgent->GetPosition() };
		const float radiusOfSmartAgent{ pSmartAgent->GetRadius() };
		const float totalRadiusToCheckForEnemies{ radiusOfSmartAgent + radiusToCheckForLargerEnemies };
		AgarioAgent* pLargerEnemyToRunAwayFrom{};
		float shortestDistanceSoFar{ FLT_MAX }; // start at float max, so we can decrease it with checks
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
				if (Elite::IsSegmentIntersectingWithCircle(positionOfDumbAgent, edgeOfRadius, positionOfSmartAgent, totalRadiusToCheckForEnemies))
				{
					if (radiusOfDumbAgent > radiusOfSmartAgent + 1)
					{
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
#pragma endregion

		// IGNORE WARNING: TRANSITIONS ASSURE THAT THERE IS AN ENEMY
		pSmartAgent->SetToFlee(pLargerEnemyToRunAwayFrom->GetPosition());
	}
	virtual void Update(Blackboard* pBlackboard, float deltaTime) override
	{
#pragma region Initializing Blackboard
		AgarioAgent* pSmartAgent{};
		bool dataAvailable{ pBlackboard->GetData("SmartAgent",pSmartAgent) };

		if (!dataAvailable || pSmartAgent == nullptr)
			return; // safety check for nullptr

		std::vector<AgarioAgent*>* pDumbAgents{};
		dataAvailable = pBlackboard->GetData("DumbAgents", pDumbAgents);

		if (!dataAvailable || pDumbAgents->empty())
			return; // safety check + if there are no agents, there can't be any larger enemies

		float radiusToCheckForLargerEnemies{};
		dataAvailable = pBlackboard->GetData("radiusToCheckForLargerEnemies", radiusToCheckForLargerEnemies);

		if (!dataAvailable)
			return; // safety check + if radius is 0.f, we won't find many enemies
		radiusToCheckForLargerEnemies += pSmartAgent->GetRadius(); // take our own radius into account
#pragma endregion

#pragma region Checking For Enemies
		const Elite::Vector2 positionOfSmartAgent{ pSmartAgent->GetPosition() };
		const float radiusOfSmartAgent{ pSmartAgent->GetRadius() };
		const float totalRadiusToCheckForEnemies{ radiusOfSmartAgent + radiusToCheckForLargerEnemies };
		AgarioAgent* pLargerEnemyToRunAwayFrom{};
		float shortestDistanceSoFar{ FLT_MAX }; // start at float max, so we can decrease it with checks
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
				if (Elite::IsSegmentIntersectingWithCircle(positionOfDumbAgent, edgeOfRadius, positionOfSmartAgent, totalRadiusToCheckForEnemies))
				{
					if (radiusOfDumbAgent > radiusOfSmartAgent + 1)
					{
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
#pragma endregion

		// IGNORE WARNING: TRANSITIONS ASSURE THAT THERE IS AN ENEMY
		pSmartAgent->SetToFlee(pLargerEnemyToRunAwayFrom->GetPosition());
	}
private:
};
class SeekSmallerEnemy final : public Elite::FSMState
{
public:
	SeekSmallerEnemy() = default;
	virtual void OnEnter(Blackboard* pBlackboard) override
	{
#pragma region Getting Information From Blackboard
		AgarioAgent* pSmartAgent{};
		bool dataAvailable{ pBlackboard->GetData("SmartAgent",pSmartAgent) };

		if (!dataAvailable || pSmartAgent == nullptr)
			return; // safety checks

		std::vector<AgarioAgent*>* pDumbAgents{};
		dataAvailable = pBlackboard->GetData("DumbAgents", pDumbAgents);

		if (!dataAvailable || pDumbAgents->empty())
			return; // safety check + there can't be smaller enemies if there are no enemies

		float radiusToCheckForSmallerEnemies{};
		dataAvailable = pBlackboard->GetData("radiusToCheckForSmallerEnemies", radiusToCheckForSmallerEnemies);

		if (!dataAvailable)
			return; // safety checks
		radiusToCheckForSmallerEnemies += pSmartAgent->GetRadius(); // take our own radius into account
#pragma endregion

#pragma region Checking For Smaller Enemies
		const Elite::Vector2 positionOfSmartAgent{ pSmartAgent->GetPosition() };
		const float radiusToCheckSquared{ radiusToCheckForSmallerEnemies * radiusToCheckForSmallerEnemies };
		const float smartAgentRadius{ pSmartAgent->GetRadius() };
		AgarioAgent* pClosestAgentToChase{}; // this is the agent that will be passed along
		float shortestDistanceToAgentToChase{ FLT_MAX }; // set to FLT_MAX so we can compare distances to it
		for (AgarioAgent* pDumbAgent : *pDumbAgents)
		{
			if (pDumbAgent->CanBeDestroyed())
				continue; // if agent is marked for destruction, skip it, because it would lead to nullptr errors

			const Elite::Vector2 positionOfDumbAgent{ pDumbAgent->GetPosition() };
			const float distanceToDumbAgentSquared{ Elite::DistanceSquared(positionOfSmartAgent,positionOfDumbAgent) };
			if (distanceToDumbAgentSquared <= radiusToCheckSquared)
			{
				// an enemy is in the radius to check
				const float radiusOfDumbAgent{ pDumbAgent->GetRadius() };
				const float minimumSizeOfEnemy{ smartAgentRadius * 0.333333f };
				if (radiusOfDumbAgent > minimumSizeOfEnemy)
				{
					// the agent is at least 1/3 of the smart agent
					if (radiusOfDumbAgent + 1 < smartAgentRadius)
					{
						// the dumb agent is smaller than the smart agent
						if (distanceToDumbAgentSquared < shortestDistanceToAgentToChase)
						{
							// it is the closest to the smart agent
							shortestDistanceToAgentToChase = distanceToDumbAgentSquared;
							pClosestAgentToChase = pDumbAgent;
						}
					}
				}
			}
		}
#pragma endregion

		// IGNORE WARNING: TRANSITIONS ASSURE THAT THERE IS AN ENEMY
		pSmartAgent->SetToSeek(pClosestAgentToChase->GetPosition());
	}
	virtual void Update(Blackboard* pBlackboard, float deltaTime) override
	{
#pragma region Getting Information From Blackboard
		AgarioAgent* pSmartAgent{};
		bool dataAvailable{ pBlackboard->GetData("SmartAgent",pSmartAgent) };

		if (!dataAvailable || pSmartAgent == nullptr)
			return; // safety checks

		std::vector<AgarioAgent*>* pDumbAgents{};
		dataAvailable = pBlackboard->GetData("DumbAgents", pDumbAgents);

		if (!dataAvailable || pDumbAgents->empty())
			return; // safety check + there can't be smaller enemies if there are no enemies

		float radiusToCheckForSmallerEnemies{};
		dataAvailable = pBlackboard->GetData("radiusToCheckForSmallerEnemies", radiusToCheckForSmallerEnemies);

		if (!dataAvailable)
			return; // safety checks
		radiusToCheckForSmallerEnemies += pSmartAgent->GetRadius(); // take our own radius into account
#pragma endregion

#pragma region Checking For Smaller Enemies
		const Elite::Vector2 positionOfSmartAgent{ pSmartAgent->GetPosition() };
		const float radiusToCheckSquared{ radiusToCheckForSmallerEnemies * radiusToCheckForSmallerEnemies };
		const float smartAgentRadius{ pSmartAgent->GetRadius() };
		AgarioAgent* pClosestAgentToChase{}; // this is the agent that will be passed along
		float shortestDistanceToAgentToChase{ FLT_MAX }; // set to FLT_MAX so we can compare distances to it
		for (AgarioAgent* pDumbAgent : *pDumbAgents)
		{
			if (pDumbAgent->CanBeDestroyed())
				continue; // if agent is marked for destruction, skip it, because it would lead to nullptr errors

			const Elite::Vector2 positionOfDumbAgent{ pDumbAgent->GetPosition() };
			const float distanceToDumbAgentSquared{ Elite::DistanceSquared(positionOfSmartAgent,positionOfDumbAgent) };
			if (distanceToDumbAgentSquared <= radiusToCheckSquared)
			{
				// an enemy is in the radius to check
				const float radiusOfDumbAgent{ pDumbAgent->GetRadius() };
				const float minimumSizeOfEnemy{ smartAgentRadius * 0.333333f };
				if (radiusOfDumbAgent > minimumSizeOfEnemy)
				{
					// the agent is at least 1/3 of the smart agent
					if (radiusOfDumbAgent + 1 < smartAgentRadius)
					{
						// the dumb agent is smaller than the smart agent
						if (distanceToDumbAgentSquared < shortestDistanceToAgentToChase)
						{
							// it is the closest to the smart agent
							shortestDistanceToAgentToChase = distanceToDumbAgentSquared;
							pClosestAgentToChase = pDumbAgent;
						}
					}
				}
			}
		}
#pragma endregion

		// IGNORE WARNING: TRANSITIONS ASSURE THAT THERE IS AN ENEMY
		pSmartAgent->SetToSeek(pClosestAgentToChase->GetPosition());
	}
private:
};
// ============
// TRANSITIONS
// ============

//			<=>------=>WANDER<=------<=>
//			|			  ^			   |
//			|			  |			   |
//			|			  v			   |
//			|		=> SEEKFOOD <=	   |
//			v		|			 |	   v
// EvadeLargeEnemy <=------------=> ChaseSmallerEnemy

// PRIORITY LIST:
// 1: Evade Larger Enemy (SURVIVAL)
// 2: if the player is small, seek food
// 3: if the player is large enough, seek out smaller players

class IsFoodNearby : public Elite::FSMTransition
{
public:
	virtual bool ToTransition(Blackboard* pBlackboard) const override
	{
#pragma region InitializingBlackboard
		AgarioAgent* pSmartAgent{};
		bool dataAvailable{ pBlackboard->GetData("SmartAgent",pSmartAgent) };

		if (!dataAvailable || pSmartAgent == nullptr)
			return false; // safety check, otherwise we get nullptr errors

		std::vector<AgarioFood*>* pMeals{};
		dataAvailable = pBlackboard->GetData("FoodVector", pMeals);

		if (!dataAvailable || pMeals->empty())
			return false; // if there is no food, we can return false

		float radiusToCheckForFood{};
		dataAvailable = pBlackboard->GetData("RadiusToCheckForFood", radiusToCheckForFood);

		if (!dataAvailable)
			return false; // safety check, if radius == 0.f, we won't find much food
		radiusToCheckForFood += pSmartAgent->GetRadius(); // we need to take our own radius into account
#pragma endregion

#pragma region CheckingForFood
		// squared radius since we use squared distance
		const float radiusToCheckForFoodSquared{ radiusToCheckForFood * radiusToCheckForFood };
		const Elite::Vector2 smartAgentPosition{ pSmartAgent->GetPosition() };
		Elite::Vector2 positionOfClosestFood{};
		float squaredDistanceToClosestFood{ FLT_MAX }; // initialize it as max, so we can do < checks with the distance
		for (AgarioFood* pFood : *pMeals)
		{
			if (pFood->CanBeDestroyed())
				continue; // if the food is marked for destruction, don't use it, because it would cause nullptr errors

			const Elite::Vector2 foodPosition{ pFood->GetPosition() };
			// use squared distance since it is a cheaper function than Distance()
			const float distanceToFoodSquared{ Elite::DistanceSquared(smartAgentPosition,foodPosition) };
			if (distanceToFoodSquared <= radiusToCheckForFoodSquared)
			{
				return true;
			}
		}

		return false; // no food was found
#pragma endregion
	}
private:
};
class IsNoFoodNearby final : public IsFoodNearby
{
public:
	virtual bool ToTransition(Blackboard* pBlackboard) const override
	{
		return !IsFoodNearby::ToTransition(pBlackboard);
	}
private:
};

class IsALargerEnemyNearby : public Elite::FSMTransition
{
public:
	virtual bool ToTransition(Blackboard* pBlackboard) const override
	{
#pragma region Initializing Blackboard
		AgarioAgent* pSmartAgent{};
		bool dataAvailable{ pBlackboard->GetData("SmartAgent",pSmartAgent) };

		if (!dataAvailable || pSmartAgent == nullptr)
			return false; // safety check for nullptr

		std::vector<AgarioAgent*>* pDumbAgents{};
		dataAvailable = pBlackboard->GetData("DumbAgents", pDumbAgents);

		if (!dataAvailable || pDumbAgents->empty())
			return false; // safety check + if there are no agents, there can't be any larger enemies

		float radiusToCheckForLargerEnemies{};
		dataAvailable = pBlackboard->GetData("radiusToCheckForLargerEnemies", radiusToCheckForLargerEnemies);

		if (!dataAvailable)
			return false; // safety check + if radius is 0.f, we won't find many enemies
		radiusToCheckForLargerEnemies += pSmartAgent->GetRadius(); // take our own radius into account
#pragma endregion

#pragma region Checking For Enemies
		const Elite::Vector2 positionOfSmartAgent{ pSmartAgent->GetPosition() };
		const float radiusOfSmartAgent{ pSmartAgent->GetRadius() };
		const float totalRadiusToCheckForEnemies{ radiusOfSmartAgent + radiusToCheckForLargerEnemies };
		AgarioAgent* pLargerEnemyToRunAwayFrom{};
		float shortestDistanceSoFar{ FLT_MAX }; // start at float max, so we can decrease it with checks
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
				if (Elite::IsSegmentIntersectingWithCircle(positionOfDumbAgent, edgeOfRadius, positionOfSmartAgent, totalRadiusToCheckForEnemies))
				{
					if (radiusOfDumbAgent > radiusOfSmartAgent + 1)
					{
						return true;
					}
				}
			}
		}

		return false;
#pragma endregion
	}
private:
};
class IsNoLargerEnemyNearby final : public IsALargerEnemyNearby
{
public:
	virtual bool ToTransition(Blackboard* pBlackboard) const override
	{
		return !IsALargerEnemyNearby::ToTransition(pBlackboard);
	}
private:
};

class IsASmallerEnemyNearby : public Elite::FSMTransition
{
public:
	virtual bool ToTransition(Blackboard* pBlackboard) const override
	{
#pragma region Getting Information From Blackboard
		AgarioAgent* pSmartAgent{};
		bool dataAvailable{ pBlackboard->GetData("SmartAgent",pSmartAgent) };

		if (!dataAvailable || pSmartAgent == nullptr)
			return false; // safety checks

		std::vector<AgarioAgent*>* pDumbAgents{};
		dataAvailable = pBlackboard->GetData("DumbAgents", pDumbAgents);

		if (!dataAvailable || pDumbAgents->empty())
			return false; // safety check + there can't be smaller enemies if there are no enemies

		float radiusToCheckForSmallerEnemies{};
		dataAvailable = pBlackboard->GetData("radiusToCheckForSmallerEnemies", radiusToCheckForSmallerEnemies);

		if (!dataAvailable)
			return false; // safety checks
		const float smartAgentRadius{ pSmartAgent->GetRadius() };
		radiusToCheckForSmallerEnemies += smartAgentRadius; // take our own radius into account
#pragma endregion

#pragma region Checking For Smaller Enemies
		if (smartAgentRadius < 6)
			return false; // first go for food before going for enemies


		const Elite::Vector2 positionOfSmartAgent{ pSmartAgent->GetPosition() };
		const float radiusToCheckSquared{ radiusToCheckForSmallerEnemies * radiusToCheckForSmallerEnemies };
		AgarioAgent* pClosestAgentToChase{}; // this is the agent that will be passed along
		float shortestDistanceToAgentToChase{ FLT_MAX }; // set to FLT_MAX so we can compare distances to it
		for (AgarioAgent* pDumbAgent : *pDumbAgents)
		{
			if (pDumbAgent->CanBeDestroyed())
				continue; // if agent is marked for destruction, skip it, because it would lead to nullptr errors

			const Elite::Vector2 positionOfDumbAgent{ pDumbAgent->GetPosition() };
			const float distanceToDumbAgentSquared{ Elite::DistanceSquared(positionOfSmartAgent,positionOfDumbAgent) };
			if (distanceToDumbAgentSquared <= radiusToCheckSquared)
			{
				// an enemy is in the radius to check
				const float radiusOfDumbAgent{ pDumbAgent->GetRadius() };
				const float minimumSizeOfEnemy{ smartAgentRadius * 0.333333f };
				if (radiusOfDumbAgent > minimumSizeOfEnemy)
				{
					// the agent is at least 1/3 of the smart agent
					if (radiusOfDumbAgent + 1 < smartAgentRadius)
					{
						return true;
					}
				}
			}
		}

		return false;
#pragma endregion
	}
private:
};
class IsNoSmallerEnemyNearby final : public IsASmallerEnemyNearby
{
public:
	virtual bool ToTransition(Blackboard* pBlackboard) const override
	{
		return !IsASmallerEnemyNearby::ToTransition(pBlackboard);
	}
private:

};
#endif