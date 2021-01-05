#include "stdafx.h"
#include "MicroAIAgent.h"
#include "projects/App_Steering/SteeringBehaviors.h"

MicroAIAgent::MicroAIAgent(const Elite::Vector2& position)
	: SteeringAgent{}
{
	SetPosition(position);
	m_pWander = new Wander{};
	m_pFlee = new Flee{};
	m_pSeek = new Seek{};
}

MicroAIAgent::~MicroAIAgent()
{
	SAFE_DELETE(m_DecisionMaking);
	SAFE_DELETE(m_pWander);
	SAFE_DELETE(m_pFlee);
	SAFE_DELETE(m_pSeek);
}

void MicroAIAgent::UpdateDecisionMaking(float dt)
{
	if (m_DecisionMaking)
		m_DecisionMaking->Update(dt);
}
void MicroAIAgent::Update(float dt)
{
	SteeringAgent::Update(dt);
}

void MicroAIAgent::Render(float dt)
{
	SteeringAgent::Render(dt);
}

void MicroAIAgent::SetDecisionMaking(Elite::IDecisionMaking* decisionMakingStructure)
{
	m_DecisionMaking = decisionMakingStructure;
}
void MicroAIAgent::SetToWander()
{
	SteeringAgent::SetSteeringBehavior(m_pWander);
}
void MicroAIAgent::SetToSeek(const Elite::Vector2& target)
{
	m_pSeek->SetTarget(target);
	SteeringAgent::SetSteeringBehavior(m_pSeek);
}
void MicroAIAgent::SetToFlee(const Elite::Vector2& target)
{
	m_pFlee->SetTarget(target);
	SteeringAgent::SetSteeringBehavior(m_pFlee);
}

Elite::IDecisionMaking* MicroAIAgent::GetDecisionMaking() const
{
	return m_DecisionMaking;
}
