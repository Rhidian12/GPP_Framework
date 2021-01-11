// PCH
#include "stdafx.h"

// includes
#include "App_MicroMacroAI.h"
#include "projects/Shared/NavigationColliderElement.h"

#include "MicroAIAgent.h"
#include "Alien.h"

#include "../App_Steering/SteeringBehaviors.h"

#include "framework\EliteAI\EliteGraphs\ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

#include "framework/EliteAI/EliteData/EBlackboard.h"
#include "framework/EliteAI/EliteDecisionMaking/EDecisionMaking.h"
#include "framework/EliteAI/EliteDecisionMaking/EliteFiniteStateMachine/EFiniteStateMachine.h"
#include "Behaviours.h"
#include "AlienBehaviours.h"
#include "Jobs.h"

#include "CollisionFunctions.h"
#include "MacroAI.h"

//Statics
bool App_MicroMacroAI::sShowPolygon = false;
bool App_MicroMacroAI::sShowGraph = false;
bool App_MicroMacroAI::sDrawPortals = false;
bool App_MicroMacroAI::sDrawFinalPath = true;
bool App_MicroMacroAI::sDrawNonOptimisedPath = false;

App_MicroMacroAI::~App_MicroMacroAI()
{
	for (auto pNC : m_vNavigationColliders)
		SAFE_DELETE(pNC);
	m_vNavigationColliders.clear();

	SAFE_DELETE(m_pNavGraph);
	SAFE_DELETE(m_pAgent);
	SAFE_DELETE(m_pWander);
	SAFE_DELETE(m_pFlee);
	SAFE_DELETE(m_pSeek);
	SAFE_DELETE(m_pAlien);
	SAFE_DELETE(m_pMacroAI);

	for (auto pState : m_pPlayerStates)
		SAFE_DELETE(pState);
	m_pPlayerStates.clear();
	for (auto pTransition : m_pPlayerTransitions)
		SAFE_DELETE(pTransition);
	m_pPlayerTransitions.clear();

	for (auto pState : m_pAlienStates)
		SAFE_DELETE(pState);
	m_pAlienStates.clear();
	for (auto pTransition : m_pAlienTransitions)
		SAFE_DELETE(pTransition);
	m_pAlienTransitions.clear();
}

void App_MicroMacroAI::Start()
{
	//Initialization of your application. 
	//----------- CAMERA ------------
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(36.782f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(12.9361f, 0.2661f));

	//----------- WORLD ------------
	m_vNavigationColliders.push_back(new NavigationColliderElement{ Elite::Vector2{0.f,-90.f},170.f,1.f });
	m_vNavigationColliders.push_back(new NavigationColliderElement{ Elite::Vector2{0.f,90.f},170.f,1.f });
	m_vNavigationColliders.push_back(new NavigationColliderElement{ Elite::Vector2{88.f,0.f},1.f,175.f,90.f });
	m_vNavigationColliders.push_back(new NavigationColliderElement{ Elite::Vector2{-88.f,0.f},1.f,175.f,90.f });

	m_vNavigationColliders.push_back(new NavigationColliderElement{ Elite::Vector2{0.f,-70.f},130.f,1.f });
	m_vNavigationColliders.push_back(new NavigationColliderElement{ Elite::Vector2{-65.f,-42.f},1.f,50.f,90.f });
	m_vNavigationColliders.push_back(new NavigationColliderElement{ Elite::Vector2{65.f,-42.f},1.f,50.f,90.f });
	m_vNavigationColliders.push_back(new NavigationColliderElement{ Elite::Vector2{50.f,-13.f},30.f,1.f });
	m_vNavigationColliders.push_back(new NavigationColliderElement{ Elite::Vector2{65.f,30.f},1.f,50.f,90.f });
	m_vNavigationColliders.push_back(new NavigationColliderElement{ Elite::Vector2{-65.f,30.f},1.f,50.f,90.f });
	m_vNavigationColliders.push_back(new NavigationColliderElement{ Elite::Vector2{35.f,-28.f},1.f,20.f,90.f });
	m_vNavigationColliders.push_back(new NavigationColliderElement{ Elite::Vector2{-20.f,-35.f},60.f,1.f });
	m_vNavigationColliders.push_back(new NavigationColliderElement{ Elite::Vector2{0.f,-57.f},1.f,20.f,90.f });
	m_vNavigationColliders.push_back(new NavigationColliderElement{ Elite::Vector2{0.f,5.f},90.f,1.f });
	m_vNavigationColliders.push_back(new NavigationColliderElement{ Elite::Vector2{0.f,58.f},130.f,1.f });


	//----------- NAVMESH  ------------
	std::list<Elite::Vector2> baseBox
	{ { -100.f, 100.f },{ -100.f, -100.f },{ 100.f, -100.f },{ 100.f, 100.f } };

	m_pNavGraph = new Elite::NavGraph(Elite::Polygon(baseBox), m_AgentRadius);


	//----------- CREATE CHECKPOINTS ------------
	const int nrCheckpointsXAxis{ 5 };
	const int nrCheckpointsYAxis{ 10 };
	for (int y{ 0 }; y <= nrCheckpointsYAxis; ++y)
	{
		for (int x{ 0 }; x <= nrCheckpointsXAxis; ++x)
		{
			Elite::Vector2 position{ -80.f + (160.f / nrCheckpointsXAxis * x), -80.f + (160.f / nrCheckpointsYAxis * y) };
			m_Checkpoints.push_back(Checkpoint{ position,false });
		}
	}


	//----------- PLAYER ------------
	InitializePlayer();


	//----------- PLAYER ------------
	InitializeAlien();


	//----------- SPAWN PICKUPS ------------
	for (int i{}; i < m_MaxAmountOfPickups; ++i)
	{
		Elite::Vector2 randomPosition{};
		do
		{
			randomPosition = Elite::randomVector2(-85.f, 85.f);
		} while ((std::find_if(m_vNavigationColliders.begin(), m_vNavigationColliders.end(), [&randomPosition](const NavigationColliderElement* a)->bool
			{
				const Elite::Rect hitbox{ a->GetPosition(),a->GetWidth(),a->GetHeight() };
				const Elite::Rect pickupHitbox{ randomPosition,5.f,5.f };
				return Elite::IsOverlapping(hitbox, pickupHitbox);
			}) != m_vNavigationColliders.end()) || (std::find_if(m_Pickups.begin(), m_Pickups.end(), [&randomPosition](const Elite::Vector2& a)->bool
				{
					const Elite::Rect hitbox{ a,5.f,5.f };
					const Elite::Rect pickupHitbox{ randomPosition,5.f,5.f };
					return Elite::IsOverlapping(hitbox, pickupHitbox);
				}) != m_Pickups.end()));
		m_Pickups.push_back(randomPosition);
	}
	for (size_t i{}; i < m_Pickups.size(); ++i)
	{
		auto it = std::find_if(m_Pickups.begin(), m_Pickups.end(), [this, &i](const Elite::Vector2& a)->bool
			{
				return Elite::DistanceSquared(m_Pickups[i], a) <= Elite::Square(m_PlayerFOVRange + 5.f);
			});
		if (it != m_Pickups.end())
		{
			Elite::Vector2 randomPosition{};
			do
			{
				randomPosition = Elite::randomVector2(-85.f, 85.f);
			} while ((std::find_if(m_vNavigationColliders.begin(), m_vNavigationColliders.end(), [&randomPosition](const NavigationColliderElement* a)->bool
				{
					const Elite::Rect hitbox{ a->GetPosition(),a->GetWidth(),a->GetHeight() };
					const Elite::Rect pickupHitbox{ randomPosition,5.f,5.f };
					return Elite::IsOverlapping(hitbox, pickupHitbox);
				}) != m_vNavigationColliders.end()) || (std::find_if(m_Pickups.begin(), m_Pickups.end(), [&randomPosition, this](const Elite::Vector2& a)->bool
					{
						return Elite::DistanceSquared(randomPosition, a) <= Elite::Square(m_PlayerFOVRange + 5.f);
					}) != m_Pickups.end()));
			m_Pickups[i] = randomPosition;
		}
	}


	//----------- FOV ------------
	const float playerIncrement{ m_PlayerFOVAngle * 0.2f };
	m_PlayerFOVRaycasts.resize(unsigned int(m_PlayerFOVAngle / playerIncrement * 2) + 1);

	m_AlienFOVs.resize(m_AlienFOVAngles.size());
	m_AlienFOVsRaycasts.resize(m_AlienFOVAngles.size());
	for (size_t i{}; i < m_AlienFOVsRaycasts.size(); ++i)
	{
		m_AlienFOVsRaycasts[i].resize(11);
	}


	//----------- MACRO AI ------------
	m_pMacroAI = new MacroAI{};
}
void App_MicroMacroAI::InitializePlayer()
{
	//----------- AGENT ------------
	m_pWander = new Wander{};
	m_pFlee = new Flee{};
	m_pSeek = new Seek{};
	m_Target = TargetData(Elite::ZeroVector2);
	m_pAgent = new MicroAIAgent{ {-85.f,-85.f} };
	m_pAgent->SetSteeringBehavior(m_pSeek);
	m_pAgent->SetMaxLinearSpeed(m_AgentSpeed);
	m_pAgent->SetAutoOrient(true);
	m_pAgent->SetMass(0.1f);
	m_pAgent->SetBodyColor(Elite::Color{ 0.f,0.f,255.f });


	//----------- Decision Making ------------
	Elite::Blackboard* pBlackboard{ new Elite::Blackboard{} };
	pBlackboard->AddData("target", Elite::Vector2{});
	pBlackboard->AddData("player", m_pAgent);
	pBlackboard->AddData("checkpoints", &m_Checkpoints);
	pBlackboard->AddData("pickupsInFOV", GetPickupsInPlayerFOV());
	pBlackboard->AddData("grabRange", m_GrabRange);
	pBlackboard->AddData("pickups", &m_Pickups);
	bool wasPickupSeen{};
	pBlackboard->AddData("wasPickupSeen", wasPickupSeen);

	PlayerWander* pWanderState{ new PlayerWander{} };
	FollowSearchPatternState* pFollowSearchPatternState{ new FollowSearchPatternState{} };
	SeekState* pSeekState{ new SeekState{} };
	PickupPickupState* pPickupPickupState{ new PickupPickupState{} };

	m_pPlayerStates.push_back(pWanderState);
	m_pPlayerStates.push_back(pFollowSearchPatternState);
	m_pPlayerStates.push_back(pSeekState);
	m_pPlayerStates.push_back(pPickupPickupState);

	HaveNotAllCheckpointsBeenVisited* pHaveNotAllCheckpointsBeenVisited{ new HaveNotAllCheckpointsBeenVisited{} };
	AreTherePickupsInFOV* pAreTherePickupsInFOV{ new AreTherePickupsInFOV{} };
	IsAgentInPickupRange* pIsAgentInPickupRange{ new IsAgentInPickupRange{} };

	m_pPlayerTransitions.push_back(pHaveNotAllCheckpointsBeenVisited);
	m_pPlayerTransitions.push_back(pAreTherePickupsInFOV);
	m_pPlayerTransitions.push_back(pIsAgentInPickupRange);

	FiniteStateMachine* pFSM{ new FiniteStateMachine{pWanderState,pBlackboard} };
	pFSM->AddTransition(pWanderState, pFollowSearchPatternState, pHaveNotAllCheckpointsBeenVisited);
	pFSM->AddTransition(pFollowSearchPatternState, pSeekState, pAreTherePickupsInFOV);
	pFSM->AddTransition(pSeekState, pPickupPickupState, pIsAgentInPickupRange);
	pFSM->AddTransition(pPickupPickupState, pFollowSearchPatternState, pHaveNotAllCheckpointsBeenVisited);

	m_pAgent->SetDecisionMaking(pFSM);
}
void App_MicroMacroAI::InitializeAlien()
{
	//----------- AGENT ------------
	m_Target = TargetData(Elite::ZeroVector2);
	m_pAlien = new Alien{ {0.f,0.f} };
	m_pAlien->SetSteeringBehavior(m_pSeek);
	m_pAlien->SetMaxLinearSpeed(m_AgentSpeed);
	m_pAlien->SetAutoOrient(true);
	m_pAlien->SetMass(0.1f);
	m_pAlien->SetBodyColor(Elite::Color{ 255.f,0.f,0.f });


	//----------- Decision Making ------------
	Elite::Blackboard* pBlackboard{ new Elite::Blackboard{} };
	pBlackboard->AddData("alienTarget", Elite::Vector2{});
	pBlackboard->AddData("alien", m_pAlien);
	pBlackboard->AddData("agent", m_pAgent);
	bool isPlayerInFOV{};
	pBlackboard->AddData("isPlayerInFOV", isPlayerInFOV);
	float isPlayerInFOVTimer{};
	pBlackboard->AddData("isPlayerInFOVTimer", isPlayerInFOVTimer);
	bool isPursuitAvailable{};
	pBlackboard->AddData("isPursuitAvailable", isPursuitAvailable);
	bool isHearingEnabled{};
	pBlackboard->AddData("isHearingEnabled", isHearingEnabled);
	pBlackboard->AddData("investigationTarget", Elite::Vector2{});
	pBlackboard->AddData("rotatedTarget", Elite::Vector2{});
	bool hasRotated180Degrees{};
	pBlackboard->AddData("hasRotated180Degrees", hasRotated180Degrees);
	pBlackboard->AddData("jobs", &m_pAlien->GetJobs());
	bool hasReachedInvestigationTarget{};
	pBlackboard->AddData("hasReachedInvestigationTarget", hasReachedInvestigationTarget);
	float cooldownTimer{};
	pBlackboard->AddData("cooldownTimer", cooldownTimer);
	bool isCooldownActive{};
	pBlackboard->AddData("isCooldownActive", isCooldownActive);


	Elite::BehaviorTree* pBHT{ new Elite::BehaviorTree
		{ pBlackboard,
			new BehaviorSelector{
				{
					new BehaviorSequence{ // == CHASE PLAYER ==
						{
							new BehaviorConditional{IsPlayerInFOV},
							new BehaviorSelector{
								{
									new BehaviorSequence{
										{
											new BehaviorConditional{IsPursuitAvailable},
											new BehaviorAction{PursuitPlayer}
										}},
									new BehaviorSequence{
										{
											new InvertedBehaviorConditional{IsPursuitAvailable},
											new BehaviorAction{ChasePlayer}
										}}
								}}
						}},
					new BehaviorSelector{ // == EXECUTE JOBS ==
						{
							new BehaviorSequence{
								{
									new BehaviorConditional{IsThereACurrentJob},
									new BehaviorAction{ExecuteFirstJob}
								}},
							new BehaviorSequence{
								{
									new InvertedBehaviorConditional{IsThereACurrentJob},
									new BehaviorAction{MakeJob}
								}}
						}},
					new BehaviorAction{WanderBehaviour}
				}}
		}
	};

	m_pAlien->SetDecisionMaking(pBHT);
}

bool App_MicroMacroAI::DetectAgentCollision()
{
	const Elite::Rect playerHitbox{ m_pAgent->GetPosition(),m_pAgent->GetRadius(),m_pAgent->GetRadius() };
	const Elite::Rect alienHitbox{ m_pAlien->GetPosition(),m_pAlien->GetRadius(),m_pAlien->GetRadius() };
	return Elite::IsOverlapping(playerHitbox, alienHitbox);
}

void App_MicroMacroAI::Update(float deltaTime)
{

	//----------- CHECK GAME OVER ------------
	m_IsGameOver = DetectAgentCollision();


	if (!m_IsGameOver)
	{
		//----------- GET PLAYER FINITE STATE MACHINE ------------
		Elite::FiniteStateMachine* pFSM{ dynamic_cast<FiniteStateMachine*>(m_pAgent->GetDecisionMaking()) };


		//----------- GET ALIEN BEHAVIOUR TREE ------------
		Elite::BehaviorTree* pBHT{ dynamic_cast<BehaviorTree*>(m_pAlien->GetDecisionMaking()) };


		//----------- UPDATE BLACKBOARD ------------
		pFSM->GetBlackboard()->ChangeData("pickupsInFOV", GetPickupsInPlayerFOV());
		pBHT->GetBlackboard()->ChangeData("isPlayerInFOV", IsPlayerInAlienFOV(pBHT->GetBlackboard(), deltaTime));
		bool isCooldownActive{};
		pBHT->GetBlackboard()->GetData("isCooldownActive", isCooldownActive);
		if (isCooldownActive)
		{
			float cooldownTimer{};
			pBHT->GetBlackboard()->GetData("cooldownTimer", cooldownTimer);
			pBHT->GetBlackboard()->ChangeData("cooldownTimer", cooldownTimer + deltaTime);
		}


		//----------- UPDATE DECISION MAKING ------------
		m_pAgent->UpdateDecisionMaking(deltaTime);
		m_pAlien->UpdateDecisionMaking(deltaTime);


		//----------- UPDATE STEERING WITH NAVMESH ------------
		if (pFSM->GetCurrentState() == m_pPlayerStates[1])
		{
			Elite::Vector2 target{};
			pFSM->GetBlackboard()->GetData("target", target);
			m_AgentPath = FindPath(m_pAgent->GetPosition(), target, m_pAgent);

			//Check if a path exist and move to the following point
			if (m_AgentPath.size() > 0)
			{
				m_pAgent->SetToSeek(m_AgentPath[0]);

				if (Elite::DistanceSquared(m_pAgent->GetPosition(), m_AgentPath[0]) < m_AgentRadius * m_AgentRadius)
				{
					//If we reached the next point of the path. Remove it 
					m_AgentPath.erase(std::remove(m_AgentPath.begin(), m_AgentPath.end(), m_AgentPath[0]));
				}
			}
		}
		if (m_pAlien->GetJobs()[0]->GetJobType() == JobType::INVESTIGATE)
		{
			bool hasReachedInvestigationTarget{};
			pBHT->GetBlackboard()->GetData("hasReachedInvestigationTarget", hasReachedInvestigationTarget);
			if (!hasReachedInvestigationTarget)
			{
				Elite::Vector2 investigationTarget{};
				pBHT->GetBlackboard()->GetData("investigationTarget", investigationTarget);
				m_AlienPath = FindPath(m_pAlien->GetPosition(), investigationTarget, m_pAlien);

				//Check if a path exist and move to the following point
				if (m_AlienPath.size() > 0)
				{
					m_pAlien->SetToSeek(m_AlienPath[0]);

					if (Elite::DistanceSquared(m_pAlien->GetPosition(), m_AlienPath[0]) < m_AgentRadius * m_AgentRadius)
					{
						//If we reached the next point of the path. Remove it 
						m_AlienPath.erase(std::remove(m_AlienPath.begin(), m_AlienPath.end(), m_AlienPath[0]));
					}
				}
			}
		}


		//----------- UPDATE MACRO AI ------------
		m_pMacroAI->Update(m_Pickups, 10, pBHT->GetBlackboard(), Elite::Rect{ { -100.f,-100.f }, 200.f, 200.f });


		//----------- UPDATE AGENTS ------------
		m_pAgent->Update(deltaTime);
		m_pAlien->Update(deltaTime);

		//----------- UPDATE CHECKPOINTS ------------
		UpdateCheckpoints();


		//----------- UPDATE PLAYER FOV ------------
		UpdatePlayerFOV();


		//----------- UPDATE ALIEN FOV ------------
		UpdateAlienFOV();


		//----------- UPDATE IMGUI ------------
		UpdateImGui();
	}
	else
		UpdateEndgameImGui();
}
void App_MicroMacroAI::UpdateCheckpoints()
{
	const Elite::Rect playerHitbox{ m_pAgent->GetPosition(),m_pAgent->GetRadius(),m_pAgent->GetRadius() };
	for (auto& checkpoint : m_Checkpoints)
	{
		if (checkpoint.hasBeenVisited)
			continue;

		const Elite::Rect checkpointHitbox{ checkpoint.position,5.f,5.f };
		if (Elite::IsOverlapping(playerHitbox, checkpointHitbox))
		{
			checkpoint.hasBeenVisited = true;
			return;
		}
	}

	// All checkpoints have been visited, so time to reset them all
	auto it = std::find_if(m_Checkpoints.begin(), m_Checkpoints.end(), [](const Checkpoint& a)->bool
		{
			return !a.hasBeenVisited;
		});
	if (it == m_Checkpoints.end())
	{
		for (auto& checkpoint : m_Checkpoints)
			checkpoint.hasBeenVisited = false;
	}
}
void App_MicroMacroAI::UpdatePlayerFOV()
{
	const float c{ cos(m_PlayerFOVAngle) };
	const float s{ sin(m_PlayerFOVAngle) };
	const Elite::Vector2 orientationToVector{ Elite::OrientationToVector(m_pAgent->GetOrientation()) };

	const Elite::Vector2 pointOne{ m_pAgent->GetPosition() };
	const float newX{ orientationToVector.x * c - s * orientationToVector.y };
	const float newY{ orientationToVector.x * s + c * orientationToVector.y };
	const Elite::Vector2 rotatedLinearVelocity{ newX, newY };
	Elite::Vector2 pointTwo{ m_pAgent->GetPosition().x + rotatedLinearVelocity.GetNormalized().x * (m_pAgent->GetRadius() + m_PlayerFOVRange),
									m_pAgent->GetPosition().y + rotatedLinearVelocity.GetNormalized().y * (m_pAgent->GetRadius() + m_PlayerFOVRange) };

	const float newX2{ orientationToVector.x * c - (-s) * orientationToVector.y };
	const float newY2{ orientationToVector.x * (-s) + c * orientationToVector.y };
	const Elite::Vector2 rotatedLinearVelocity2{ newX2, newY2 };
	Elite::Vector2 pointThree{ m_pAgent->GetPosition().x + rotatedLinearVelocity2.GetNormalized().x * (m_pAgent->GetRadius() + m_PlayerFOVRange),
									m_pAgent->GetPosition().y + rotatedLinearVelocity2.GetNormalized().y * (m_pAgent->GetRadius() + m_PlayerFOVRange) };

	for (const auto& collider : m_vNavigationColliders)
	{
		const Elite::Vector2 position{ collider->GetPosition() };
		const float width{ collider->GetWidth() };
		const float height{ collider->GetHeight() };
		Elite::Rect hitbox{ {position.x - width / 2.f, position.y - height / 2.f},width, height };
		Collisions::HitInfo hitInfo{};
		if (Collisions::IsOverlapping(m_pAgent->GetPosition(), pointTwo, hitbox, hitInfo))
		{
			pointTwo = hitInfo.intersectPoint;
		}
		if (Collisions::IsOverlapping(m_pAgent->GetPosition(), pointThree, hitbox, hitInfo))
		{
			pointThree = hitInfo.intersectPoint;
		}
	}

	std::vector<Elite::Vector2> pointsOfTriangle{ pointOne,pointTwo,pointThree };
	m_PlayerFOV = Elite::Polygon{ pointsOfTriangle };

	int counter{};
	const float amountOfRaycasts{ s / (s * 10.f) }; // 0.10f => Gives us 10 raycasts
	for (float i{ -s }; i <= s; i += amountOfRaycasts)
	{
		const float newX3{ orientationToVector.x * c - i * orientationToVector.y };
		const float newY3{ orientationToVector.x * i + c * orientationToVector.y };
		const Elite::Vector2 rotatedLinearVelocity3{ newX3, newY3 };
		Elite::Vector2 outerPointOfFOV{ m_pAgent->GetPosition().x + rotatedLinearVelocity3.GetNormalized().x * (m_pAgent->GetRadius() + m_PlayerFOVRange),
								m_pAgent->GetPosition().y + rotatedLinearVelocity3.GetNormalized().y * (m_pAgent->GetRadius() + m_PlayerFOVRange) };

		Elite::Vector2 array[2] = { pointTwo,pointThree };

		Collisions::HitInfo hitInfo{};
		if (Collisions::Raycast(array, 2, m_pAgent->GetPosition(), outerPointOfFOV, hitInfo))
		{
			outerPointOfFOV = hitInfo.intersectPoint;
		}

		m_PlayerFOVRaycasts[counter++] = outerPointOfFOV;
	}
}
void App_MicroMacroAI::UpdateAlienFOV()
{
	for (size_t i{}; i < m_AlienFOVsRaycasts.size(); ++i)
	{
		const float c{ cos(m_AlienFOVAngles[i]) };
		const float s{ sin(m_AlienFOVAngles[i]) };
		const Elite::Vector2 orientationVector{ Elite::OrientationToVector(m_pAlien->GetOrientation()) };

		const Elite::Vector2 pointOne{ m_pAlien->GetPosition() };

		const float newX{ orientationVector.x * c - s * orientationVector.y };
		const float newY{ orientationVector.x * s + c * orientationVector.y };
		const Elite::Vector2 rotatedLinearVelocity{ newX, newY };
		Elite::Vector2 pointTwo{ m_pAlien->GetPosition().x + rotatedLinearVelocity.GetNormalized().x * (m_pAlien->GetRadius() + m_AlienFOVRanges[i]),
										m_pAlien->GetPosition().y + rotatedLinearVelocity.GetNormalized().y * (m_pAlien->GetRadius() + m_AlienFOVRanges[i]) };

		const float newX2{ orientationVector.x * c - (-s) * orientationVector.y };
		const float newY2{ orientationVector.x * (-s) + c * orientationVector.y };
		const Elite::Vector2 rotatedLinearVelocity2{ newX2, newY2 };
		Elite::Vector2 pointThree{ m_pAlien->GetPosition().x + rotatedLinearVelocity2.GetNormalized().x * (m_pAlien->GetRadius() + m_AlienFOVRanges[i]),
										m_pAlien->GetPosition().y + rotatedLinearVelocity2.GetNormalized().y * (m_pAlien->GetRadius() + m_AlienFOVRanges[i]) };

		for (const auto& collider : m_vNavigationColliders)
		{
			const Elite::Vector2 position{ collider->GetPosition() };
			const float width{ collider->GetWidth() };
			const float height{ collider->GetHeight() };
			Elite::Rect hitbox{ {position.x - width / 2.f, position.y - height / 2.f},width, height };
			Collisions::HitInfo hitInfo{};
			if (Collisions::IsOverlapping(m_pAlien->GetPosition(), pointTwo, hitbox, hitInfo))
			{
				pointTwo = hitInfo.intersectPoint;
			}
			if (Collisions::IsOverlapping(m_pAlien->GetPosition(), pointThree, hitbox, hitInfo))
			{
				pointThree = hitInfo.intersectPoint;
			}
		}

		std::vector<Elite::Vector2> pointsOfTriangle{ pointOne,pointTwo,pointThree };
		m_AlienFOVs[i] = Elite::Polygon{ pointsOfTriangle };

		int counter{};
		// 2s
		const float amountOfRaycasts{ s / 5 };
		for (float j{ -s }; j <= s; j += amountOfRaycasts)
		{
			const float newX3{ orientationVector.x * c - j * orientationVector.y };
			const float newY3{ orientationVector.x * j + c * orientationVector.y };
			const Elite::Vector2 rotatedLinearVelocity3{ newX3, newY3 };
			Elite::Vector2 outerPointOfFOV{ m_pAlien->GetPosition().x + rotatedLinearVelocity3.GetNormalized().x * (m_pAlien->GetRadius() + m_AlienFOVRanges[i]),
									m_pAlien->GetPosition().y + rotatedLinearVelocity3.GetNormalized().y * (m_pAlien->GetRadius() + m_AlienFOVRanges[i]) };

			Elite::Vector2 array[2] = { pointTwo,pointThree };

			Collisions::HitInfo hitInfo{};
			if (Collisions::Raycast(array, 2, m_pAlien->GetPosition(), outerPointOfFOV, hitInfo))
			{
				outerPointOfFOV = hitInfo.intersectPoint;
			}

			m_AlienFOVsRaycasts[i][counter++] = outerPointOfFOV;
		}
	}
}
void App_MicroMacroAI::Render(float deltaTime) const
{
#pragma region RenderNavMesh
	if (sShowGraph)
	{
		m_GraphRenderer.RenderGraph(m_pNavGraph, true, true);
	}

	if (sShowPolygon)
	{
		DEBUGRENDERER2D->DrawPolygon(m_pNavGraph->GetNavMeshPolygon(),
			Color(0.1f, 0.1f, 0.1f));
		DEBUGRENDERER2D->DrawSolidPolygon(m_pNavGraph->GetNavMeshPolygon(),
			Color(0.0f, 0.5f, 0.1f, 0.05f), 0.4f);
	}

	if (sDrawPortals)
	{
		for (const auto& portal : m_Portals)
		{
			DEBUGRENDERER2D->DrawSegment(portal.Line.p1, portal.Line.p2, Color(1.f, .5f, 0.f), -0.1f);
			//Draw just p1 p2
			std::string p1{ "p1" };
			std::string p2{ "p2" };
			//Add the positions to the debugdrawing
			//p1 +=" x:" + std::to_string(portal.Line.p1.x) + ", y: " + std::to_string(portal.Line.p1.y);
			//p2 +=" x:" + std::to_string(portal.Line.p2.x) + ", y: " + std::to_string(portal.Line.p2.y);
			DEBUGRENDERER2D->DrawString(portal.Line.p1, p1.c_str(), Color(1.f, .5f, 0.f), -0.1f);
			DEBUGRENDERER2D->DrawString(portal.Line.p2, p2.c_str(), Color(1.f, .5f, 0.f), -0.1f);

		}
	}

	if (sDrawNonOptimisedPath && m_DebugNodePositions.size() > 0)
	{
		for (auto pathNode : m_DebugNodePositions)
			DEBUGRENDERER2D->DrawCircle(pathNode, 2.0f, Color(0.f, 0.f, 1.f), -0.45f);
	}

	if (sDrawFinalPath && m_AgentPath.size() > 0)
	{

		for (auto pathPoint : m_AgentPath)
			DEBUGRENDERER2D->DrawCircle(pathPoint, 2.0f, Color(1.f, 0.f, 0.f), -0.2f);

		DEBUGRENDERER2D->DrawSegment(m_pAgent->GetPosition(), m_AgentPath[0], Color(1.f, 0.0f, 0.0f), -0.2f);
		for (size_t i = 0; i < m_AgentPath.size() - 1; i++)
		{
			float g = float(i) / m_AgentPath.size();
			DEBUGRENDERER2D->DrawSegment(m_AgentPath[i], m_AgentPath[i + 1], Color(1.f, g, g), -0.2f);
		}

	}
#pragma endregion

	//----------- RENDER PLAYER FOV ------------
	DEBUGRENDERER2D->DrawPolygon(const_cast<Elite::Polygon*>(&m_PlayerFOV), { 0.f,255.f,0.f });


	//----------- RENDER ALIEN FOV ------------
	DEBUGRENDERER2D->DrawPolygon(const_cast<Elite::Polygon*>(&m_AlienFOVs[0]), { 255.f,0.f,0.f });
	DEBUGRENDERER2D->DrawPolygon(const_cast<Elite::Polygon*>(&m_AlienFOVs[1]), { 0.f,255.f,0.f });
	DEBUGRENDERER2D->DrawPolygon(const_cast<Elite::Polygon*>(&m_AlienFOVs[2]), { 0.f,0.f,255.f });


	//----------- RENDER FOV RAYCASTS ------------
	for (const auto& rayCast : m_PlayerFOVRaycasts)
	{
		DEBUGRENDERER2D->DrawSegment(m_pAgent->GetPosition(), rayCast, { 0.f,0.f,255.f });
		//DEBUGRENDERER2D->DrawPoint(rayCast, 5.f, { 0.f,0.f,255.f });
	}


	//----------- RENDER PICKUPS ------------
	for (const auto& pickup : m_Pickups)
	{
		DEBUGRENDERER2D->DrawPoint(pickup, 5.f, { 0.f,0.f,255.f });
	}


	//----------- RENDER CHECKPOINTS ------------
	for (const auto& checkpoint : m_Checkpoints)
	{
		if (checkpoint.hasBeenVisited)
			DEBUGRENDERER2D->DrawPoint(checkpoint.position, 5.f, { 0.f,255.f,0.f });
		else
			DEBUGRENDERER2D->DrawPoint(checkpoint.position, 5.f, { 255.f,0.f,0.f });
	}


	for (const auto& hits : GetPickupsInPlayerFOV())
	{
		DEBUGRENDERER2D->DrawCircle(hits, 10.f, { 255.f,255.f,0.f }, -1.f);
	}


	//----------- RENDER GRABRANGE ------------
	DEBUGRENDERER2D->DrawCircle(m_pAgent->GetPosition(), m_GrabRange, { 255.f,0.f,0.f }, -1.f);
}

const std::vector<Elite::Vector2> App_MicroMacroAI::FindPath(const Elite::Vector2& startPos, const Elite::Vector2& endPos, MicroAIAgent* pAgent)
{
	//Create the path to return
	std::vector<Elite::Vector2> finalPath{};
	m_DebugNodePositions.clear();

	//Get the start and endTriangle
	const Elite::Triangle* pStartTriangle{ m_pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(startPos) };
	const Elite::Triangle* pEndTriangle{ m_pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(endPos) };

	//If we have valid start/end triangles and they are not the same
	if (pStartTriangle == nullptr || pEndTriangle == nullptr)
	{
		//std::cout << "Triangle was nullptr! App_NavMeshGraph.cpp FindPath()" << std::endl;
		return finalPath;
	}
	if (pStartTriangle == pEndTriangle)
	{
		finalPath.push_back(endPos);
		return finalPath;
	}

	//=> Start looking for a path

	//Copy the graph
	auto pCopiedGraph{ m_pNavGraph->Clone() };
	//auto pCopiedGraph = m_pNavGraph;
	int index{ pCopiedGraph->GetNextFreeNodeIndex() };

	//Create extra node for the Start Node (Agent's position)
	Elite::NavGraphNode* pStartNode{ new Elite::NavGraphNode{index,pStartTriangle->metaData.IndexLines[0],pAgent->GetPosition()} };
	pCopiedGraph->AddNode(pStartNode);

	for (auto pEdge : pStartTriangle->metaData.IndexLines)
	{
		auto lineIndexFinder{ std::find_if(pCopiedGraph->GetAllNodes().begin(),pCopiedGraph->GetAllNodes().end(),[&pEdge](const NavGraphNode* n)->bool
			{
				return n->GetLineIndex() == pEdge;
				}) };
		if (lineIndexFinder != pCopiedGraph->GetAllNodes().end())
		{
			if (pCopiedGraph->IsNodeValid((*lineIndexFinder)->GetIndex())) // if there is a valid node
			{
				GraphNode2D* pValidEdgeNode{ pCopiedGraph->GetNode((*lineIndexFinder)->GetIndex()) };
				if (pValidEdgeNode != pStartNode) // if that valid node is not the start node, because we don't want a connection to itself
				{
					GraphConnection2D* pNewConnection{ new GraphConnection2D{pStartNode->GetIndex(),pValidEdgeNode->GetIndex(),
					Distance(pStartNode->GetPosition(),pValidEdgeNode->GetPosition())} };
					pCopiedGraph->AddConnection(pNewConnection);
				}
			}
		}
	}

	//Create extra node for the End Node
	index = pCopiedGraph->GetNextFreeNodeIndex();
	Elite::NavGraphNode* pEndNode{ new Elite::NavGraphNode{index,pEndTriangle->metaData.IndexLines[0],endPos} };
	pCopiedGraph->AddNode(pEndNode);
	for (auto pEdge : pEndTriangle->metaData.IndexLines)
	{
		auto lineIndexFinder{ std::find_if(pCopiedGraph->GetAllNodes().begin(),pCopiedGraph->GetAllNodes().end(),[pEdge](const NavGraphNode* n)->bool
			{
				return n->GetLineIndex() == pEdge;
				}) };
		if (lineIndexFinder != pCopiedGraph->GetAllNodes().end())
		{
			if (pCopiedGraph->IsNodeValid((*lineIndexFinder)->GetIndex())) // if there is a valid node
			{
				GraphNode2D* pValidEdgeNode{ pCopiedGraph->GetNode((*lineIndexFinder)->GetIndex()) };
				if (pValidEdgeNode != pEndNode) // if that valid node is not the start node
				{
					GraphConnection2D* pNewConnection{ new GraphConnection2D{pEndNode->GetIndex(),pValidEdgeNode->GetIndex(),
					Distance(pEndNode->GetPosition(),pValidEdgeNode->GetPosition())} };
					pCopiedGraph->AddConnection(pNewConnection);
				}
			}
		}
	}

	//Run A star on new graph
	auto pathfinder = AStar<NavGraphNode, GraphConnection2D>(pCopiedGraph.get(), Elite::HeuristicFunctions::Euclidean);
	auto path{ pathfinder.FindPath(pStartNode,pEndNode) };
	for (auto pNode : path)
	{
		m_DebugNodePositions.push_back(pNode->GetPosition());
		finalPath.push_back(pNode->GetPosition());
	}

	//Extra: Run optimiser on new graph, Make sure the A star path is fine before uncommenting this!
	m_Portals = SSFA::FindPortals(path, m_pNavGraph->GetNavMeshPolygon());
	finalPath = SSFA::OptimizePortals(m_Portals);

	return finalPath;
}

const std::vector<Elite::Vector2> App_MicroMacroAI::GetPickupsInPlayerFOV() const
{
	std::vector<Elite::Vector2> pickUpsSeen{};

	const Elite::Vector2 agentPosition{ m_pAgent->GetPosition() };
	const float pickupSize{ 5.f };
	for (const auto& ray : m_PlayerFOVRaycasts)
	{
		for (const auto& pickup : m_Pickups)
		{
			for (const auto& navMeshCollider : m_vNavigationColliders)
			{
				const Elite::Rect colliderHitbox{ navMeshCollider->GetPosition(),navMeshCollider->GetWidth(),navMeshCollider->GetHeight() };
				Collisions::HitInfo hitInfo{};
				if (!Collisions::IsOverlapping(agentPosition, pickup, colliderHitbox, hitInfo))
					if (Elite::IsSegmentIntersectingWithCircle(agentPosition, ray, pickup, pickupSize))
						pickUpsSeen.push_back(pickup);
			}
		}
	}
	return pickUpsSeen;
}
bool App_MicroMacroAI::IsPlayerInAlienFOV(Elite::Blackboard* pBlackboard, const float dt) const
{
	const Elite::Vector2 agentPosition{ m_pAgent->GetPosition() };
	const Elite::Vector2 alienPosition{ m_pAlien->GetPosition() };
	const float playerHitboxSize{ 3.f };
	for (size_t i{}; i < m_AlienFOVs.size(); ++i)
	{
		for (const auto& navMeshCollider : m_vNavigationColliders)
		{
			const Elite::Vector2 position{ navMeshCollider->GetPosition() };
			const float width{ navMeshCollider->GetWidth() };
			const float height{ navMeshCollider->GetHeight() };
			Elite::Rect hitbox{ {position.x - width / 2.f, position.y - height / 2.f},width, height };
			Collisions::HitInfo hitInfo{};
			if (!Collisions::IsOverlapping(alienPosition, agentPosition, hitbox, hitInfo)) // no wall between alien and player
			{
				if (i == 0)
				{
					for (const auto& ray : m_AlienFOVsRaycasts[i])
					{
						if (Elite::IsSegmentIntersectingWithCircle(alienPosition, ray, agentPosition, playerHitboxSize))
						{
							float isPlayerInFOVTimer{};
							pBlackboard->GetData("isPlayerInFOVTimer", isPlayerInFOVTimer);
							isPlayerInFOVTimer += dt;
							pBlackboard->ChangeData("isPlayerInFOVTimer", isPlayerInFOVTimer);
							if (isPlayerInFOVTimer >= 2.f)
							{
								pBlackboard->ChangeData("isPlayerInFOVTimer", 0.f);
								return true;
							}
							break; // only increment the timer once
						}
					}
				}
				else
				{
					for (const auto& ray : m_AlienFOVsRaycasts[i])
						if (Elite::IsSegmentIntersectingWithCircle(alienPosition, ray, agentPosition, playerHitboxSize)) // no wall between alien and player
							return true;
				}
			}
		}
	}
	return false;
}


void App_MicroMacroAI::UpdateImGui()
{
	//------- UI --------
#ifdef PLATFORM_WINDOWS
#pragma region UI
	{
		//Setup
		int menuWidth = 150;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 90));
		ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::PushAllowKeyboardFocus(false);
		ImGui::SetWindowFocus();
		ImGui::PushItemWidth(70);
		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Checkbox("Show Polygon", &sShowPolygon);
		ImGui::Checkbox("Show Graph", &sShowGraph);
		ImGui::Checkbox("Show Portals", &sDrawPortals);
		ImGui::Checkbox("Show Path Nodes", &sDrawNonOptimisedPath);
		ImGui::Checkbox("Show Final Path", &sDrawFinalPath);
		ImGui::Spacing();
		ImGui::Spacing();

		if (ImGui::SliderFloat("AgentSpeed", &m_AgentSpeed, 0.0f, 22.0f))
		{
			m_pAgent->SetMaxLinearSpeed(m_AgentSpeed);
		}

		ImGui::Spacing();
		ImGui::Spacing();

		if (ImGui::SliderFloat("AlienSpeed", &m_AlienSpeed, 0.0f, 22.0f))
		{
			m_pAlien->SetMaxLinearSpeed(m_AlienSpeed);
		}

		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
#pragma endregion
#endif
}

void App_MicroMacroAI::UpdateEndgameImGui()
{
	//Setup
	int menuWidth = 150;
	int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
	int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
	bool windowActive = true;
	ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
	ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 90));
	ImGui::Begin("Game Over", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
}
