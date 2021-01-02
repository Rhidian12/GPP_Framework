#pragma once
#include "../SteeringBehaviors.h"
class Flock;

class FlockSteeringBehaviour : public ISteeringBehavior
{
public:
	FlockSteeringBehaviour(Flock* pFlock);
	virtual ~FlockSteeringBehaviour() = default;

	virtual SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) = 0;
	
protected:
	Flock* m_pFlock{};
};
//SEPARATION - FLOCKING
//*********************
class SeperationFlocking : public Flee
{
public:
	SeperationFlocking(Flock* pFlock);
	virtual ~SeperationFlocking() = default;

	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

private:
	Flock* m_pFlock;
};

//COHESION - FLOCKING
//*******************
class Cohesion : public FlockSteeringBehaviour
{
public:
	Cohesion(Flock* pFlock);
	virtual ~Cohesion() = default;

	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

private:
};

//VELOCITY MATCH - FLOCKING
//************************
class Alignment : public Seek
{
public:
	Alignment(Flock* pFlock);
	virtual ~Alignment() = default;

	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

private:
	Flock* m_pFlock;
};

class RunAway : public Flee
{
public:
	RunAway(SteeringAgent* pAgentToAvoid);
	virtual ~RunAway() = default;

	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
	
private:
	SteeringAgent* m_pAgentToAvoid;
};