//=== General Includes ===
#include "stdafx.h"
#include "NavigationColliderElement.h"

//=== Constructor & Destructor ===
NavigationColliderElement::NavigationColliderElement(const Elite::Vector2& position, float width, float height) :
	m_Position(position), m_Width(width), m_Height(height)
{
	//Create Rigidbody
	const Elite::RigidBodyDefine define = Elite::RigidBodyDefine(0.01f, 0.1f, Elite::eStatic, false);
	const Transform transform = Transform(position, Elite::ZeroVector2);
	m_pRigidBody = new RigidBody(define, transform, PhysicsFlags::NavigationCollider);

	//Add shape
	Elite::EPhysicsBoxShape shape;
	shape.width = width;
	shape.height = height;
	m_pRigidBody->AddShape(&shape);
}
NavigationColliderElement::NavigationColliderElement(const Elite::Vector2& position, float width, float height, float rotation) :
	m_Position(position), m_Width(width), m_Height(height)
{
	//Create Rigidbody
	const Elite::RigidBodyDefine define = Elite::RigidBodyDefine(0.01f, 0.1f, Elite::eStatic, false);
	const Transform transform = Transform(position, Elite::Vector2{ rotation,0 });
	m_pRigidBody = new RigidBody(define, transform, PhysicsFlags::NavigationCollider);

	//Add shape
	Elite::EPhysicsBoxShape shape;
	shape.width = width;
	shape.height = height;
	m_pRigidBody->AddShape(&shape);
}

NavigationColliderElement::~NavigationColliderElement()
{
	SAFE_DELETE(m_pRigidBody);
}

const Elite::Vector2& NavigationColliderElement::GetPosition() const
{
	return m_Position;
}

const float NavigationColliderElement::GetWidth() const
{
	return m_Width;
}

const float NavigationColliderElement::GetHeight() const
{
	return m_Height;
}

//=== Functions ===
void NavigationColliderElement::RenderElement()
{
	//Do Nothing
}