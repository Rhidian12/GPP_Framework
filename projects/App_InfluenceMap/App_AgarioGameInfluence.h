#ifndef AGARIO_GAME_APPLICATION_H
#define AGARIO_GAME_APPLICATION_H
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteInterfaces/EIApp.h"
#include "framework\EliteAI\EliteGraphs\EGraph2D.h"
#include "framework\EliteAI\EliteGraphs\EInfluenceMap.h"
#include "framework\EliteAI\EliteGraphs\EGridGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphEditor.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphRenderer.h"

class AgarioFood;
class AgarioAgent;
class AgarioContactListener;

class App_AgarioGameInfluence final : public IApp
{
public:
	App_AgarioGameInfluence();
	~App_AgarioGameInfluence();

	void Start() override;
	void Update(float deltaTime) override;
	void Render(float deltaTime) const override;

	using InfluenceGrid = Elite::GridGraph<Elite::InfluenceNode, Elite::GraphConnection>;
	using InfluenceGraph = Elite::Graph2D<Elite::InfluenceNode, Elite::GraphConnection2D>;

private:
	// == Influence Map ==
	Elite::InfluenceMap<InfluenceGrid>* m_pInfluenceGrid = nullptr;
	Elite::InfluenceMap<InfluenceGraph>* m_pInfluenceGraph2D = nullptr;

	Elite::EGraphEditor m_GraphEditor{};
	Elite::EGraphRenderer m_GraphRenderer{};

	bool m_UseWaypointGraph = false;
	bool m_EditGraphEnabled = false;
	bool m_RenderAsGraph = false;

	// == AgarIo ==
	float m_TrimWorldSize = 140.f;
	const int m_AmountOfAgents{ 30 };
	std::vector<AgarioAgent*> m_pAgentVec{};

	AgarioAgent* m_pUberAgent = nullptr;

	const int m_AmountOfFood{ 60 };
	const float m_FoodSpawnDelay{ 2.f };
	float m_TimeSinceLastFoodSpawn{ 0.f };
	std::vector<AgarioFood*> m_pFoodVec{};

	AgarioContactListener* m_pContactListener = nullptr;
	bool m_GameOver = false;

	//Decision making 
	std::vector<Elite::FSMState*> m_pStates{};
	std::vector<Elite::FSMTransition*> m_pTransitions{};

private:
	template<class T_AgarioType>
	void UpdateAgarioEntities(vector<T_AgarioType*>& entities, float deltaTime, const Elite::Rect& bounds);

	void UpdateImGui();
private:
	//C++ make the class non-copyable
	App_AgarioGameInfluence(const App_AgarioGameInfluence&) {};
	App_AgarioGameInfluence& operator=(const App_AgarioGameInfluence&) {};
};

#endif

template<class T_AgarioType>
inline void App_AgarioGameInfluence::UpdateAgarioEntities(vector<T_AgarioType*>& entities, float deltaTime, const Elite::Rect& bounds)
{
	for (auto& e : entities)
	{
		e->Update(deltaTime);

		auto agent = dynamic_cast<AgarioAgent*>(e);
		if (agent)
			agent->TrimToWorld(bounds);

		if (e->CanBeDestroyed())
			SAFE_DELETE(e);
	}

	auto toRemoveEntityIt = std::remove_if(entities.begin(), entities.end(),
		[](T_AgarioType* e) {return e == nullptr; });
	if (toRemoveEntityIt != entities.end())
	{
		entities.erase(toRemoveEntityIt, entities.end());
	}
}
