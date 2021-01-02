//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "SteeringAgent.h"

//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	pAgent->SetAutoOrient(true);
	SteeringOutput steering{};

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	// DEBUG RENDERING
	/*if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 }, 0.4f);
	}*/
	return steering;
}

//WANDER (base> SEEK)
//******
SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	pAgent->SetAutoOrient(true);

	const Elite::Vector2 circleCentre{ pAgent->GetPosition() + (pAgent->GetDirection() * m_Offset) };
	const float angleChange = m_AngleChange * (randomFloat(2.f) - 1.f);
	m_WanderAngle += angleChange;

	const Elite::Vector2 centreToTarget{ m_Radius * cos(m_WanderAngle), m_Radius * sin(m_WanderAngle) };

	m_Target.Position = circleCentre + centreToTarget;

	// DEBUG RENDERING
	/*if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawCircle(circleCentre, m_Radius, { 0.f, 1.f, 0.f }, 0.4f);
		DEBUGRENDERER2D->DrawPoint(circleCentre + centreToTarget, 5.f, { 1.f, 0.f, 0.f }, 0.3f);
	}*/

	return Seek::CalculateSteering(deltaT, pAgent);
}
//FLEE (base> Seek)
//*******
SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{ Seek::CalculateSteering(deltaT, pAgent) };

	steering.LinearVelocity = -steering.LinearVelocity;

	return steering;
}
// ARRIVE (base > Seek)
//*********
SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	pAgent->SetAutoOrient(true);

	SteeringOutput steering{};

	const float distanceToTarget{ float(sqrt(pow((m_Target.Position.x - pAgent->GetPosition().x),2) + pow((m_Target.Position.y - pAgent->GetPosition().y),2))) };
	const float slowingRadius{ 20.f };

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();

	if (distanceToTarget <= slowingRadius)
	{
		// start slowing down
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed() * (distanceToTarget / slowingRadius);
	}
	else
	{
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
	}

	return steering;
}
// FACE (base > Seek)
//*******
SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	const float targetAngle{ Elite::GetOrientationFromVelocity(steering.LinearVelocity) };

	steering.LinearVelocity *= 0;

	pAgent->SetAutoOrient(false);

	float targetAngleInDegrees{ abs(targetAngle * 180.f / float(M_PI)) };

	if (m_Target.Position.x < pAgent->GetPosition().x)
	{
		targetAngleInDegrees = 360 - targetAngleInDegrees;
	}

	const float agentAngle{ abs(float(int(pAgent->GetRotation() * 180 / float(M_PI)) % 360)) };

	steering.AngularVelocity = targetAngleInDegrees - agentAngle;

	return steering;
}
// PURSUIT (base > Seek)	
//**********
Pursuit::Pursuit(SteeringAgent* pTarget)
	: m_pTarget{ pTarget }
{

}
SteeringOutput Pursuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	if (m_pTarget != nullptr)
	{
		SteeringOutput steering{};

		// don't use sqrt() because it is pretty expensive, doing pow() everywhere fixes this
		const float distanceToEnemy{ float(pow((m_pTarget->GetPosition().x - pAgent->GetPosition().x),2) + pow((m_pTarget->GetPosition().y - pAgent->GetPosition().y),2)) };
		const float timeToEnemy{ float(pow(pAgent->GetMaxLinearSpeed(),2)) };
		const float percentageOfWayTravelled{ (distanceToEnemy / timeToEnemy) * 100 };
		float timeToPredict{};
		if (timeToEnemy >= percentageOfWayTravelled)
		{
			timeToPredict = 5.f;
		}
		else if (timeToEnemy >= percentageOfWayTravelled * 0.8f)
		{
			timeToPredict = 3.f;
		}
		else if (timeToEnemy >= percentageOfWayTravelled * 0.5f)
		{
			timeToPredict = 2.f;
		}
		else if (timeToEnemy >= percentageOfWayTravelled * 0.25f)
		{
			timeToPredict = 1.5f;
		}
		else
		{
			timeToPredict = 1.f;
		}
		Elite::Vector2 projectedPosition{ m_pTarget->GetPosition() };
		projectedPosition += m_pTarget->GetLinearVelocity() * timeToPredict;

		steering.LinearVelocity = projectedPosition - pAgent->GetPosition();
		steering.LinearVelocity.Normalize();
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

		// DEBUG RENDERING
		/*if (pAgent->CanRenderBehavior())
		{
			DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 }, 0.4f);
		}*/
		return steering;
	}
	return Seek::CalculateSteering(deltaT, pAgent);
}

// EVADE (base > Pursuit)
//********
Evade::Evade(SteeringAgent* pTarget)
	:Pursuit{ pTarget }
{

}
SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	/*float distanceToTarget{ Distance(pAgent->GetPosition(),m_Target.Position) };

	if (distanceToTarget > m_EvadeRadius)
	{
		SteeringOutput steering{};
		steering.IsValid = false;
		return steering;
	}*/

	SteeringOutput steering{ Pursuit::CalculateSteering(deltaT, pAgent) };

	steering.LinearVelocity *= -1;

	return steering;
}
// SEPERATION
Seperation::~Seperation()
{
	/*for (SteeringAgent* pAgent : m_pAgents)
	{
		delete pAgent;
		pAgent = nullptr;
	}*/
}
void Seperation::SetAgents(const std::vector<SteeringAgent*>& pAgents)
{
	m_pAgents = pAgents;
}
SteeringOutput Seperation::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	for (size_t i{}; i < m_pAgents.size(); ++i)
	{
		if (m_pAgents[i] != nullptr && m_pAgents[i] != pAgent)
		{
			const Elite::Vector2 agentPosition{ m_pAgents[i]->GetPosition() };
			// Do pow -1, then times speed
			const float distanceToAgent{ Distance(agentPosition,pAgent->GetPosition()) };

			m_Target.Position = agentPosition;
			steering.LinearVelocity += Flee::CalculateSteering(deltaT, pAgent).LinearVelocity.GetNormalized() *
				float(pow(distanceToAgent, -1)) * pAgent->GetMaxLinearSpeed();

			/*if (pAgent->CanRenderBehavior())
			{
				DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5.f, { 1,0,0 });
			}*/
		}
	}
	return steering;
}