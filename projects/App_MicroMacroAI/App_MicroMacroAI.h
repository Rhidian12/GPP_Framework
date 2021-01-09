#ifndef SANDBOX_APPLICATION_H
#define SANDBOX_APPLICATION_H
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteInterfaces/EIApp.h"
#include "../App_Steering/SteeringHelpers.h"

#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphRenderer.h"
#include "framework\EliteAI\EliteNavigation\Algorithms\EPathSmoothing.h"

#include "Structs.h"

class NavigationColliderElement;
class Wander;
class Flee;
class Seek;
class MicroAIAgent;
class Alien;
class MacroAI;
namespace Elite
{
	class FSMState;
	class FSMTransition;
}

namespace Elite
{
	class NavGraph;
}

//-----------------------------------------------------------------
// Application
//-----------------------------------------------------------------
class App_MicroMacroAI final : public IApp
{
public:
	//Constructor & Destructor
	App_MicroMacroAI() = default;
	App_MicroMacroAI(const App_MicroMacroAI&) = delete;
	App_MicroMacroAI& operator=(const App_MicroMacroAI&) = delete;

	virtual ~App_MicroMacroAI() final;

	//App Functions
	void Start() override;
	void Update(float deltaTime) override;
	void Render(float deltaTime) const override;

private:
	const std::vector<Elite::Vector2> FindPath(const Elite::Vector2& startPos, const Elite::Vector2& endPos, MicroAIAgent* pAgent);
	const std::vector<Elite::Vector2> GetPickupsInPlayerFOV() const;
	bool IsPlayerInAlienFOV(Elite::Blackboard* pBlackboard, const float dt) const;
	void UpdateCheckpoints();
	void UpdatePlayerFOV();
	void UpdateAlienFOV();
	void InitializePlayer();
	void InitializeAlien();

	void UpdateImGui();

	// --Agents--
	MicroAIAgent* m_pAgent = nullptr;
	Alien* m_pAlien = nullptr;
	Wander* m_pWander = nullptr;
	Flee* m_pFlee = nullptr;
	Seek* m_pSeek = nullptr;
	TargetData m_Target = {};
	MacroAI* m_pMacroAI = nullptr;
	float m_AgentRadius = 1.0f;
	float m_AgentSpeed = 5.f;
	float m_AlienSpeed = 5.f;
	std::vector<Elite::Vector2> m_Pickups;
	const int m_MaxAmountOfPickups{ 10 };
	std::vector<Checkpoint> m_Checkpoints;
	std::vector<Elite::FSMState*> m_pPlayerStates;
	std::vector<Elite::FSMState*> m_pAlienStates;
	std::vector<Elite::FSMTransition*> m_pPlayerTransitions;
	std::vector<Elite::FSMTransition*> m_pAlienTransitions;
	std::vector<Elite::Vector2> m_PlayerFOVRaycasts;
	Elite::Polygon m_PlayerFOV{};
	std::vector<Elite::Polygon> m_AlienFOVs{};
	const std::vector<float> m_AlienFOVAngles{Elite::ToRadians(45.f), Elite::ToRadians(22.5f), Elite::ToRadians(80.f)};
	const std::vector<float> m_AlienFOVRanges{ 30.f,15.f,5.f };
	std::vector<std::vector<Elite::Vector2>> m_AlienFOVsRaycasts{};
	const float m_GrabRange{ 2.f };
	const float m_PlayerFOVAngle{ Elite::ToRadians(30.f) };
	const float m_PlayerFOVRange{ 15.f };
	

	// --Level--
	std::vector<NavigationColliderElement*> m_vNavigationColliders = {};

	// --Pathfinder--
	std::vector<Elite::Vector2> m_AgentPath;
	std::vector<Elite::Vector2> m_AlienPath;

	// --Graph--
	Elite::NavGraph* m_pNavGraph = nullptr;
	Elite::EGraphRenderer m_GraphRenderer{};

	// --Debug drawing information--
	std::vector<Elite::Portal> m_Portals;
	std::vector<Elite::Vector2> m_DebugNodePositions;
	static bool sShowPolygon;
	static bool sShowGraph;
	static bool sDrawPortals;
	static bool sDrawFinalPath;
	static bool sDrawNonOptimisedPath;
};
#endif