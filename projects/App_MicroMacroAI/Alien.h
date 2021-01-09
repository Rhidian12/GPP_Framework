#pragma once
#include "MicroAIAgent.h"
#include <deque>
class Job;
class Alien final : public MicroAIAgent
{
public:
	Alien(const Elite::Vector2& position);
	virtual ~Alien();

	virtual void Update(float dt) override;
	virtual void UpdateDecisionMaking(float dt) override;
	virtual void Render(float dt) override;

	void AddJob(Job* pJob);

	std::deque<Job*>& GetJobs();

private:
	std::deque<Job*> m_pJobs;
};