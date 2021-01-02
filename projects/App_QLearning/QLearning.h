#pragma once
#include "FMatrix.h"
#include "framework/EliteMath/EVector2.h"

class QLearning
{
public:
	QLearning(int nrOfLocations, int startIndex, int endIndex);
	~QLearning() {
		SAFE_DELETE(m_pRewardMatrix);
		SAFE_DELETE(m_pQMatrix);
		if (m_pIndexBuffer) {
			delete[] m_pIndexBuffer;
			m_pIndexBuffer = 0;
		}
	}

	void SetNodeLocations(int index, Elite::Vector2 location);
	void SetKoboldLocation(const int location);
	void SetTreasureLocation(const int location);
	void AddConnection(int from, int to);

	void Train();
	void Render(float deltaTime);

	void PrintRewardMatrix() {
		m_pRewardMatrix->Print();
	}

	void PrintQMatrix() {
		m_pQMatrix->Print();
	}

	void PrintKoboldMatrix() {
		m_pKoboldMatrix->Print();
	}

	void PrintTreasureMatrix() {
		m_pTreasureMatrix->Print();
	}

	void PrintEnviromentMatrix() {
		m_pEnviromentMatrix->Print();
	}

protected:
	int SelectAction(int currentLoc);
	int SelectActionWithEnviroment(const int currentLoc);
	float Update(int currentLoc, int nextAction);
private:
	int m_NrOfLocations;
	int m_StartIndex;
	int m_EndIndex;
	float m_Gamma{ 0.8f };
	int m_NrOfIterations{ 700 };
	int m_CurrentIteration{ 0 };
	std::vector<Elite::Vector2> m_Locations;
	std::vector<int> m_KoboldLocations;
	std::vector<int> m_TreasureLocations;
	FMatrix* m_pRewardMatrix{ 0 };
	FMatrix* m_pQMatrix{ 0 };
	FMatrix* m_pTreasureMatrix{ 0 };
	FMatrix* m_pKoboldMatrix{ 0 };
	FMatrix* m_pEnviromentMatrix{ 0 };
	FMatrix* m_pSubtractedMatrix{ 0 };
	int* m_pIndexBuffer{ 0 };

	// colors
	Elite::Color m_NormalColor{ 0.f,1.f,1.f,1.f };
	Elite::Color m_StartColor{ 0.f, 1.f, 0.f, 1.f };
	Elite::Color m_EndColor{ 1.f,0.f,0.f,1.f };
	Elite::Color m_ConnectionColor{ 1.f,1.f,0.f,1.f };
	Elite::Color m_NoQConnection{ 0.5f,0.f,0.f,1.f };
	Elite::Color m_MaxQConnection{ 0.1f,1.f,1.f,1.f };

	//C++ make the class non-copyable
	QLearning(const QLearning&) = delete;
	QLearning& operator=(const QLearning&) = delete;
};

