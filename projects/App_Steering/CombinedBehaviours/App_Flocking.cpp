//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_Flocking.h"
#include "../SteeringAgent.h"
#include "TheFlock.h"
#include "CombinedSteeringBehaviors.h"
#include "FlockingSteeringBehaviors.h"

//Destructor
App_Flocking::~App_Flocking()
{
	SAFE_DELETE(m_pAgentToAvoid);
	SAFE_DELETE(m_pFlock);
	SAFE_DELETE(m_pSeek);
}

//Functions
void App_Flocking::Start()
{
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(55.0f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(m_TrimWorldSize / 1.5f, m_TrimWorldSize / 2));
	m_pAgentToAvoid = new SteeringAgent{};
	m_pSeek = new Seek{};

	m_pAgentToAvoid->SetAutoOrient(true);
	m_pAgentToAvoid->SetSteeringBehavior(m_pSeek);
	m_pAgentToAvoid->SetMaxLinearSpeed(m_pAgentToAvoid->GetMaxLinearSpeed() + 15.f);
	m_pAgentToAvoid->SetMass(1.f);
	m_pAgentToAvoid->SetBodyColor({ 0,1,1 });

	m_pFlock = new Flock{ 4000,100.f,m_pAgentToAvoid,true };
}

void App_Flocking::Update(float deltaTime)
{
	//INPUT
	if (INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eLeft) && m_VisualizeMouseTarget)
	{
		auto const mouseData = INPUTMANAGER->GetMouseData(InputType::eMouseButton, InputMouseButton::eLeft);
		m_MouseTarget.Position = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y) });
	}
	m_pSeek->SetTarget(m_MouseTarget);
	m_pAgentToAvoid->Update(deltaTime);

	m_pFlock->SetSeekTarget(m_MouseTarget.Position);
	m_pFlock->Update(deltaTime);
	m_pFlock->UpdateAndRenderUI();
}

void App_Flocking::Render(float deltaTime) const
{
	bool canDebugRender{ m_pFlock->GetCanRenderBehaviour() };
	if (canDebugRender)
	{
		for (SteeringAgent* pAgent : m_pFlock->GetAgents())
		{
			pAgent->SetRenderBehavior(canDebugRender);
		}
	}
	std::vector<Elite::Vector2> points
	{
		{ -m_TrimWorldSize, m_TrimWorldSize },
		{ m_TrimWorldSize, m_TrimWorldSize },
		{ m_TrimWorldSize, -m_TrimWorldSize },
		{ -m_TrimWorldSize, -m_TrimWorldSize }
	};
	m_pFlock->Render(deltaTime);
	DEBUGRENDERER2D->DrawPolygon(&points[0], 4, { 1,0,0,1 }, 0.4f);
	m_pAgentToAvoid->Render(deltaTime);
	//Render Target
	if (m_VisualizeMouseTarget)
		DEBUGRENDERER2D->DrawSolidCircle(m_MouseTarget.Position, 0.3f, { 0.f,0.f }, { 1.f,0.f,0.f }, -0.8f);
}
