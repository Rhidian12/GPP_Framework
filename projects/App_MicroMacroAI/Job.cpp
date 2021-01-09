#include "stdafx.h"
#include "Job.h"

Job::Job(std::function<JobState(Elite::Blackboard* pAlienBlackboard)> pJobFunction, const JobPriority& jobPriority, const JobType& jobType)
	: m_pJobFunction{ pJobFunction }
	, m_JobPriority{ jobPriority }
	, m_JobType{ jobType }
{
}

const JobState Job::ExecuteJob(Elite::Blackboard* pBlackboard)
{
	return m_pJobFunction(pBlackboard);
}

const JobPriority Job::GetJobPriority() const
{
	return m_JobPriority;
}
const JobType Job::GetJobType() const
{
	return m_JobType;
}
