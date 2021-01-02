// PCH
#include "stdafx.h"

// includes
#include "App_MicroMacroAI.h"
#include "projects/Shared/NavigationColliderElement.h"

#include "../App_Steering/SteeringAgent.h"
#include "../App_Steering/SteeringBehaviors.h"

#include "framework\EliteAI\EliteGraphs\ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

//Statics
bool App_MicroMacroAI::sShowPolygon = true;
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
	m_vNavigationColliders.push_back(new NavigationColliderElement{ Elite::Vector2{35.f,-25.f},1.f,20.f,90.f });
	m_vNavigationColliders.push_back(new NavigationColliderElement{ Elite::Vector2{-20.f,-35.f},60.f,1.f });
	m_vNavigationColliders.push_back(new NavigationColliderElement{ Elite::Vector2{0.f,-57.f},1.f,20.f,90.f });
	m_vNavigationColliders.push_back(new NavigationColliderElement{ Elite::Vector2{0.f,5.f},90.f,1.f });
	m_vNavigationColliders.push_back(new NavigationColliderElement{ Elite::Vector2{0.f,58.f},130.f,1.f });

	//----------- NAVMESH  ------------
	std::list<Elite::Vector2> baseBox
	{ { -100, 100 },{ -100, -100 },{ 100, -100 },{ 100, 100 } };

	m_pNavGraph = new Elite::NavGraph(Elite::Polygon(baseBox), m_AgentRadius);

	//----------- AGENT ------------
	m_pWander = new Wander{};
	m_pFlee = new Flee{};
	m_pSeek = new Seek{};
	m_Target = TargetData(Elite::ZeroVector2);
	m_pAgent = new SteeringAgent();
	m_pAgent->SetSteeringBehavior(m_pSeek);
	m_pAgent->SetMaxLinearSpeed(m_AgentSpeed);
	m_pAgent->SetAutoOrient(true);
	m_pAgent->SetMass(0.1f);
	m_pAgent->SetBodyColor(Elite::Color{ 0.f,0.f,255.f });
}
void App_MicroMacroAI::Update(float deltaTime)
{
	//Update target/path based on input
	if (INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eMiddle))
	{
		auto mouseData = INPUTMANAGER->GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eMiddle);
		Elite::Vector2 mouseTarget = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld(
			Elite::Vector2((float)mouseData.X, (float)mouseData.Y));
		m_vPath = FindPath(m_pAgent->GetPosition(), mouseTarget);
	}

	//Check if a path exist and move to the following point
	if (m_vPath.size() > 0)
	{
		if (m_vPath.size() == 1)
		{
			m_pAgent->SetSteeringBehavior(m_pSeek);
			m_pSeek->SetTarget(m_vPath[0]);
		}
		else
		{
			m_pAgent->SetSteeringBehavior(m_pSeek);
			m_pSeek->SetTarget(m_vPath[0]);
		}

		if (Elite::DistanceSquared(m_pAgent->GetPosition(), m_vPath[0]) < m_AgentRadius * m_AgentRadius)
		{
			//If we reached the next point of the path. Remove it 
			m_vPath.erase(std::remove(m_vPath.begin(), m_vPath.end(), m_vPath[0]));
		}
	}

	UpdateImGui();

	m_pAgent->Update(deltaTime);
	//m_pAgent->TrimToWorld(Elite::Vector2{ -100.f,-100.f }, Elite::Vector2{ 100.f,100.f });
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

	if (sDrawFinalPath && m_vPath.size() > 0)
	{

		for (auto pathPoint : m_vPath)
			DEBUGRENDERER2D->DrawCircle(pathPoint, 2.0f, Color(1.f, 0.f, 0.f), -0.2f);

		DEBUGRENDERER2D->DrawSegment(m_pAgent->GetPosition(), m_vPath[0], Color(1.f, 0.0f, 0.0f), -0.2f);
		for (size_t i = 0; i < m_vPath.size() - 1; i++)
		{
			float g = float(i) / m_vPath.size();
			DEBUGRENDERER2D->DrawSegment(m_vPath[i], m_vPath[i + 1], Color(1.f, g, g), -0.2f);
		}

	}
#pragma endregion
	//DEBUGRENDERER2D->DrawSolidCircle(m_pAgent->GetPosition(), m_pAgent->GetRadius(), Elite::Vector2{ 1.f,0.f }, Elite::Color{ 0.f,0.f,255.f,0.5f });
}

const std::vector<Elite::Vector2> App_MicroMacroAI::FindPath(const Elite::Vector2& startPos, const Elite::Vector2& endPos)
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
		std::cout << "Triangle was nullptr! App_NavMeshGraph.cpp FindPath()" << std::endl;
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
	Elite::NavGraphNode* pStartNode{ new Elite::NavGraphNode{index,pStartTriangle->metaData.IndexLines[0],m_pAgent->GetPosition()} };
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

		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
#pragma endregion
#endif
}
