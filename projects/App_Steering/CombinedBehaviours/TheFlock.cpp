#include "stdafx.h"
#include "TheFlock.h"

#include "../SteeringAgent.h"
#include "../SteeringBehaviors.h"
#include "CombinedSteeringBehaviors.h"
#include "SpacePartitioning.h"

using namespace Elite;

//Constructor & Destructor
Flock::Flock(
	int flockSize /*= 50*/,
	float worldSize /*= 100.f*/,
	SteeringAgent* pAgentToEvade /*= nullptr*/,
	bool trimWorld /*= false*/)

	: m_WorldSize{ worldSize }
	, m_FlockSize{ flockSize }
	, m_TrimWorld{ trimWorld }
	, m_pAgentToEvade{ pAgentToEvade }
	, m_NeighborhoodRadius{ 5.f }
	, m_NrOfNeighbors{ 0 }
	, m_MaxAmountOfNeighboursinRadius{}
	, m_IsSpatialPartitioningActive{}
{

	m_pSeek = new Seek{};
	m_pCohesion = new Cohesion{ this };
	m_pSeperation = new SeperationFlocking{ this };
	m_pAlignment = new Alignment{ this };
	m_pWander = new Wander{};
	m_pRunAway = new RunAway{ m_pAgentToEvade };


	m_pBlendedSteering = new BlendedSteering{ {{m_pCohesion,0.5f},{m_pSeperation,0.5f},
		{m_pAlignment,0.5f},{m_pSeek,0.5f},{m_pWander,0.5f}} };
	m_pPrioritySteering = new PrioritySteering{ { {m_pRunAway},{m_pBlendedSteering} } };

	m_Agents.reserve(m_FlockSize);
	m_Agents.resize(m_FlockSize);
	for (int i{}; i < m_FlockSize; ++i)
	{
		SteeringAgent* pAgent{ new SteeringAgent{} };
		pAgent->SetAutoOrient(true);
		//pAgent->SetSteeringBehavior(m_pBlendedSteering);
		pAgent->SetSteeringBehavior(m_pPrioritySteering);
		pAgent->SetMass(1.f);
		pAgent->SetBodyColor({ 0,1,0 });
		pAgent->SetMaxLinearSpeed(pAgent->GetMaxLinearSpeed() + 15);
		Elite::Vector2 position{ Elite::randomVector2(m_WorldSize) };
		pAgent->SetPosition(position);
		m_Agents[i] = pAgent;
	}
	m_Agents[0]->SetBodyColor({ 1,0,0 }); // now we can see who is the first agent

	m_Neighbors.reserve(m_FlockSize);
	m_Neighbors.resize(m_FlockSize);

	// WIDTH AND HEIGHT NEED TO BE EQUAL IN CELLSPACE, SAME FOR ROWS AND COLUMNS
	m_pCellSpace = new CellSpace{ m_WorldSize,m_WorldSize,25,25,m_FlockSize };
	for (SteeringAgent* pAgent : m_Agents)
	{
		m_pCellSpace->AddAgent(pAgent);
	}
	m_OldPositions.reserve(m_FlockSize);
	m_OldPositions.resize(m_FlockSize);

	for (int i{}; i < m_FlockSize; ++i)
	{
		m_OldPositions[i] = m_Agents[i]->GetPosition();
	}
}
Flock::~Flock()
{
	SAFE_DELETE(m_pSeek);
	SAFE_DELETE(m_pCohesion);
	SAFE_DELETE(m_pSeperation);
	SAFE_DELETE(m_pAlignment);
	SAFE_DELETE(m_pWander);
	SAFE_DELETE(m_pRunAway);
	m_pAgentToEvade = nullptr;
	SAFE_DELETE(m_pBlendedSteering);
	SAFE_DELETE(m_pPrioritySteering);
	SAFE_DELETE(m_pCellSpace);
	for (int i{}; i < m_FlockSize; ++i)
	{
		SAFE_DELETE(m_Agents[i]);
	}
	for (int i{}; i < m_FlockSize; ++i)
	{
		m_Neighbors[i] = nullptr;
	}
}

void Flock::Update(float deltaT)
{
	// loop over all the boids
	// register its neighbors
	// update it
	// trim it to the world
	for (int i{}; i < m_FlockSize; ++i)
	{
		if (!m_IsSpatialPartitioningActive)
		{
			RegisterNeighbors(m_Agents[i]);
		}
		if (m_IsSpatialPartitioningActive)
		{
			m_pCellSpace->RegisterNeighbors(m_Agents[i]->GetPosition(), m_NeighborhoodRadius);
			m_pCellSpace->UpdateAgentCell(m_Agents[i], m_OldPositions[i]);
			m_OldPositions[i] = m_Agents[i]->GetPosition();
		}
		m_Agents[i]->Update(deltaT);
		if (m_TrimWorld)
		{
			m_Agents[i]->TrimToWorld(m_WorldSize);
		}
	}
}

void Flock::Render(float deltaT)
{
	/*for (SteeringAgent* pAgent : m_Agents)
	{
		pAgent->Render(deltaT);
	}*/
	m_pCellSpace->RenderCells();
	if (m_CanDebugRender)
	{
		DEBUGRENDERER2D->DrawCircle(m_Agents[0]->GetPosition(), m_NeighborhoodRadius, { 0,1,0 }, 0);
		m_pCellSpace->DebugNeighbourhood(m_Agents[0], m_NeighborhoodRadius);
	}
}

void Flock::UpdateAndRenderUI()
{
	//Setup
	int menuWidth = 235;
	int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
	int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
	bool windowActive = true;
	ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
	ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
	ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::PushAllowKeyboardFocus(false);

	//Elements
	ImGui::Text("CONTROLS");
	ImGui::Indent();
	ImGui::Text("LMB: place target");
	ImGui::Text("RMB: move cam.");
	ImGui::Text("Scrollwheel: zoom cam.");
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

	ImGui::Text("Flocking");
	ImGui::Spacing();

	ImGui::Checkbox("Debug Rendering", &m_CanDebugRender);
	ImGui::Checkbox("Spatial Partitioning", &m_IsSpatialPartitioningActive);



	// Implement checkboxes and sliders here
	ImGui::Text("Behaviour Weights");
	ImGui::Spacing();

	ImGui::SliderFloat("Cohesion", &m_pBlendedSteering->m_WeightedBehaviors[0].weight, 0.f, 1.f, "%2");
	ImGui::SliderFloat("Seperation", &m_pBlendedSteering->m_WeightedBehaviors[1].weight, 0.f, 1.f, "%2");
	ImGui::SliderFloat("Alignment", &m_pBlendedSteering->m_WeightedBehaviors[2].weight, 0.f, 1.f, "%2");
	ImGui::SliderFloat("Seek", &m_pBlendedSteering->m_WeightedBehaviors[3].weight, 0.f, 1.f, "%2");
	ImGui::SliderFloat("Wander", &m_pBlendedSteering->m_WeightedBehaviors[4].weight, 0.f, 1.f, "%2");

	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();

}

void Flock::RegisterNeighbors(SteeringAgent* pAgent)
{
	// register the agents neighboring the currently evaluated agent
	// store how many they are, so you know which part of the vector to loop over
	m_NrOfNeighbors = 0;
	for (int i{}; i < m_FlockSize; ++i)
	{
		if (pAgent != m_Neighbors[i])
		{
			const float distanceToAgent{ Elite::Distance(pAgent->GetPosition(), m_Agents[i]->GetPosition()) };
			if (distanceToAgent < m_NeighborhoodRadius)
			{
				m_Neighbors[m_NrOfNeighbors] = m_Agents[i];
				++m_NrOfNeighbors;
			}
		}
	}
}

Elite::Vector2 Flock::GetAverageNeighborPos() const
{
	Elite::Vector2 averagePos{};
	if (m_IsSpatialPartitioningActive)
	{
		for (int i{}; i < m_pCellSpace->GetNrOfNeighbors(); ++i)
		{
			if (m_pCellSpace->GetNeighbors()[i] != nullptr)
			{
				averagePos += m_pCellSpace->GetNeighbors()[i]->GetPosition();
			}
		}
		return averagePos / float(m_pCellSpace->GetNrOfNeighbors());
	}
	else
	{
		for (int i{}; i < m_NrOfNeighbors; ++i)
		{
			if (m_Neighbors[i] != nullptr)
			{
				averagePos += m_Neighbors[i]->GetPosition();
			}
		}
		return averagePos / float(m_NrOfNeighbors);
	}
}

Elite::Vector2 Flock::GetAverageNeighborVelocity() const
{
	Elite::Vector2 averageVelocity{};
	if (m_IsSpatialPartitioningActive)
	{
		for (int i{}; i < m_pCellSpace->GetNrOfNeighbors(); ++i)
		{
			if (m_pCellSpace->GetNeighbors()[i] != nullptr)
			{
				averageVelocity += m_pCellSpace->GetNeighbors()[i]->GetLinearVelocity();
			}
		}
		return averageVelocity / float(m_pCellSpace->GetNrOfNeighbors());
	}
	else
	{
		for (int i{}; i < m_NrOfNeighbors; ++i)
		{
			if (m_Neighbors[i] != nullptr)
			{
				averageVelocity += m_Neighbors[i]->GetLinearVelocity();
			}
		}
		return averageVelocity / float(m_NrOfNeighbors);
	}
}


float* Flock::GetWeight(ISteeringBehavior* pBehavior)
{
	if (m_pBlendedSteering)
	{
		auto& weightedBehaviors = m_pBlendedSteering->m_WeightedBehaviors;
		auto it = find_if(weightedBehaviors.begin(),
			weightedBehaviors.end(),
			[pBehavior](BlendedSteering::WeightedBehavior el)
			{
				return el.pBehavior == pBehavior;
			}
		);

		if (it != weightedBehaviors.end())
			return &it->weight;
	}

	return nullptr;
}
void Flock::SetSeekTarget(const Elite::Vector2& position)
{
	m_pSeek->SetTarget(position);
}
bool Flock::GetCanRenderBehaviour() const
{
	return m_CanDebugRender;
}
const std::vector<SteeringAgent*>& Flock::GetAgents() const
{
	return m_Agents;
}
float Flock::GetNeighbourhoodRadius() const
{
	return m_NeighborhoodRadius;
}
bool Flock::GetIsSpatialPartitioningActive() const
{
	return m_IsSpatialPartitioningActive;
}
const CellSpace* Flock::GetCellSpace() const
{
	return m_pCellSpace;
}