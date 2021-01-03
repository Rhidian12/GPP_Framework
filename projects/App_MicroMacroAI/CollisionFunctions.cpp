#include "stdafx.h"
#include "CollisionFunctions.h"
bool Collisions::IsPointInRect(const Elite::Vector2& p, const Elite::Rect& r)
{
	return (p.x >= r.bottomLeft.x &&
		p.x <= r.bottomLeft.x + r.width &&
		p.y >= r.bottomLeft.y &&
		p.y <= r.bottomLeft.y + r.height);
}
bool Collisions::IsOverlapping(const Elite::Vector2& a, const Elite::Vector2& b, const Elite::Rect& r, HitInfo& hitInfo)
{
	// if one of the line segment end points is in the rect
	if (IsPointInRect(a, r) || IsPointInRect(b, r))
	{
		return true;
	}

	Elite::Vector2 vertices[]{ Elite::Vector2 {r.bottomLeft.x, r.bottomLeft.y},
		Elite::Vector2{ r.bottomLeft.x + r.width, r.bottomLeft.y },
		Elite::Vector2{ r.bottomLeft.x + r.width, r.bottomLeft.y + r.height },
		Elite::Vector2{ r.bottomLeft.x, r.bottomLeft.y + r.height } };

	return Raycast(vertices, 4, a, b, hitInfo);
}

bool Collisions::Raycast(const Elite::Vector2* vertices, const size_t nrVertices, const Elite::Vector2& rayP1, const Elite::Vector2& rayP2, HitInfo& hitInfo)
{
	if (nrVertices == 0)
	{
		return false;
	}

	std::vector<HitInfo> hits;

	Elite::Rect r1, r2;
	// r1: minimal AABB rect enclosing the ray
	r1.bottomLeft.x = min(rayP1.x, rayP2.x);
	r1.bottomLeft.y = min(rayP1.y, rayP2.y);
	r1.width = std::max(rayP1.x, rayP2.x) - r1.bottomLeft.x;
	r1.height = std::max(rayP1.y, rayP2.y) - r1.bottomLeft.y;

	// Line-line intersections.
	for (size_t idx{ 0 }; idx <= nrVertices; ++idx)
	{
		// Consider line segment between 2 consecutive vertices
		// (modulo to allow closed polygon, last - first vertice)
		Elite::Vector2 q1 = vertices[(idx + 0) % nrVertices];
		Elite::Vector2 q2 = vertices[(idx + 1) % nrVertices];

		// r2: minimal AABB rect enclosing the 2 vertices
		r2.bottomLeft.x = min(q1.x, q2.x);
		r2.bottomLeft.y = min(q1.y, q2.y);
		r2.width = std::max(q1.x, q2.x) - r2.bottomLeft.x;
		r2.height = std::max(q1.y, q2.y) - r2.bottomLeft.y;

		if (Elite::IsOverlapping(r1, r2))
		{
			float lambda1{};
			float lambda2{};
			if (IntersectLineSegments(rayP1, rayP2, q1, q2, lambda1, lambda2))
			{
				if (lambda1 > 0 && lambda1 <= 1 && lambda2 > 0 && lambda2 <= 1)
				{
					HitInfo linesHitInfo{};
					linesHitInfo.lambda = lambda1;
					linesHitInfo.intersectPoint = Elite::Vector2{ rayP1.x + ((rayP2.x - rayP1.x) * lambda1), rayP1.y + ((rayP2.y - rayP1.y) * lambda1) };
					linesHitInfo.normal = Elite::Vector2{ q2 - q1 }.Orthogonal().GetNormalized();
					hits.push_back(linesHitInfo);
				}
			}
		}
	}

	if (hits.size() == 0)
	{
		return false;
	}

	// Get closest intersection point and copy it into the hitInfo parameter
	hitInfo = *std::min_element
	(
		hits.begin(), hits.end(),
		[](const HitInfo& first, const HitInfo& last)
		{
			return first.lambda < last.lambda;
		}
	);
	return true;
}
bool Collisions::IntersectLineSegments(const Elite::Vector2& p1, const Elite::Vector2& p2, const Elite::Vector2& q1, const Elite::Vector2& q2, float& outLambda1, float& outLambda2, float epsilon)
{
	bool intersecting{ false };

	Elite::Vector2 p1p2{ p2 - p1 };
	Elite::Vector2 q1q2{ q2 - q1 };

	// Cross product to determine if parallel
	float denom = p1p2.Cross(q1q2);

	// Don't divide by zero
	if (std::abs(denom) > epsilon)
	{
		intersecting = true;

		Elite::Vector2 p1q1{ q1 - p1 };

		float num1 = p1q1.Cross(q1q2);
		float num2 = p1q1.Cross(p1p2);
		outLambda1 = num1 / denom;
		outLambda2 = num2 / denom;
	}
	else // are parallel
	{
		// Connect start points
		Elite::Vector2 p1q1{ q1 - p1 };

		// Cross product to determine if segments and the line connecting their start points are parallel, 
		// if so, than they are on a line
		// if not, then there is no intersection
		if (std::abs(p1q1.Cross(q1q2)) > epsilon)
		{
			return false;
		}

		// Check the 4 conditions
		outLambda1 = 0;
		outLambda2 = 0;
		if (IsPointOnLineSegment(p1, q1, q2) ||
			IsPointOnLineSegment(p2, q1, q2) ||
			IsPointOnLineSegment(q1, p1, p2) ||
			IsPointOnLineSegment(q2, p1, p2))
		{
			intersecting = true;
		}
	}
	return intersecting;
}
bool Collisions::IsPointOnLineSegment(const Elite::Vector2& p, const Elite::Vector2& a, const Elite::Vector2& b)
{
	Elite::Vector2 ap{ p - a }, bp{ p - b };
	// If not on same line, return false
	if (abs(ap.Cross(bp)) > 0.001f)
	{
		return false;
	}

	// Both vectors must point in opposite directions if p is between a and b
	if (ap.Dot(bp) > 0)
	{
		return false;
	}

	return true;
}