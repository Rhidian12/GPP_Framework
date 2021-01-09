/*=============================================================================*/
// Copyright 2017-2018 Elite Engine
// Authors: Matthieu Delaere, Thomas Goussaert
/*=============================================================================*/
// SteeringBehaviors.h: SteeringBehaviors interface and different implementations
/*=============================================================================*/
#ifndef ELITE_STEERINGBEHAVIORS
#define ELITE_STEERINGBEHAVIORS

//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "SteeringHelpers.h"
class SteeringAgent;
using namespace Elite;

#pragma region **ISTEERINGBEHAVIOR** (BASE)
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	virtual SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) = 0;

	//Seek Functions
	void SetTarget(TargetData target) { m_Target = target; }

	template<class T, typename std::enable_if<std::is_base_of<ISteeringBehavior, T>::value>::type * = nullptr>
	T* As()
	{
		return static_cast<T*>(this);
	}

protected:
	TargetData m_Target;
};
#pragma endregion

///////////////////////////////////////
//SEEK
//****
class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() = default;

	//Seek Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

	//virtual void SetTarget(const TargetData pTarget) { m_Target = pTarget; }
protected:
	//const TargetData m_pTargetRef{};
};

//////////////////////////
//WANDER
//******
class Wander : public Seek
{
public:
	Wander() = default;
	virtual ~Wander() = default;

	//Wander Behavior
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

	void SetWanderOffset(float offset) { m_Offset = offset; }
	void SetWanderRadius(float radius) { m_Radius = radius; }
	void SetMaxAngleChange(float rad) { m_AngleChange = rad; }

protected:
	float m_Offset{ 6.f }; // offset for direction
	float m_Radius{ 4.f }; // wander Radius
	float m_AngleChange{ ToRadians(45) }; // max angle change per frame
	float m_WanderAngle{ 0.f };

private:
	//void SetTarget(const TargetData& pTarget) override {};
};
//////////////////////////
//Flee
//******
class Flee : public Seek
{
public:
	Flee() = default;
	virtual ~Flee() = default;

	//Wander Behavior
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
};
class Arrive : public Seek
{
public:
	Arrive() = default;
	virtual ~Arrive() = default;

	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
};
class Face : public Seek
{
public:
	Face() = default;
	virtual ~Face() = default;

	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
};
class Pursuit : public Seek
{
public:
	Pursuit(SteeringAgent* pTarget);
	virtual ~Pursuit() = default;

	void SetAgentTarget(SteeringAgent* pAgent);

	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
protected:
	SteeringAgent* m_pTarget;
};
class Evade : public Pursuit
{
public:
	Evade(SteeringAgent* pTarget);
	virtual ~Evade() = default;

	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
private:
	float m_EvadeRadius{ 15.f };
};
class Seperation : public Flee
{
public:
	Seperation() = default;
	virtual ~Seperation();

	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
	void SetAgents(const std::vector<SteeringAgent*>& pAgents);

private:
	std::vector<SteeringAgent*> m_pAgents;
};
#endif