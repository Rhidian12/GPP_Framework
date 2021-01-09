#pragma once
#include "../App_Steering/SteeringAgent.h"

class Wander;
class Flee;
class Seek;
class Face;
class Pursuit;

class MicroAIAgent : public SteeringAgent
{
public:
	MicroAIAgent(const Elite::Vector2& position);
	virtual ~MicroAIAgent();

	//--- Agent Functions ---
	virtual void Update(float dt) override;
	virtual void UpdateDecisionMaking(float dt);
	virtual void Render(float dt) override;

	//-- AI Functions --
	void SetDecisionMaking(Elite::IDecisionMaking* decisionMakingStructure);
	void SetToWander();
	void SetToSeek(const Elite::Vector2& target);
	void SetToFlee(const Elite::Vector2& target);
	void SetToFace(const Elite::Vector2& target);
	void SetToPursuit(SteeringAgent* pAgent);
	
	Elite::IDecisionMaking* GetDecisionMaking() const;

protected:
	Elite::IDecisionMaking* m_DecisionMaking = nullptr;

	Wander* m_pWander = nullptr;
	Flee* m_pFlee = nullptr;
	Seek* m_pSeek = nullptr;
	Face* m_pFace = nullptr;
	Pursuit* m_pPursuit = nullptr;
};