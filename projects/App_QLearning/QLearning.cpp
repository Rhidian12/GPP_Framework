#include "stdafx.h"
#include "QLearning.h"
#include <stdio.h>

QLearning::QLearning(int nrOfLocations, int startIndex, int endIndex)
	:m_pRewardMatrix(new FMatrix(nrOfLocations, nrOfLocations)),
	m_pQMatrix(new FMatrix(nrOfLocations, nrOfLocations)),
	m_pTreasureMatrix{ new FMatrix{nrOfLocations,nrOfLocations} },
	m_pKoboldMatrix{ new FMatrix{nrOfLocations,nrOfLocations} },
	m_pEnviromentMatrix{ new FMatrix{nrOfLocations,nrOfLocations} },
	m_pSubtractedMatrix{ new FMatrix{nrOfLocations,nrOfLocations} },
	m_StartIndex(startIndex),
	m_EndIndex(endIndex),
	m_NrOfLocations(nrOfLocations),
	m_pIndexBuffer(new int[nrOfLocations])
{
	m_Locations.resize(nrOfLocations);
	m_pRewardMatrix->SetAll(-1.0f);
	m_pQMatrix->SetAll(0.0f);
	m_pTreasureMatrix->SetAll(0.f);
	m_pKoboldMatrix->SetAll(0.f);
	m_pEnviromentMatrix->SetAll(0.f);
	m_pSubtractedMatrix->SetAll(0.f);
	m_pRewardMatrix->Set(endIndex, endIndex, 100);
}

void QLearning::SetNodeLocations(int index, Elite::Vector2 location)
{
	if (index < m_NrOfLocations) {
		m_Locations[index] = location;
	}
}
void QLearning::SetKoboldLocation(const int location)
{
	if (location < m_NrOfLocations)
		m_KoboldLocations.push_back(location);
}
void QLearning::SetTreasureLocation(const int location)
{
	if (location < m_NrOfLocations)
		m_TreasureLocations.push_back(location);
}

void QLearning::AddConnection(int from, int to)
{
	// ----------------------------------------------
	// connections are set in the m_pRewardMatrix !!!
	// ----------------------------------------------
	// set two cells corresponding to (from,to) and (to,from) to 0
	// unless the to is equal to the end index, then the (from,to) should be 100.
	// use the set method of the fmatrix class
	if (to == m_EndIndex && from != m_EndIndex) // from->to == 100.f && to->from == 0.f
	{
		m_pRewardMatrix->Set(from, to, 100.f);
		m_pRewardMatrix->Set(to, from, 0.f);
	}
	else if (to == m_EndIndex && from == m_EndIndex) // from->to == 100.f
	{
		m_pRewardMatrix->Set(from, to, 100.f);
	}
	else // from->to == 0.f && to->from == 0.f
	{
		m_pRewardMatrix->Set(from, to, 0.f);
		m_pRewardMatrix->Set(to, from, 0.f);
	}
}

void QLearning::Render(float deltaTime)
{
	char buffer[10];
	Elite::Vector2 arrowPoints[3];
	for (int row = 0; row < m_pRewardMatrix->GetNrOfRows(); ++row)
	{
		for (int column = 0; column < m_pRewardMatrix->GetNrOfColumns(); ++column)
		{
			if (m_pRewardMatrix->Get(row, column) >= 0.f) {

				Elite::Vector2 start = m_Locations[row];
				Elite::Vector2 end = m_Locations[column];
				float length = start.Distance(end);


				Elite::Vector2 dir = end - start;
				dir.Normalize();
				Elite::Vector2 perpDir(dir.y, -dir.x);


				Elite::Vector2 tStart = start + perpDir * 2;
				Elite::Vector2 tEnd = end + perpDir * 2;

				Elite::Vector2 mid = (tEnd + tStart) * .5 + 5 * dir;

				arrowPoints[0] = mid + dir * 5;
				arrowPoints[1] = mid + perpDir * 1.5f;
				arrowPoints[2] = mid - perpDir * 1.5f;

				float qValue = m_pQMatrix->Get(row, column);
				float max = m_pQMatrix->Max();
				float ip = qValue / max;
				float ipOneMinus = 1 - ip;
				Elite::Color c;
				c.r = m_NoQConnection.r * ipOneMinus + m_MaxQConnection.r * ip;
				c.g = m_NoQConnection.g * ipOneMinus + m_MaxQConnection.g * ip;
				c.b = m_NoQConnection.b * ipOneMinus + m_MaxQConnection.b * ip;
				DEBUGRENDERER2D->DrawSegment(tStart, tEnd, c);
				DEBUGRENDERER2D->DrawSolidPolygon(&arrowPoints[0], 3, c, 0.5);
				snprintf(buffer, 10, "%.0f", qValue);
				DEBUGRENDERER2D->DrawString(mid + perpDir * 3, buffer);
			}
		}
	}

	int index = 0;


	for (Elite::Vector2 loc : m_Locations)
	{
		snprintf(buffer, 3, "%d", index);
		DEBUGRENDERER2D->DrawString(loc + Elite::Vector2(1.5f, 0), buffer);
		if (index == m_StartIndex)
		{
			DEBUGRENDERER2D->DrawSolidCircle(loc, 2.0f, Elite::Vector2(1, 0), m_StartColor, 0.5f);
		}
		else if (index == m_EndIndex) {
			DEBUGRENDERER2D->DrawSolidCircle(loc, 2.0f, Elite::Vector2(1, 0), m_EndColor, 0.5f);
		}
		else {
			DEBUGRENDERER2D->DrawSolidCircle(loc, 2.0f, Elite::Vector2(1, 0), m_NormalColor, 0.5f);
		}

		++index;
	}

}

int QLearning::SelectAction(int currentLocation)
{
	// Step 2 in the slides, select a to node via the reward matrix.
	// return this to-node as the result
	std::vector<int> possibleConnections{};
	const int amountOfColumns{ m_pRewardMatrix->GetNrOfColumns() };
	for (int i{}; i < amountOfColumns; ++i)
	{
		if (m_pRewardMatrix->Get(currentLocation, i) >= 0.f) // == It has to be either 100.f or 0.f, -1.f is invalid ==
		{
			possibleConnections.push_back(i);
		}
	}
	return possibleConnections[Elite::randomInt(possibleConnections.size())];
}
int QLearning::SelectActionWithEnviroment(const int currentLocation)
{
	std::vector<int> possibleConnections{};
	const int amountOfColumns{ m_pEnviromentMatrix->GetNrOfColumns() };
	for (int i{}; i < amountOfColumns; ++i)
	{
		if (m_pRewardMatrix->Get(currentLocation, i) >= 0.f)
		{
			if (m_pEnviromentMatrix->Get(currentLocation, i) >= 0.f)
			{
				possibleConnections.push_back(i);
			}
		}
	}
	if (possibleConnections.size() == 0)
		return SelectAction(currentLocation);
	return possibleConnections[Elite::randomInt(possibleConnections.size())];
}

float QLearning::Update(int currentLocation, int nextAction)
{
	// Check if the to node is a kobold / treasure
	for (const auto& location : m_KoboldLocations)
	{
		if (nextAction == location)
			m_pKoboldMatrix->Add(currentLocation, nextAction, 1);
	}
	for (const auto& location : m_TreasureLocations)
	{
		if (nextAction == location)
			m_pTreasureMatrix->Add(currentLocation, nextAction, 1);
	}
	m_pSubtractedMatrix->Copy(*m_pTreasureMatrix);
	m_pSubtractedMatrix->Subtract(*m_pKoboldMatrix);
	m_pEnviromentMatrix->Copy(*m_pSubtractedMatrix);

	// step 3 
	// A : get the max q-value of the row nextAction in the Q matrix
	float max = m_pQMatrix->MaxOfRow(nextAction);

	// B gather the elements that are equal to this max in an index buffer.
	// can use m_pIndexBuffer if it suits you. Devise your own way if you don't like it.
	std::vector<int> indexBuffer{};
	for (int i{}; i < m_pQMatrix->GetNrOfColumns(); ++i) // == Loop over the columns ==
	{
		if (Elite::AreEqual(m_pQMatrix->Get(nextAction, i), max)) // == Is the value in that {row,column} equal to the max of the row ==
		{
			indexBuffer.push_back(i); // == Save the column if it is ==
		}
	}
	const int randomColumn{ indexBuffer[Elite::randomInt(int(indexBuffer.size()))] };
	const float valueFromRandomColumn{ m_pQMatrix->Get(nextAction,randomColumn) };

	// C pick a random index from the index buffer and implement the update formula
	// for the q matrix. (slide 14)
	const float randomConnectionInRewardMatrix{ m_pRewardMatrix->Get(currentLocation,nextAction) };
	const float qUpdate{ randomConnectionInRewardMatrix + m_Gamma * valueFromRandomColumn };

	m_pQMatrix->Set(currentLocation, nextAction, qUpdate);

	// calculate the score of the q-matrix and return it. (slide 15)
	if (Elite::AreEqual(m_pQMatrix->Max(), 0.f))
	{
		return 0.f;
	}
	const float score{ 100.f * (m_pQMatrix->Sum() / m_pQMatrix->Max()) };
	return score;
}

void QLearning::Train() {
	if (m_CurrentIteration < m_NrOfIterations)
	{
		int currentLocation = Elite::randomInt(m_NrOfLocations);
		int nextLocation = SelectActionWithEnviroment(currentLocation);
		//int nextLocation = SelectAction(currentLocation);
		float score = Update(currentLocation, nextLocation);
		printf("Score %.2f\n", score);
		m_CurrentIteration++;
	}
	else if (m_CurrentIteration == m_NrOfIterations)
	{
		std::cout << "QMatrix: " << std::endl;
		m_pQMatrix->Print();
		std::cout << std::endl;
		std::cout << "Kobold Matrix: " << std::endl;
		m_pKoboldMatrix->Print();
		std::cout << std::endl;
		std::cout << "Treasure Matrix: " << std::endl;
		m_pTreasureMatrix->Print();
		//test from start point 0
		int location = m_StartIndex;

		printf("start at %d\t", location);

		// TODO : find the best path via the q-matrix.
		// uncomment the while loop when implementing, be careful for infinite loop.
		while (location != m_EndIndex)
		{
			//what is the maximum of the next action in the q-matrix
			const float max{ m_pQMatrix->MaxOfRow(location) };
			//gather the elements that are equal to this max in an index buffer.
			int amountOfElementsInIndexBuffer{};
			std::vector<int> tempIndexBuffer{};

			for (int c{}; c < m_NrOfLocations; ++c)
			{
				if (Elite::AreEqual(m_pQMatrix->Get(location, c), max))
				{
					tempIndexBuffer.push_back(c);
				}
			}
			location = tempIndexBuffer[Elite::randomInt(tempIndexBuffer.size())];
			//pick a random index from the index buffer.
			printf("%d\t", location);
		}
		m_CurrentIteration++;
	}
	//else if (m_CurrentIteration > m_NrOfIterations)
	//{
	//	m_CurrentIteration = 0;
	//}
}