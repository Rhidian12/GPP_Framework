#pragma once
#include "../App_Steering/SteeringAgent.h"

class MicroAIAgent final : public SteeringAgent
{
public:
	MicroAIAgent(const Elite::Vector2& position);
	~MicroAIAgent();

	//--- Agent Functions ---
	virtual void Update(float dt) override;
	virtual void Render(float dt) override;

	//-- Agario Functions --
	void SetDecisionMaking(Elite::IDecisionMaking* decisionMakingStructure);

private:
	Elite::IDecisionMaking* m_DecisionMaking = nullptr;
	bool m_ToDestroy = false;
	float m_SpeedBase = 25.f;
};