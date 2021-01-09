#include "stdafx.h"
#include "Alien.h"
#include "Job.h"
Alien::Alien(const Elite::Vector2& position)
	: MicroAIAgent{ position }
{
}
Alien::~Alien()
{
	for (auto pJob : m_pJobs)
		SAFE_DELETE(pJob);
}

void Alien::Update(float dt)
{
	SteeringAgent::Update(dt);
}

void Alien::UpdateDecisionMaking(float dt)
{
	if (m_DecisionMaking)
		m_DecisionMaking->Update(dt);
}

void Alien::Render(float dt)
{
	SteeringAgent::Render(dt);
}

void Alien::AddJob(Job* pJob)
{
	if (pJob->GetJobPriority() == JobPriority::NORMAL)
		m_pJobs.push_back(pJob);
	else
		m_pJobs.push_front(pJob);
}

std::deque<Job*>& Alien::GetJobs()
{
	return m_pJobs;
}