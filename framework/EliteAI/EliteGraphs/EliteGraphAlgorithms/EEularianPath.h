#pragma once
#include <stack>

namespace Elite
{
	enum class Eulerianity
	{
		notEulerian,
		semiEulerian,
		eulerian,
	};

	template <class T_NodeType, class T_ConnectionType>
	class EulerianPath
	{
	public:

		EulerianPath(IGraph<T_NodeType, T_ConnectionType>* pGraph);

		Eulerianity IsEulerian() const;
		vector<T_NodeType*> FindPath(Eulerianity& eulerianity) const;

	private:
		void VisitAllNodesDFS(int startIdx, vector<bool>& visited) const;
		bool IsConnected() const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
	};

	template<class T_NodeType, class T_ConnectionType>
	inline EulerianPath<T_NodeType, T_ConnectionType>::EulerianPath(IGraph<T_NodeType, T_ConnectionType>* pGraph)
		: m_pGraph(pGraph)
	{
	}

	template<class T_NodeType, class T_ConnectionType>
	inline Eulerianity EulerianPath<T_NodeType, T_ConnectionType>::IsEulerian() const
	{
		// If the graph is not connected, there can be no Eulerian Trail
		if (!IsConnected())
			return Eulerianity::notEulerian;

		// Count nodes with odd degree 
		const int nrOfNodes{ m_pGraph->GetNrOfNodes() };
		int oddCount{};
		for (int i{}; i < nrOfNodes; ++i)
		{
			if (m_pGraph->IsNodeValid(i) && (m_pGraph->GetNodeConnections(i).size() & 1))
				++oddCount;
		}

		// A connected graph with more than 2 nodes with an odd degree (an odd amount of connections) is not Eulerian
		if (oddCount > 2)
			return Eulerianity::notEulerian;

		// A connected graph with exactly 2 nodes with an odd degree is Semi-Eulerian (an Euler trail can be made, but only starting and ending in these 2 nodes)
		else if (oddCount == 2 && nrOfNodes != 2)
			return Eulerianity::semiEulerian;

		// A connected graph with no odd nodes is Eulerian
		return Eulerianity::eulerian;

	}

	template<class T_NodeType, class T_ConnectionType>
	inline vector<T_NodeType*> EulerianPath<T_NodeType, T_ConnectionType>::FindPath(Eulerianity& eulerianity) const
	{
		// Get a copy of the graph because this algorithm involves removing edges
		auto graphCopy = m_pGraph->Clone();
		const int nrOfNodes = graphCopy->GetNrOfNodes();
		auto path = vector<T_NodeType*>();
		auto stack = vector<int>();

		// algorithm...

		eulerianity = IsEulerian(); // assume a bunch of monkeys are using this program and haven't determined eulerianity

		if (eulerianity == Eulerianity::notEulerian)
			return path;

		// it is an undirected graph
		bool areAllNodesEvenDegreed{ true };
		int index{};
		int amountOfOddDegreedNodes{};
		for (int i{}; i < nrOfNodes; ++i)
		{
			if (graphCopy->GetNodeConnections(i).size() != 0 && graphCopy->GetNodeConnections(i).size() % 2 != 0)
			{
				areAllNodesEvenDegreed = false;
				++amountOfOddDegreedNodes;
				index = graphCopy->GetAllNodes()[i]->GetIndex();
			}
		}
		if (amountOfOddDegreedNodes != 0 && amountOfOddDegreedNodes != 2)
			return path;

		if (areAllNodesEvenDegreed)
			index = graphCopy->GetAllNodes()[0]->GetIndex();

		bool isAlgorithmCompleted{};
		while (!isAlgorithmCompleted)
		{
			if (graphCopy->GetNodeConnections(index).size() > 0) // does this node have neighbours
			{
				stack.push_back(index); // add node to stack
				size_t neighbourToGoTo{};
				for (const T_ConnectionType* pConnection : graphCopy->GetNodeConnections(index)) // go over all the nodes connections
				{
					if (graphCopy->GetNodeConnections(pConnection->GetTo()).size() > neighbourToGoTo) // how many neighbours does neighbouring node have
					{
						neighbourToGoTo = pConnection->GetTo();
					}
				}
				graphCopy->RemoveConnection(index, neighbourToGoTo);
				index = neighbourToGoTo;
			}
			else if (graphCopy->GetNodeConnections(index).size() == 0 && stack.size() == 0)
			{
				isAlgorithmCompleted = true; // end algorithm
				path.push_back(m_pGraph->GetNode(index));
			}
			else if (graphCopy->GetNodeConnections(index).size() == 0)
			{
				// no neighbours
				path.push_back(m_pGraph->GetNode(index)); // add node to path
				index = stack[stack.size() - 1]; // set current node as last node in the stack
				stack.pop_back(); // remove last node from stack
			}
		}

		// sort list end->start to start->end
		std::reverse(path.begin(), path.end());
		return path;
	}

	template<class T_NodeType, class T_ConnectionType>
	inline void EulerianPath<T_NodeType, T_ConnectionType>::VisitAllNodesDFS(int startIdx, vector<bool>& visited) const
	{
		// mark the visited node
		visited[startIdx] = true;

		// recursively visit any valid connected nodes that were not visited before
		for (T_ConnectionType* connection : m_pGraph->GetNodeConnections(startIdx))
		{
			if (m_pGraph->IsNodeValid(connection->GetTo()) && !visited[connection->GetTo()])
			{
				VisitAllNodesDFS(connection->GetTo(), visited);
			}
		}
	}

	template<class T_NodeType, class T_ConnectionType>
	inline bool EulerianPath<T_NodeType, T_ConnectionType>::IsConnected() const
	{
		const int nrOfNodes{ m_pGraph->GetNrOfNodes() };
		std::vector<bool> visitedNodes(nrOfNodes, false);

		// find a valid starting node that has connections
		int connectedIndex{ invalid_node_index };
		for (int i{}; i < nrOfNodes; ++i)
		{
			if (m_pGraph->IsNodeValid(i))
			{
				if (m_pGraph->GetNodeConnections(i).size() != 0)
				{
					connectedIndex = i;
					break;
				}
				else
					return false;
			}
		}
		// if no valid node could be found, return false
		if (connectedIndex == invalid_node_index)
			return false;

		// start a depth-first-search traversal from a node that has connections
		VisitAllNodesDFS(connectedIndex, visitedNodes);

		// if a node was never visited, this graph is not connected
		for (int i{}; i < nrOfNodes; ++i)
		{
			if (m_pGraph->IsNodeValid(i) && !visitedNodes[i])
				return false;
		}

		return true;
	}
}