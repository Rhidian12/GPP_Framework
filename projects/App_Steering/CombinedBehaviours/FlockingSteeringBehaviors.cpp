#include "stdafx.h"
#include "FlockingSteeringBehaviors.h"
#include "TheFlock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"
#include "SpacePartitioning.h"
//*********************
//FLOCKSTEERINGBEHAVIOUR (BASE CLASS)
FlockSteeringBehaviour::FlockSteeringBehaviour(Flock* pFlock)
	: m_pFlock{ pFlock }
{

}
//*********************
//SEPARATION (FLOCKING)
SeperationFlocking::SeperationFlocking(Flock* pFlock)
	:Flee()
	, m_pFlock{ pFlock }
{

}
SteeringOutput SeperationFlocking::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	if (m_pFlock->GetIsSpatialPartitioningActive())
	{
		const int nrOfNeighbours{ m_pFlock->GetCellSpace()->GetNrOfNeighbors() };
		const std::vector<SteeringAgent*> neighbours{ m_pFlock->GetCellSpace()->GetNeighbors() };
		for (int i{}; i < nrOfNeighbours; ++i)
		{
			if (neighbours[i] != nullptr && neighbours[i] != pAgent)
			{
				const Elite::Vector2 neighbourPosition{ neighbours[i]->GetPosition() };
				// Do pow -1, then times speed
				const float distanceToNeigbour{ Distance(neighbourPosition,pAgent->GetPosition()) };

				m_Target.Position = neighbourPosition;
				steering.LinearVelocity += Flee::CalculateSteering(deltaT, pAgent).LinearVelocity.GetNormalized() *
					float(pow(distanceToNeigbour, -1)) * pAgent->GetMaxLinearSpeed();

				/*if (pAgent->CanRenderBehavior())
				{
					DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5.f, { 1,0,0 });
				}*/
			}
		}
	}
	else
	{
		const int nrOfNeighbours{ m_pFlock->GetNrOfNeighbors() };
		const std::vector<SteeringAgent*> neighbours{ m_pFlock->GetNeighbors() };
		for (int i{}; i < nrOfNeighbours; ++i)
		{
			if (neighbours[i] != nullptr && neighbours[i] != pAgent)
			{
				const Elite::Vector2 neighbourPosition{ neighbours[i]->GetPosition() };
				// Do pow -1, then times speed
				const float distanceToNeigbour{ Distance(neighbourPosition,pAgent->GetPosition()) };

				m_Target.Position = neighbourPosition;
				steering.LinearVelocity += Flee::CalculateSteering(deltaT, pAgent).LinearVelocity.GetNormalized() *
					pow(distanceToNeigbour, -1) * pAgent->GetMaxLinearSpeed();

				/*if (pAgent->CanRenderBehavior())
				{
					DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5.f, { 1,0,0 });
				}*/
			}
		}
	}
	return steering;
}

//*******************
//COHESION (FLOCKING)
Cohesion::Cohesion(Flock* pFlock)
	:FlockSteeringBehaviour(pFlock)
{

}
SteeringOutput Cohesion::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	steering.LinearVelocity = m_pFlock->GetAverageNeighborPos() - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	return steering;
}

//*************************
//VELOCITY MATCH (FLOCKING)
Alignment::Alignment(Flock* pFlock)
	:Seek{}
	, m_pFlock{ pFlock }
{

}
SteeringOutput Alignment::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	if (m_pFlock != nullptr)
	{
		m_Target.Position = m_pFlock->GetAverageNeighborVelocity();
		return Seek::CalculateSteering(deltaT, pAgent);
	}

	return Seek::CalculateSteering(deltaT, pAgent);
}

RunAway::RunAway(SteeringAgent* pAgentToAvoid)
	: Flee{}
	, m_pAgentToAvoid{ pAgentToAvoid }
{
}
SteeringOutput RunAway::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	const Elite::Vector2 positionOfAgentToAvoid{ m_pAgentToAvoid->GetPosition() };
	const float distanceToTargetSquared{ Elite::DistanceSquared(pAgent->GetPosition(),positionOfAgentToAvoid) };
	const float evadeRadius{ 25.f };
	SteeringOutput steering{};
	if (distanceToTargetSquared > evadeRadius * evadeRadius)
	{
		steering.IsValid = false;
		return steering;
	}
	if (m_pAgentToAvoid != nullptr)
	{
		SteeringOutput steering{};

		const float distanceToTarget{ Elite::Distance(pAgent->GetPosition(),positionOfAgentToAvoid) };

		m_Target.Position = positionOfAgentToAvoid;
		steering.LinearVelocity += Flee::CalculateSteering(deltaT, pAgent).LinearVelocity.GetNormalized() *
			float(pow(distanceToTarget, -1)) * pAgent->GetMaxLinearSpeed();

		if (pAgent->CanRenderBehavior())
		{
			DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), pAgent->GetRadius(), { 1,0,1 }, 0);
			DEBUGRENDERER2D->DrawCircle(m_pAgentToAvoid->GetPosition(), evadeRadius, { 1,0,1 }, 0);
		}
		return Flee::CalculateSteering(deltaT, pAgent);
	}
	return Flee::CalculateSteering(deltaT, pAgent);
}