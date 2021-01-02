#include "stdafx.h"
#include "MicroAIAgent.h"

MicroAIAgent::MicroAIAgent(const Elite::Vector2& position)
{
}

MicroAIAgent::~MicroAIAgent()
{
	SAFE_DELETE(m_DecisionMaking);
}

void MicroAIAgent::Update(float dt)
{
	if (m_DecisionMaking)
		m_DecisionMaking->Update(dt);

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
