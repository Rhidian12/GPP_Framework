#pragma once
#include "stdafx.h"
#include <functional>
class Blackboard;
enum class JobState
{
	FAILURE = 0,
	RUNNING = 1,
	COMPLETE = 2
};
enum class JobPriority
{
	PRIORITY = 0,
	NORMAL = 1
};
enum class JobType
{
	INVESTIGATE = 0,
};
class Job
{
public:
	explicit Job(std::function<JobState(Elite::Blackboard* pAlienBlackboard)> pJobFunction, const JobPriority& jobPriority, const JobType& jobType);
	~Job() = default;

	const JobState ExecuteJob(Elite::Blackboard* pBlackboard);
	const JobPriority GetJobPriority() const;
	const JobType GetJobType() const;

private:
	std::function<JobState(Elite::Blackboard* pAlienBlackboard)> m_pJobFunction;
	JobPriority m_JobPriority;
	JobType m_JobType;
};