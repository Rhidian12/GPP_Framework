#pragma once

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class AStar
	{
	public:
		AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);

		// stores the optimal connection to a node and its total costs related to the start and end node of the path
		struct NodeRecord
		{
			T_NodeType* pNode = nullptr;
			T_ConnectionType* pConnection = nullptr;
			float costSoFar = 0.f; // accumulated g-costs of all the connections leading up to this one
			float estimatedTotalCost = 0.f; // f-cost (= costSoFar + h-cost)

			bool operator==(const NodeRecord& other) const
			{
				return pNode == other.pNode
					&& pConnection == other.pConnection
					&& costSoFar == other.costSoFar
					&& estimatedTotalCost == other.estimatedTotalCost;
			};

			bool operator<(const NodeRecord& other) const
			{
				return estimatedTotalCost < other.estimatedTotalCost;
			};
			bool operator>(const NodeRecord& other) const
			{
				return estimatedTotalCost > other.estimatedTotalCost;
			};
		};

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);

	private:
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};

	template <class T_NodeType, class T_ConnectionType>
	AStar<T_NodeType, T_ConnectionType>::AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: m_pGraph(pGraph)
		, m_HeuristicFunction(hFunction)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> AStar<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pGoalNode)
	{
		float gCost{};
		NodeRecord startRecord{ pStartNode,nullptr,gCost,GetHeuristicCost(pStartNode,pGoalNode) }; // this is where we start

		NodeRecord currentRecord{}; // holds the current to be evaluated connection
		std::vector<T_NodeType*> path; // final path
		std::vector<NodeRecord> openList; // connections to be checked
		std::vector<NodeRecord> closedList; // connections already checked

		openList.push_back(startRecord); // jump start while loop

		while (!openList.empty()) // 2
		{
			currentRecord = *std::min_element(openList.begin(), openList.end());

			if (currentRecord.pNode == pGoalNode) // 2B
				break; // we have found the end node

			for (T_ConnectionType* pNodeConnection : m_pGraph->GetNodeConnections(currentRecord.pNode->GetIndex())) // 2C
			{
				T_NodeType* pNextNode{ m_pGraph->GetNode(pNodeConnection->GetTo()) }; // get the next node
				const float gCost{ currentRecord.costSoFar + pNodeConnection->GetCost() }; // calculate the gcost

				const size_t closedListSizeBefore{ closedList.size() };
				const size_t openListSizeBefore{ openList.size() };

				auto closedListIt = std::find_if(closedList.begin(), closedList.end(), [pNextNode](const NodeRecord& nodeRecord)->bool
					{
						return pNextNode == nodeRecord.pNode; // find if the next node is already in the closed list
					});
				if (closedListIt != closedList.end()) // check if something was found
				{
					if (gCost < closedListIt->costSoFar) // check if this connection is cheaper than the existing one
					{
						closedList.erase(closedListIt); // new connection is cheaper so erase the old one
					}
					else
					{
						auto openListIt = std::find_if(openList.begin(), openList.end(), [pNextNode](const NodeRecord& nodeRecord)->bool
							{
								return pNextNode == nodeRecord.pNode; // find if the next node is already in the open list
							});
						if (openListIt != openList.end()) // check if something was found
						{
							if (gCost < openListIt->costSoFar) // check if new connection is cheaper than the old one
							{
								openList.erase(openListIt); // new connection is cheaper so erase the old one
							}
						}
					}
				}

				NodeRecord newRecord{ pNextNode,pNodeConnection,gCost,GetHeuristicCost(pNextNode,pGoalNode) + gCost };
				//openList.push_back(newRecord); // 2F

				if (openListSizeBefore != openList.size() || closedListSizeBefore != closedList.size())
				{
					openList.push_back(currentRecord);
				}
				bool isNodeAlreadyInList{};
				for (const NodeRecord& nodeRecord : openList)
				{
					if (nodeRecord.pNode == newRecord.pNode)
					{
						isNodeAlreadyInList = true;
						break;
					}
				}
				if (!isNodeAlreadyInList)
				{
					for (const NodeRecord& nodeRecord : closedList)
					{
						if (nodeRecord.pNode == newRecord.pNode)
						{
							isNodeAlreadyInList = true;
							break;
						}
					}
				}
				if (!isNodeAlreadyInList)
				{
					openList.push_back(newRecord);
				}
			}
			// 2G
			auto openListIt = std::remove_if(openList.begin(), openList.end(), [&currentRecord](const NodeRecord& nodeRecord)->bool
				{
					return currentRecord == nodeRecord; // find where the current node is in the open list
				});

			openList.erase(openListIt, openList.end());
			closedList.push_back(currentRecord);
		}
		/*if (openList.empty())
			return path;*/
		while (currentRecord.pNode != pStartNode)
		{
			path.push_back(currentRecord.pNode);
			auto closedListIt = std::find_if(closedList.begin(), closedList.end(), [this, &currentRecord](const NodeRecord& nodeRecord)->bool
				{
					return m_pGraph->GetNode(currentRecord.pConnection->GetFrom()) == nodeRecord.pNode; // find if the next node is already in the open list
				});
			if (closedListIt != closedList.end()) // check if we found something
			{
				currentRecord = *closedListIt;
			}
		}
		path.push_back(pStartNode); // add startnode
		std::reverse(path.begin(), path.end()); // reverse path, since we got it backwards
		return path;
	}

	template <class T_NodeType, class T_ConnectionType>
	float Elite::AStar<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}
}