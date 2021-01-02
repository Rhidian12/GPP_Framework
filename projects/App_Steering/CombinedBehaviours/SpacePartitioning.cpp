#include "stdafx.h"
#include "SpacePartitioning.h"
#include "projects\App_Steering\SteeringAgent.h"

// --- Cell ---
// ------------
Cell::Cell(float left, float bottom, float width, float height)
{
	boundingBox.bottomLeft = { left, bottom };
	boundingBox.width = width;
	boundingBox.height = height;
}
bool Cell::operator==(const Cell& rhs) const
{
	if (boundingBox.bottomLeft == rhs.boundingBox.bottomLeft)
	{
		if (boundingBox.width == rhs.boundingBox.width && boundingBox.height == rhs.boundingBox.height)
		{
			return true;
		}
	}
	return false;
}

std::vector<Elite::Vector2> Cell::GetRectPoints() const
{
	auto left = boundingBox.bottomLeft.x;
	auto bottom = boundingBox.bottomLeft.y;
	auto width = boundingBox.width;
	auto height = boundingBox.height;

	std::vector<Elite::Vector2> rectPoints =
	{
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
		{ left , bottom  },
	};

	return rectPoints;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(float width, float height, int rows, int cols, int maxEntities)
	: m_SpaceWidth(width)
	, m_SpaceHeight(height)
	, m_NrOfRows(rows * 2)
	, m_NrOfCols(cols * 2)
	, m_Neighbors(maxEntities)
	, m_NrOfNeighbors{}
	, m_CellWidth{ width / cols }
	, m_CellHeight{ height / rows }
	, m_AmountOfCells{}
{
	for (int y{ -rows }; y < rows; ++y)
	{
		for (int x{ -cols }; x < cols; ++x)
		{
			m_Cells.push_back(Cell{ x * m_CellWidth,y * m_CellHeight,m_CellWidth,m_CellHeight });
			++m_AmountOfCells;
		}
	}
}

void CellSpace::AddAgent(SteeringAgent* pAgent)
{
	const Elite::Vector2 position{ pAgent->GetPosition() };
	const int index{ PositionToIndex(position) };
	m_Cells[index].agents.push_back(pAgent);
}

void CellSpace::UpdateAgentCell(SteeringAgent* pAgent, const Elite::Vector2& oldPos)
{
	const int oldCellIndex{ PositionToIndex(oldPos) };
	const int newCellIndex{ PositionToIndex(pAgent->GetPosition()) };
	if (oldCellIndex != newCellIndex)
	{
		// agent is no longer in the same cell
		// update agent to new cell
		m_Cells[oldCellIndex].agents.remove(pAgent);
		m_Cells[newCellIndex].agents.push_back(pAgent);
	}
}

void CellSpace::RegisterNeighbors(const Elite::Vector2& pos, float queryRadius)
{
	m_NrOfNeighbors = 0;

	const int cellIndex{ PositionToIndex(pos) };
	const float diameter{ 2 * queryRadius };
	const Elite::Rect radiusRect{ Elite::Vector2{pos.x - queryRadius,pos.y - queryRadius},diameter,diameter };

	const int amountOfCellsMax{ (int(queryRadius) / int(m_CellWidth)) + 1 };
	const float queryRadiusSquared{ queryRadius * queryRadius };

	for (int topRightAndBot{ -amountOfCellsMax }; topRightAndBot <= amountOfCellsMax; ++topRightAndBot)
	{
		for (int i{ 0 }; i <= amountOfCellsMax; ++i)
		{
			const int newIndex{ cellIndex + topRightAndBot * m_NrOfCols + i };
			Elite::Clamp(newIndex, 0, m_AmountOfCells - 1);

			const Elite::Vector2 bottonLeftPosition{ m_Cells[newIndex].boundingBox.bottomLeft };

			const Elite::Vector2 topLeftPosition{ m_Cells[newIndex].boundingBox.bottomLeft.x,
				m_Cells[newIndex].boundingBox.bottomLeft.y + m_Cells[newIndex].boundingBox.height };

			if (Elite::DistanceSquared(pos, bottonLeftPosition) <= queryRadiusSquared ||
				Elite::DistanceSquared(pos, topLeftPosition) <= queryRadiusSquared)
			{
				for (SteeringAgent* pAgent : m_Cells[newIndex].agents)
				{
					const Elite::Vector2 agentPosition{ pAgent->GetPosition() };
					if (Elite::DistanceSquared(agentPosition, pos) < queryRadiusSquared)
					{
						m_Neighbors[m_NrOfNeighbors] = pAgent;
						++m_NrOfNeighbors;
					}
				}
			}
		}
	}
	for (int topLeftAndBot{ -amountOfCellsMax }; topLeftAndBot <= amountOfCellsMax; ++topLeftAndBot)
	{
		for (int i{ 0 }; i >= -amountOfCellsMax; --i)
		{
			const int newIndex{ cellIndex + topLeftAndBot * m_NrOfCols + i };
			Elite::Clamp(newIndex, 0, m_AmountOfCells - 1);

			const Elite::Vector2 topRightPosition{ m_Cells[newIndex].boundingBox.bottomLeft.x + m_Cells[newIndex].boundingBox.width,
			m_Cells[newIndex].boundingBox.bottomLeft.y + m_Cells[newIndex].boundingBox.height };

			const Elite::Vector2 rightBottomPosition{ m_Cells[newIndex].boundingBox.bottomLeft.x + m_Cells[newIndex].boundingBox.width,
			m_Cells[newIndex].boundingBox.bottomLeft.y };

			if (Elite::DistanceSquared(pos, topRightPosition) <= queryRadiusSquared ||
				Elite::DistanceSquared(pos, rightBottomPosition) <= queryRadiusSquared)
			{
				for (SteeringAgent* pAgent : m_Cells[newIndex].agents)
				{
					const Elite::Vector2 agentPosition{ pAgent->GetPosition() };
					if (Elite::DistanceSquared(agentPosition, pos) < queryRadiusSquared)
					{
						m_Neighbors[m_NrOfNeighbors] = pAgent;
						++m_NrOfNeighbors;
					}
				}
			}
		}
	}
}

void CellSpace::RenderCells() const
{
	std::vector<Elite::Vector2> vertices;
	for (const Cell& cell : m_Cells)
	{
		vertices = cell.GetRectPoints();
		DEBUGRENDERER2D->DrawPolygon(&vertices[0], 4, { 1,0,0 }, 0);
	}
}

void CellSpace::DebugNeighbourhood(SteeringAgent* pAgentToRender, const float queryRadius) const
{
	const int cellIndex{ PositionToIndex(pAgentToRender->GetPosition()) };
	const float diameter{ 2 * queryRadius };
	const Elite::Rect radiusRect{ Elite::Vector2{pAgentToRender->GetPosition().x - queryRadius,pAgentToRender->GetPosition().y - queryRadius},diameter,diameter };

	const int amountOfCellsMax{ (int(queryRadius) / int(m_CellWidth)) + 1 };
	const Elite::Vector2 agentPosition{ pAgentToRender->GetPosition() };
	const float queryRadiusSquared{ queryRadius * queryRadius };

	Cell cell{ 0,0,0,0 };
	std::vector<Elite::Vector2> vertices;

	for (int topRightAndBot{ -amountOfCellsMax }; topRightAndBot <= amountOfCellsMax; ++topRightAndBot)
	{
		for (int i{ 0 }; i <= amountOfCellsMax; ++i)
		{
			const int newIndex{ cellIndex + topRightAndBot * m_NrOfCols + i };

			const Elite::Vector2 bottonLeftPosition{ m_Cells[newIndex].boundingBox.bottomLeft };

			const Elite::Vector2 topLeftPosition{ m_Cells[newIndex].boundingBox.bottomLeft.x,
				m_Cells[newIndex].boundingBox.bottomLeft.y + m_Cells[newIndex].boundingBox.height };

			if (Elite::DistanceSquared(agentPosition, bottonLeftPosition) <= queryRadiusSquared ||
				Elite::DistanceSquared(agentPosition, topLeftPosition) <= queryRadiusSquared)
			{
				cell.boundingBox = m_Cells[cellIndex + topRightAndBot * m_NrOfCols + i].boundingBox;
				vertices = cell.GetRectPoints();
				DEBUGRENDERER2D->DrawPolygon(&vertices[0], 4, { 0,0,1 }, -1);
			}
		}
	}
	for (int topLeftAndBot{ -amountOfCellsMax }; topLeftAndBot <= amountOfCellsMax; ++topLeftAndBot)
	{
		for (int i{ 0 }; i >= -amountOfCellsMax; --i)
		{
			const int newIndex{ cellIndex + topLeftAndBot * m_NrOfCols + i };

			const Elite::Vector2 topRightPosition{ m_Cells[newIndex].boundingBox.bottomLeft.x + m_Cells[newIndex].boundingBox.width,
			m_Cells[newIndex].boundingBox.bottomLeft.y + m_Cells[newIndex].boundingBox.height };

			const Elite::Vector2 rightBottomPosition{ m_Cells[newIndex].boundingBox.bottomLeft.x + m_Cells[newIndex].boundingBox.width,
			m_Cells[newIndex].boundingBox.bottomLeft.y };

			if (Elite::DistanceSquared(agentPosition, topRightPosition) <= queryRadiusSquared ||
				Elite::DistanceSquared(agentPosition, rightBottomPosition) <= queryRadiusSquared)
			{
				cell.boundingBox = m_Cells[cellIndex + topLeftAndBot * m_NrOfCols + i].boundingBox;
				vertices = cell.GetRectPoints();
				DEBUGRENDERER2D->DrawPolygon(&vertices[0], 4, { 0,0,1 }, -1);
			}
		}
	}
}

int CellSpace::PositionToIndex(const Elite::Vector2& pos) const
{
	// This function assumes cellwidth == cellheight 
	// and that rows == columns
	int col{ int(pos.x + m_SpaceWidth) / int(m_CellWidth) };
	int row{ int(pos.y + m_SpaceHeight) / int(m_CellHeight) };

	col = Elite::Clamp(col, 0, m_NrOfCols - 1);
	row = Elite::Clamp(row, 0, m_NrOfRows - 1);

	return row * m_NrOfCols + col;
}