#pragma once
namespace Collisions
{
	struct HitInfo
	{
		float lambda;
		Elite::Vector2 intersectPoint;
		Elite::Vector2 normal;
	};

	bool IsPointInRect(const Elite::Vector2& p, const Elite::Rect& r);
	bool IsOverlapping(const Elite::Vector2& a, const Elite::Vector2& b, const Elite::Rect& r, HitInfo& hitInfo);
	bool Raycast(const Elite::Vector2* vertices, const size_t nrVertices, const Elite::Vector2& rayP1, const Elite::Vector2& rayP2, HitInfo& hitInfo);
	bool IntersectLineSegments(const Elite::Vector2& p1, const Elite::Vector2& p2, const Elite::Vector2& q1, const Elite::Vector2& q2, float& outLambda1, float& outLambda2, float epsilon = 1e-6);
	bool IsPointOnLineSegment(const Elite::Vector2& p, const Elite::Vector2& a, const Elite::Vector2& b);
}