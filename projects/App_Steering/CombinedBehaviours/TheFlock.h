#pragma once
#include "../SteeringHelpers.h"
#include "FlockingSteeringBehaviors.h"

class ISteeringBehavior;
class SteeringAgent;
class BlendedSteering;
class PrioritySteering;
class Cohesion;
class SeperationFlocking;
class Alignment;
class Evade;
class RunAway;
class CellSpace;

class Flock
{
public:
	Flock(
		int flockSize = 50, 
		float worldSize = 100.f, 
		SteeringAgent* pAgentToEvade = nullptr, 
		bool trimWorld = false);

	~Flock();

	void Update(float deltaT);
	void UpdateAndRenderUI();
	void Render(float deltaT);

	void RegisterNeighbors(SteeringAgent* pAgent);
	int GetNrOfNeighbors() const { return m_NrOfNeighbors; }
	const vector<SteeringAgent*>& GetNeighbors() const { return m_Neighbors; }

	Elite::Vector2 GetAverageNeighborPos() const;
	Elite::Vector2 GetAverageNeighborVelocity() const;

	void SetSeekTarget(const Elite::Vector2& position);
	bool GetCanRenderBehaviour() const;
	const std::vector<SteeringAgent*>& GetAgents() const;
	float GetNeighbourhoodRadius() const;
	bool GetIsSpatialPartitioningActive() const;
	const CellSpace* GetCellSpace() const;

private:
	// flock agents
	int m_FlockSize = 0;
	vector<SteeringAgent*> m_Agents;

	// neighborhood agents
	vector<SteeringAgent*> m_Neighbors;
	float m_NeighborhoodRadius = 15.f;
	int m_NrOfNeighbors = 0;
	int m_MaxAmountOfNeighboursinRadius;

	// evade target
	SteeringAgent* m_pAgentToEvade = nullptr;

	// world info
	bool m_TrimWorld = false;
	float m_WorldSize = 0.f;
	
	// steering Behaviors
	BlendedSteering* m_pBlendedSteering = nullptr;
	PrioritySteering* m_pPrioritySteering = nullptr;

	Seek* m_pSeek{};
	Cohesion* m_pCohesion{};
	SeperationFlocking* m_pSeperation{};
	Alignment* m_pAlignment{};
	Wander* m_pWander{};
	RunAway* m_pRunAway{};

	bool m_CanDebugRender = false;

	// Spatial partitioning
	CellSpace* m_pCellSpace = nullptr;
	bool m_IsSpatialPartitioningActive;
	std::vector<Elite::Vector2> m_OldPositions;

	// private functions
	float* GetWeight(ISteeringBehavior* pBehaviour);

private:
	Flock(const Flock& other);
	Flock& operator=(const Flock& other);
};