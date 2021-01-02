#include "stdafx.h"
#include "ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

using namespace Elite;

Elite::NavGraph::NavGraph(const Polygon& contourMesh, float playerRadius = 1.0f) :
	Graph2D(false),
	m_pNavMeshPolygon(nullptr)
{
	//Create the navigation mesh (polygon of navigatable area= Contour - Static Shapes)
	m_pNavMeshPolygon = new Polygon(contourMesh); // Create copy on heap

	//Get all shapes from all static rigidbodies with NavigationCollider flag
	auto vShapes = PHYSICSWORLD->GetAllStaticShapesInWorld(PhysicsFlags::NavigationCollider);

	//Store all children
	for (auto shape : vShapes)
	{
		shape.ExpandShape(playerRadius);
		m_pNavMeshPolygon->AddChild(shape);
	}

	//Triangulate
	m_pNavMeshPolygon->Triangulate();

	//Create the actual graph (nodes & connections) from the navigation mesh
	CreateNavigationGraph();
}

Elite::NavGraph::~NavGraph()
{
	delete m_pNavMeshPolygon;
	m_pNavMeshPolygon = nullptr;
}

int Elite::NavGraph::GetNodeIdxFromLineIdx(int lineIdx) const
{
	auto nodeIt = std::find_if(m_Nodes.begin(), m_Nodes.end(), [lineIdx](const NavGraphNode* n) { return n->GetLineIndex() == lineIdx; });
	if (nodeIt != m_Nodes.end())
	{
		return (*nodeIt)->GetIndex();
	}

	return invalid_node_index;
}

Elite::Polygon* Elite::NavGraph::GetNavMeshPolygon() const
{
	return m_pNavMeshPolygon;
}

void Elite::NavGraph::CreateNavigationGraph()
{
	//1. Go over all the edges of the navigationmesh and create nodes
	auto pLines{ m_pNavMeshPolygon->GetLines() };
	int index{ GetNextFreeNodeIndex() };
	for (auto pLine : pLines)
	{
		if (m_pNavMeshPolygon->GetTrianglesFromLineIndex(pLine->index).size() >= 2)
		{
			const Elite::Vector2 midPoint{ (pLine->p1.x + pLine->p2.x) * 0.5f, (pLine->p1.y + pLine->p2.y) * 0.5f };
			NavGraphNode* pNewNode{ new NavGraphNode{index,pLine->index,midPoint} };
			AddNode(pNewNode);
			index = GetNextFreeNodeIndex();
		}
	}

	//2. Create connections now that every node is created
	for (auto pTriangle : m_pNavMeshPolygon->GetTriangles())
	{
		std::vector<NavGraphNode*> pNodes{};
		for (auto pEdge : pTriangle->metaData.IndexLines)
		{
			for (auto pNode : m_Nodes)
			{
				if (pNode->GetLineIndex() == pEdge)
				{
					pNodes.push_back(pNode);
				}
			}
		}
		if (pNodes.size() == 2)
		{
			Elite::GraphConnection2D* pNewConnection{ new Elite::GraphConnection2D{pNodes[0]->GetIndex(),pNodes[1]->GetIndex()} };
			AddConnection(pNewConnection);
		}
		else if (pNodes.size() == 3)
		{
			Elite::GraphConnection2D* pNewConnection{ new Elite::GraphConnection2D{pNodes[0]->GetIndex(),pNodes[1]->GetIndex()} };
			AddConnection(pNewConnection);
			pNewConnection = new Elite::GraphConnection2D{ pNodes[1]->GetIndex(),pNodes[2]->GetIndex() };
			AddConnection(pNewConnection);
			pNewConnection = new Elite::GraphConnection2D{ pNodes[2]->GetIndex(),pNodes[0]->GetIndex() };
			AddConnection(pNewConnection);
		}
		//pNodes.clear();
	}
	//3. Set the connections cost to the actual distance
	SetConnectionCostsToDistance();
}

