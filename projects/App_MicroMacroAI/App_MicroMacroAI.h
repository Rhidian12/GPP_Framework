#ifndef SANDBOX_APPLICATION_H
#define SANDBOX_APPLICATION_H
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteInterfaces/EIApp.h"
#include "../App_Steering/SteeringHelpers.h"

#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphRenderer.h"
#include "framework\EliteAI\EliteNavigation\Algorithms\EPathSmoothing.h"

class NavigationColliderElement;
class SteeringAgent;
class Wander;
class Flee;
class Seek;

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
	const std::vector<Elite::Vector2> FindPath(const Elite::Vector2& startPos, const Elite::Vector2& endPos);

	void UpdateImGui();

	// --Agents--
	SteeringAgent* m_pAgent = nullptr;
	Wander* m_pWander = nullptr;
	Flee* m_pFlee = nullptr;
	Seek* m_pSeek = nullptr;
	TargetData m_Target = {};
	float m_AgentRadius = 1.0f;
	float m_AgentSpeed = 16.0f;

	// --Level--
	std::vector<NavigationColliderElement*> m_vNavigationColliders = {};

	// --Pathfinder--
	std::vector<Elite::Vector2> m_vPath;

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