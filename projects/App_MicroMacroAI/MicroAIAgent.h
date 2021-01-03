#pragma once
#include "../App_Steering/SteeringAgent.h"

class Wander;
class Flee;
class Seek;

class MicroAIAgent final : public SteeringAgent
{
public:
	MicroAIAgent(const Elite::Vector2& position);
	~MicroAIAgent();

	//--- Agent Functions ---
	virtual void Update(float dt) override;
	void UpdateDecisionMaking(float dt);
	virtual void Render(float dt) override;

	//-- AI Functions --
	void SetDecisionMaking(Elite::IDecisionMaking* decisionMakingStructure);
	void SetToWander();
	void SetToSeek(const Elite::Vector2& target);
	void SetToFlee(const Elite::Vector2& target);
	
	Elite::IDecisionMaking* GetFSM() const;

private:
	Elite::IDecisionMaking* m_DecisionMaking = nullptr;

	Wander* m_pWander = nullptr;
	Flee* m_pFlee = nullptr;
	Seek* m_pSeek = nullptr;
};