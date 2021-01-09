#include "stdafx.h"
#include "Jobs.h"
#include "Alien.h"
#include "CollisionFunctions.h"
JobState InvestigateArea(Elite::Blackboard* pAlienBlackboard)
{
	Elite::Vector2 investigationTarget{};
	Elite::Vector2 rotatedTarget{};
	Alien* pAlien{};
	bool hasRotated180Degrees{};
	if (!pAlienBlackboard->GetData("investigationTarget", investigationTarget) || !pAlienBlackboard->GetData("alien", pAlien)
		|| !pAlienBlackboard->GetData("rotatedTarget", rotatedTarget) || !pAlienBlackboard->GetData("hasRotated180Degrees", hasRotated180Degrees))
		return JobState::FAILURE;


	const Elite::Rect alien{ pAlien->GetPosition(),pAlien->GetRadius(),pAlien->GetRadius() };
	const float seekTargetSize{ 5.f };
	const Elite::Rect seekTarget{ investigationTarget,seekTargetSize,seekTargetSize };
	if (Elite::IsOverlapping(alien, seekTarget))
	{
		// alien has reached the area to investigate
		pAlienBlackboard->ChangeData("hasReachedInvestigationTarget", true);

		if (rotatedTarget == Elite::Vector2{}) // set rotatedTarget if it hasn't been set yet
		{
			const float radiusAway{ 5.f };
			rotatedTarget = Elite::Vector2{ pAlien->GetPosition().x + cos(float(E_PI)) * radiusAway, pAlien->GetPosition().y + sin(float(E_PI)) * radiusAway };
			pAlienBlackboard->ChangeData("rotatedTarget", rotatedTarget);
		}

		pAlien->SetAutoOrient(false);

		const Elite::Vector2 toTarget{ rotatedTarget - pAlien->GetPosition() };

		const float to{ atan2f(toTarget.y, toTarget.x) + float(E_PI_2) };
		float from{ pAlien->GetOrientation() };
		from = atan2(sin(from), cos(from));
		float desired = to - from;

		const float Pi2 = float(E_PI) * 2.f;
		if (desired > E_PI)
			desired -= Pi2;
		else if (desired < -E_PI)
			desired += Pi2;

		if (Elite::AreEqual(desired, 0.f, 0.01f))
		{
			// target angle has been reached
			if (hasRotated180Degrees)
			{
				// it already rotated 180 degrees once, so this job is complete
				pAlien->SetAutoOrient(true);
				pAlienBlackboard->ChangeData("alien", pAlien);
				pAlienBlackboard->ChangeData("hasRotated180Degrees", false);
				pAlienBlackboard->ChangeData("rotatedTarget", Elite::Vector2{});
				pAlienBlackboard->ChangeData("investigationTarget", Elite::Vector2{});
				pAlienBlackboard->ChangeData("hasReachedInvestigationTarget", false);
				return JobState::COMPLETE;
			}
			else
			{
				// it hasn't rotated 360 degrees yet, so do this once more
				pAlienBlackboard->ChangeData("hasRotated180Degrees", true);
				pAlienBlackboard->ChangeData("rotatedTarget", Elite::Vector2{});
				return JobState::RUNNING;
			}
		}
		else
		{
			pAlien->SetToFace(rotatedTarget);
			pAlienBlackboard->ChangeData("alien", pAlien);
			return JobState::RUNNING;
		}
	}
	else
	{
		pAlien->SetToSeek(investigationTarget);
		pAlienBlackboard->ChangeData("alien", pAlien);
		return JobState::RUNNING;
	}
}
JobState Cooldown(Elite::Blackboard* pAlienBlackboard)
{
	Alien* pAlien{};
	float cooldownTimer{};
	if (!pAlienBlackboard->GetData("alien", pAlien) || !pAlienBlackboard->GetData("cooldownTimer", cooldownTimer))
		return JobState::FAILURE;

	if (cooldownTimer <= 5.f)
	{
		pAlienBlackboard->ChangeData("isCooldownActive", true);
		return JobState::RUNNING;
	}
	else
	{
		pAlienBlackboard->ChangeData("isCooldownActive", false);
		pAlienBlackboard->ChangeData("cooldownTimer", 0.f);
		return JobState::COMPLETE;
	}
}