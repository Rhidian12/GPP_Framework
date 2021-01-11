#include "stdafx.h"
#include "App_AgarioGame.h"
#include "StatesAndTransitions.h"
#include "../Shared/Agario/AgarioFood.h"
#include "../Shared/Agario/AgarioAgent.h"
#include "../Shared/Agario/AgarioContactListener.h"

using namespace Elite;
App_AgarioGame::App_AgarioGame()
{
}

App_AgarioGame::~App_AgarioGame()
{
	for (auto& f : m_pFoodVec)
	{
		SAFE_DELETE(f);
	}
	m_pFoodVec.clear();

	for (auto& a : m_pAgentVec)
	{
		SAFE_DELETE(a);
	}
	m_pAgentVec.clear();

	SAFE_DELETE(m_pContactListener);
	for (auto& s : m_pStates)
	{
		SAFE_DELETE(s);
	}

	for (auto& t : m_pTransitions)
	{
		SAFE_DELETE(t);
	}
	SAFE_DELETE(m_pUberAgent);

}

void App_AgarioGame::Start()
{
	// === Common States ===
	WanderState* pWanderState{ new WanderState{} };
	m_pStates.push_back(pWanderState); // add state to vector so that it gets deleted

	//Create food items
	m_pFoodVec.reserve(m_AmountOfFood);
	for (int i = 0; i < m_AmountOfFood; i++)
	{
		Elite::Vector2 randomPos = randomVector2(-m_TrimWorldSize, m_TrimWorldSize);
		m_pFoodVec.push_back(new AgarioFood(randomPos));
	}

	//Create agents
	m_pAgentVec.reserve(m_AmountOfAgents);
	for (int i = 0; i < m_AmountOfAgents; i++)
	{
		Elite::Vector2 randomPos = randomVector2(-m_TrimWorldSize, m_TrimWorldSize);
		AgarioAgent* pNewAgent = new AgarioAgent(randomPos);

		// Need to add a dumb Finite State Machine

		Blackboard* pBlackBoard{ new Blackboard{} };
		bool isSmartAgent{};
		pBlackBoard->AddData("Agent", pNewAgent);
		pBlackBoard->AddData("isSmartAgent", isSmartAgent);

		FiniteStateMachine* pDumbFSM{ new FiniteStateMachine{pWanderState,pBlackBoard} };
		pNewAgent->SetDecisionMaking(pDumbFSM);

		m_pAgentVec.push_back(pNewAgent);
	}

	//Creating the world contact listener that informs us of collisions
	m_pContactListener = new AgarioContactListener();

	//Create Uber Agent
	Elite::Vector2 randomPos = randomVector2(-m_TrimWorldSize, m_TrimWorldSize);
	Color customColor = Color{ randomFloat(), randomFloat(), randomFloat() };
	m_pUberAgent = new AgarioAgent(randomPos, customColor);

	// Need to add a smart Finite State Machine
	// We will need to add states
	// We will need to add transitions with appropriate checks
	Blackboard* pBlackBoard{ new Blackboard{} };
	bool isSmartAgent{ true };
	AgarioAgent* pClosestEnemyToChase{};
	const float radiusToCheckForFood{ 35.f }; // divided by 5.f
	const float radiusToCheckForLargerEnemies{ 30.f }; // divided by 2.5f
	const float radiusToCheckForSmallerEnemies{ 20.f }; // divided by 3.f
	Elite::Vector2 positionOfClosestFood{};
	AgarioAgent* pLargerEnemyToRunAwayFrom{};
	pBlackBoard->AddData("FoodVector", &m_pFoodVec);
	pBlackBoard->AddData("SmartAgent", m_pUberAgent);
	pBlackBoard->AddData("DumbAgents", &m_pAgentVec);
	pBlackBoard->AddData("ClosestEnemyToChase", pClosestEnemyToChase);
	pBlackBoard->AddData("isSmartAgent", isSmartAgent);
	pBlackBoard->AddData("RadiusToCheckForFood", radiusToCheckForFood);
	pBlackBoard->AddData("positionOfClosestFood", positionOfClosestFood);
	pBlackBoard->AddData("radiusToCheckForLargerEnemies", radiusToCheckForLargerEnemies);
	pBlackBoard->AddData("largerEnemyToRunAwayFrom", pLargerEnemyToRunAwayFrom);
	pBlackBoard->AddData("radiusToCheckForSmallerEnemies", radiusToCheckForSmallerEnemies);

	FiniteStateMachine* pSmartFSM{ new FiniteStateMachine{pWanderState,pBlackBoard} };

	SeekFood* pSeekFood{ new SeekFood{} };
	EvadeLargerEnemy* pEvadeLargerEnemy{ new EvadeLargerEnemy{} };
	SeekSmallerEnemy* pSeekSmallerEnemy{ new SeekSmallerEnemy{} };

	m_pStates.push_back(pSeekFood);
	m_pStates.push_back(pEvadeLargerEnemy);
	m_pStates.push_back(pSeekSmallerEnemy);

	IsFoodNearby* pIsFoodNearby{ new IsFoodNearby{} };
	IsNoFoodNearby* pIsNoFoodNearby{ new IsNoFoodNearby{} };
	IsALargerEnemyNearby* pIsALargerEnemyNearby{ new IsALargerEnemyNearby{} };
	IsNoLargerEnemyNearby* pIsNoLargerEnemyNearby{ new IsNoLargerEnemyNearby{} };
	IsASmallerEnemyNearby* pIsASmallerEnemyNearby{ new IsASmallerEnemyNearby{} };
	IsNoSmallerEnemyNearby* pIsNoSmallerEnemyNearby{ new IsNoSmallerEnemyNearby{} };

	m_pTransitions.push_back(pIsFoodNearby);
	m_pTransitions.push_back(pIsNoFoodNearby);
	m_pTransitions.push_back(pIsALargerEnemyNearby);
	m_pTransitions.push_back(pIsNoLargerEnemyNearby);
	m_pTransitions.push_back(pIsASmallerEnemyNearby);
	m_pTransitions.push_back(pIsNoSmallerEnemyNearby);

	// == SeekFood <=> EvadeLargerEnemy ==
	pSmartFSM->AddTransition(pSeekFood, pEvadeLargerEnemy, pIsALargerEnemyNearby);
	pSmartFSM->AddTransition(pEvadeLargerEnemy, pSeekFood, pIsNoLargerEnemyNearby);

	// == SeekFood <=> ChaseSmallerEnemy ==
	pSmartFSM->AddTransition(pSeekFood, pSeekSmallerEnemy, pIsASmallerEnemyNearby);
	pSmartFSM->AddTransition(pSeekSmallerEnemy, pSeekFood, pIsNoSmallerEnemyNearby);

	// == SeekFood <=> Wander ==
	pSmartFSM->AddTransition(pSeekFood, pWanderState, pIsNoFoodNearby);
	pSmartFSM->AddTransition(pWanderState, pSeekFood, pIsFoodNearby);

	m_pUberAgent->SetDecisionMaking(pSmartFSM);
}

void App_AgarioGame::Update(float deltaTime)
{
	UpdateImGui();

	//Check if agent is still alive
	if (m_pUberAgent->CanBeDestroyed())
	{
		m_GameOver = true;
		return;
	}
	//Update the custom agent
	m_pUberAgent->Update(deltaTime);
	m_pUberAgent->TrimToWorld(m_TrimWorldSize);

	//Update the other agents and food
	UpdateAgarioEntities(m_pFoodVec, deltaTime);
	UpdateAgarioEntities(m_pAgentVec, deltaTime);


	//Check if we need to spawn new food
	m_TimeSinceLastFoodSpawn += deltaTime;
	if (m_TimeSinceLastFoodSpawn > m_FoodSpawnDelay)
	{
		m_TimeSinceLastFoodSpawn = 0.f;
		m_pFoodVec.push_back(new AgarioFood(randomVector2(-m_TrimWorldSize, m_TrimWorldSize)));
	}
}

void App_AgarioGame::Render(float deltaTime) const
{
	std::vector<Elite::Vector2> points =
	{
		{ -m_TrimWorldSize, m_TrimWorldSize },
		{ m_TrimWorldSize, m_TrimWorldSize },
		{ m_TrimWorldSize, -m_TrimWorldSize },
		{ -m_TrimWorldSize, -m_TrimWorldSize }
	};
	DEBUGRENDERER2D->DrawPolygon(&points[0], 4, { 1,0,0,1 }, 0.4f);

	for (AgarioFood* f : m_pFoodVec)
	{
		f->Render(deltaTime);
	}

	for (AgarioAgent* a : m_pAgentVec)
	{
		a->Render(deltaTime);
	}

	m_pUberAgent->Render(deltaTime);
}

void App_AgarioGame::UpdateImGui()
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
		ImGui::Begin("Agario", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
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

		ImGui::Text("Agent Info");
		ImGui::Text("Radius: %.1f", m_pUberAgent->GetRadius());
		ImGui::Text("Survive Time: %.1f", TIMER->GetTotal());

		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
	if (m_GameOver)
	{
		//Setup
		int menuWidth = 300;
		int menuHeight = 100;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2(width / 2.0f - menuWidth, height / 2.0f - menuHeight));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)menuHeight));
		ImGui::Begin("Game Over", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
		ImGui::Text("Final Agent Info");
		ImGui::Text("Radius: %.1f", m_pUberAgent->GetRadius());
		ImGui::Text("Survive Time: %.1f", TIMER->GetTotal());
		ImGui::End();
	}
#pragma endregion
#endif

}
