/*
* Copyright (c) 2006-2007 Erin Catto http://www.gphysics.com
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#include "b2Shape.h"
#include "../Dynamics/b2Body.h"
#include "../Dynamics/b2World.h"
#include "../Common/b2BlockAllocator.h"

#include <new>

// Polygon mass, centroid, and inertia.
// Let rho be the polygon density in mass per unit area.
// Then:
// mass = rho * int(dA)
// centroid.x = (1/mass) * rho * int(x * dA)
// centroid.y = (1/mass) * rho * int(y * dA)
// I = rho * int((x*x + y*y) * dA)
//
// We can compute these integrals by summing all the integrals
// for each triangle of the polygon. To evaluate the integral
// for a single triangle, we make a change of variables to
// the (u,v) coordinates of the triangle:
// x = x0 + e1x * u + e2x * v
// y = y0 + e1y * u + e2y * v
// where 0 <= u && 0 <= v && u + v <= 1.
//
// We integrate u from [0,1-v] and then v from [0,1].
// We also need to use the Jacobian of the transformation:
// D = cross(e1, e2)
//
// Simplification: triangle centroid = (1/3) * (p1 + p2 + p3)
//
// The rest of the derivation is handled by computer algebra.
static void PolyMass(b2MassData* massData, const b2Vec2* vs, int32 count, float32 rho)
{
	b2Assert(count >= 3);

	b2Vec2 center; center.Set(0.0f, 0.0f);
	float32 area = 0.0f;
	float32 I = 0.0f;

	// pRef is the reference point for forming triangles.
	// It's location doesn't change the result (except for rounding error).
	b2Vec2 pRef(0.0f, 0.0f);
#if 0
	// This code would put the reference point inside the polygon.
	for (int32 i = 0; i < count; ++i)
	{
		pRef += vs[i];
	}
	pRef *= 1.0f / count;
#endif

	const float32 inv3 = 1.0f / 3.0f;

	for (int32 i = 0; i < count; ++i)
	{
		// Triangle vertices.
		b2Vec2 p1 = pRef;
		b2Vec2 p2 = vs[i];
		b2Vec2 p3 = i + 1 < count ? vs[i+1] : vs[0];

		b2Vec2 e1 = p2 - p1;
		b2Vec2 e2 = p3 - p1;

		float32 D = b2Cross(e1, e2);

		float32 triangleArea = 0.5f * D;
		area += triangleArea;

		// Area weighted centroid
		center += triangleArea * inv3 * (p1 + p2 + p3);

		float32 px = p1.x, py = p1.y;
		float32 ex1 = e1.x, ey1 = e1.y;
		float32 ex2 = e2.x, ey2 = e2.y;

		float32 intx2 = inv3 * (0.25f * (ex1*ex1 + ex2*ex1 + ex2*ex2) + (px*ex1 + px*ex2)) + 0.5f*px*px;
		float32 inty2 = inv3 * (0.25f * (ey1*ey1 + ey2*ey1 + ey2*ey2) + (py*ey1 + py*ey2)) + 0.5f*py*py;

		I += D * (intx2 + inty2);
	}

	// Total mass
	massData->mass = rho * area;

	// Center of mass
	b2Assert(area > FLT_EPSILON);
	center *= 1.0f / area;
	massData->center = center;

	// Inertia tensor relative to the center.
	I = rho * (I - area * b2Dot(center, center));
	massData->I = I;
}

static b2Vec2 PolyCentroid(const b2Vec2* vs, int32 count)
{
	b2Assert(count >= 3);

	b2Vec2 c; c.Set(0.0f, 0.0f);
	float32 area = 0.0f;

	// pRef is the reference point for forming triangles.
	// It's location doesn't change the result (except for rounding error).
	b2Vec2 pRef(0.0f, 0.0f);
#if 0
	// This code would put the reference point inside the polygon.
	for (int32 i = 0; i < count; ++i)
	{
		pRef += vs[i];
	}
	pRef *= 1.0f / count;
#endif

	const float32 inv3 = 1.0f / 3.0f;

	for (int32 i = 0; i < count; ++i)
	{
		// Triangle vertices.
		b2Vec2 p1 = pRef;
		b2Vec2 p2 = vs[i];
		b2Vec2 p3 = i + 1 < count ? vs[i+1] : vs[0];

		b2Vec2 e1 = p2 - p1;
		b2Vec2 e2 = p3 - p1;

		float32 D = b2Cross(e1, e2);

		float32 triangleArea = 0.5f * D;
		area += triangleArea;

		// Area weighted centroid
		c += triangleArea * inv3 * (p1 + p2 + p3);
	}

	// Centroid
	b2Assert(area > FLT_EPSILON);
	c *= 1.0f / area;
	return c;
}

void b2ShapeDef::ComputeMass(b2MassData* massData) const
{
	if (density == 0.0f)
	{
		massData->mass = 0.0f;
		massData->center.Set(0.0f, 0.0f);
		massData->I = 0.0f;
	}

	switch (type)
	{
	case e_circleShape:
		{
			b2CircleDef* circle = (b2CircleDef*)this;
			massData->mass = density * b2_pi * circle->radius * circle->radius;
			massData->center.Set(0.0f, 0.0f);
			massData->I = 0.5f * (massData->mass) * circle->radius * circle->radius;
		}
		break;

	case e_boxShape:
		{
			b2BoxDef* box = (b2BoxDef*)this;
			massData->mass = 4.0f * density * box->extents.x * box->extents.y;
			massData->center.Set(0.0f, 0.0f);
			massData->I = massData->mass / 3.0f * b2Dot(box->extents, box->extents);
		}
		break;

	case e_polyShape:
		{
			b2PolyDef* poly = (b2PolyDef*)this;
			PolyMass(massData, poly->vertices, poly->vertexCount, density);
		}
		break;

	default:
		massData->mass = 0.0f;
		massData->center.Set(0.0f, 0.0f);
		massData->I = 0.0f;
		break;
	}
}

b2Shape* b2Shape::Create(const b2ShapeDef* def,
					 b2Body* body, const b2Vec2& center)
{
	switch (def->type)
	{
	case e_circleShape:
		{
			void* mem = body->m_world->m_blockAllocator.Allocate(sizeof(b2CircleShape));
			return new (mem) b2CircleShape(def, body, center);
		}

	case e_boxShape:
	case e_polyShape:
		{
			void* mem = body->m_world->m_blockAllocator.Allocate(sizeof(b2PolyShape));
			return new (mem) b2PolyShape(def, body, center);
		}
	}

	b2Assert(false);
	return NULL;
}

void b2Shape::Destroy(b2Shape*& shape)
{
	b2BlockAllocator& allocator = shape->m_body->m_world->m_blockAllocator;
	shape->~b2Shape();

	switch (shape->m_type)
	{
	case e_circleShape:
		allocator.Free(shape, sizeof(b2CircleShape));
		break;

	case e_polyShape:
		allocator.Free(shape, sizeof(b2PolyShape));
		break;

	default:
		b2Assert(false);
	}

	shape = NULL;
}

b2Shape::b2Shape(const b2ShapeDef* def, b2Body* body)
{
	m_userData = def->userData;
	m_friction = def->friction;
	m_restitution = def->restitution;
	m_body = body;

	m_proxyId = b2_nullProxy;
	m_maxRadius = 0.0f;

	m_categoryBits = def->categoryBits;
	m_maskBits = def->maskBits;
	m_groupIndex = def->groupIndex;
}

b2Shape::~b2Shape()
{
	if (m_proxyId != b2_nullProxy)
	{
		m_body->m_world->m_broadPhase->DestroyProxy(m_proxyId);
	}
}

void b2Shape::DestroyProxy()
{
	if (m_proxyId != b2_nullProxy)
	{
		m_body->m_world->m_broadPhase->DestroyProxy(m_proxyId);
		m_proxyId = b2_nullProxy;
	}
}

b2CircleShape::b2CircleShape(const b2ShapeDef* def, b2Body* body, const b2Vec2& localCenter)
: b2Shape(def, body)
{
	b2Assert(def->type == e_circleShape);
	const b2CircleDef* circle = (const b2CircleDef*)def;

	m_localPosition = def->localPosition - localCenter;
	m_type = e_circleShape;
	m_radius = circle->radius;

	m_R = m_body->m_R;
	b2Vec2 r = b2Mul(m_body->m_R, m_localPosition);
	m_position = m_body->m_position + r;
	m_maxRadius = r.Length() + m_radius;
	m_minRadius = m_radius;

	b2AABB aabb;
	aabb.minVertex.Set(m_position.x - m_radius, m_position.y - m_radius);
	aabb.maxVertex.Set(m_position.x + m_radius, m_position.y + m_radius);

	b2BroadPhase* broadPhase = m_body->m_world->m_broadPhase;
	if (broadPhase->InRange(aabb))
	{
		m_proxyId = broadPhase->CreateProxy(aabb, this);
	}
	else
	{
		m_proxyId = b2_nullProxy;
	}

	if (m_proxyId == b2_nullProxy)
	{
		m_body->Freeze();
	}
}

void b2CircleShape::Synchronize(const b2Vec2& position1, const b2Mat22& R1,
								const b2Vec2& position2, const b2Mat22& R2)
{
	m_R = R2;
	m_position = position2 + b2Mul(m_R, m_localPosition);

	if (m_proxyId == b2_nullProxy)
	{	
		return;
	}

	// Compute an AABB that covers the swept shape (may miss some rotation effect).
	b2Vec2 p1 = position1 + b2Mul(R1, m_localPosition);
	b2Vec2 lower = b2Min(p1, m_position);
	b2Vec2 upper = b2Max(p1, m_position);

	b2AABB aabb;
	aabb.minVertex.Set(lower.x - m_radius, lower.y - m_radius);
	aabb.maxVertex.Set(upper.x + m_radius, upper.y + m_radius);

	b2BroadPhase* broadPhase = m_body->m_world->m_broadPhase;
	if (broadPhase->InRange(aabb))
	{
		broadPhase->MoveProxy(m_proxyId, aabb);
	}
	else
	{
		m_body->Freeze();
	}
}

void b2CircleShape::QuickSync(const b2Vec2& position, const b2Mat22& R)
{
	m_R = R;
	m_position = position + b2Mul(R, m_localPosition);
}

b2Vec2 b2CircleShape::Support(const b2Vec2& d) const
{
	b2Vec2 u = d;
	u.Normalize();
	float32 r = b2Max(0.0f, m_radius - b2_toiSlop);
	return m_position + r * u;
}

bool b2CircleShape::TestPoint(const b2Vec2& p)
{
	b2Vec2 d = p - m_position;
	return b2Dot(d, d) <= m_radius * m_radius;
}

void b2CircleShape::ResetProxy(b2BroadPhase* broadPhase)
{
	if (m_proxyId == b2_nullProxy)
	{	
		return;
	}

	b2Proxy* proxy = broadPhase->GetProxy(m_proxyId);

	broadPhase->DestroyProxy(m_proxyId);
	proxy = NULL;

	b2AABB aabb;
	aabb.minVertex.Set(m_position.x - m_radius, m_position.y - m_radius);
	aabb.maxVertex.Set(m_position.x + m_radius, m_position.y + m_radius);

	if (broadPhase->InRange(aabb))
	{
		m_proxyId = broadPhase->CreateProxy(aabb, this);
	}
	else
	{
		m_proxyId = b2_nullProxy;
	}

	if (m_proxyId == b2_nullProxy)
	{
		m_body->Freeze();
	}
}




b2PolyShape::b2PolyShape(const b2ShapeDef* def, b2Body* body,
					 const b2Vec2& newOrigin)
: b2Shape(def, body)
{
	b2Assert(def->type == e_boxShape || def->type == e_polyShape);
	m_type = e_polyShape;
	b2Mat22 localR(def->localRotation);

	// Get the vertices transformed into the body frame.
	if (def->type == e_boxShape)
	{
		m_localCentroid = def->localPosition - newOrigin;

		const b2BoxDef* box = (const b2BoxDef*)def;
		m_vertexCount = 4;
		b2Vec2 h = box->extents;
		m_vertices[0] = b2Mul(localR, b2Vec2(h.x, h.y));
		m_vertices[1] = b2Mul(localR, b2Vec2(-h.x, h.y));
		m_vertices[2] = b2Mul(localR, b2Vec2(-h.x, -h.y));
		m_vertices[3] = b2Mul(localR, b2Vec2(h.x, -h.y));
	}
	else
	{
		const b2PolyDef* poly = (const b2PolyDef*)def;
		m_vertexCount = poly->vertexCount;
		b2Assert(3 <= m_vertexCount && m_vertexCount <= b2_maxPolyVertices);
		b2Vec2 centroid = PolyCentroid(poly->vertices, poly->vertexCount);
		m_localCentroid = def->localPosition + b2Mul(localR, centroid) - newOrigin;
		for (int32 i = 0; i < m_vertexCount; ++i)
		{
			m_vertices[i] = b2Mul(localR, poly->vertices[i] - centroid);
		}
	}

	// Compute bounding box. TODO_ERIN optimize OBB
	b2Vec2 minVertex(FLT_MAX, FLT_MAX);
	b2Vec2 maxVertex(-FLT_MAX, -FLT_MAX);
	for (int32 i = 0; i < m_vertexCount; ++i)
	{
		b2Vec2 v = m_vertices[i];
		minVertex = b2Min(minVertex, v);
		maxVertex = b2Max(maxVertex, v);
	}

	m_localOBB.R.SetIdentity();
	m_localOBB.center = 0.5f * (minVertex + maxVertex);
	m_localOBB.extents = 0.5f * (maxVertex - minVertex);

	// Compute the edge normals and next index map.
	for (int32 i = 0; i < m_vertexCount; ++i)
	{
		int32 i1 = i;
		int32 i2 = i + 1 < m_vertexCount ? i + 1 : 0;
		b2Vec2 edge = m_vertices[i2] - m_vertices[i1];
		m_normals[i] = b2Cross(edge, 1.0f);
		m_normals[i].Normalize();
	}

	// Create core polygon shape by shifting edges inward.
	m_minRadius = FLT_MAX;
	m_maxRadius = -FLT_MAX;
	for (int32 i = 0; i < m_vertexCount; ++i)
	{
		int32 i1 = i - 1 >= 0 ? i - 1 : m_vertexCount - 1;
		int32 i2 = i;

		b2Vec2 n1 = m_normals[i1];
		b2Vec2 n2 = m_normals[i2];
		b2Vec2 v = m_vertices[i];

		// dot(n1, vc) = d.x
		// dot(n2, vc) = d.y
		b2Vec2 d;
		d.x = b2Dot(n1, v) - b2_toiSlop;
		d.y = b2Dot(n2, v) - b2_toiSlop;

		// Shifting the edge inward by b2_toiSlop should
		// not cause the plane to pass the centroid.
		b2Assert(d.x >= 0.0f);
		b2Assert(d.y >= 0.0f);
		b2Mat22 A;
		A.col1.x = n1.x; A.col2.x = n1.y;
		A.col1.y = n2.x; A.col2.y = n2.y;
		m_coreVertices[i] = A.Solve(d);
		
		m_minRadius = b2Min(m_minRadius, b2Min(d.x, d.y));
		
		b2Vec2 p = m_coreVertices[i] + m_localCentroid;
		m_maxRadius = b2Max(m_maxRadius, p.Length());
	}

	// Ensure the polygon in convex. TODO_ERIN compute convex hull.
	// TODO_ERIN check each vertex against each edge.
	for (int32 i = 0; i < m_vertexCount; ++i)
	{
		int32 i1 = i;
		int32 i2 = i + 1 < m_vertexCount ? i + 1 : 0;
		NOT_USED(i1);
		NOT_USED(i2);
		b2Assert(b2Cross(m_normals[i1], m_normals[i2]) > FLT_EPSILON);
	}

	m_R = m_body->m_R;
	m_position = m_body->m_position + b2Mul(m_body->m_R, m_localCentroid);

	b2Mat22 R = b2Mul(m_R, m_localOBB.R);
	b2Mat22 absR = b2Abs(R);
	b2Vec2 h = b2Mul(absR, m_localOBB.extents);
	b2Vec2 position = m_position + b2Mul(m_R, m_localOBB.center);
	b2AABB aabb;
	aabb.minVertex = position - h;
	aabb.maxVertex = position + h;

	b2BroadPhase* broadPhase = m_body->m_world->m_broadPhase;
	if (broadPhase->InRange(aabb))
	{
		m_proxyId = broadPhase->CreateProxy(aabb, this);
	}
	else
	{
		m_proxyId = b2_nullProxy;
	}

	if (m_proxyId == b2_nullProxy)
	{
		m_body->Freeze();
	}
}

void b2PolyShape::Synchronize(	const b2Vec2& position1, const b2Mat22& R1,
								const b2Vec2& position2, const b2Mat22& R2)
{
	// The body transform is copied for convenience.
	m_R = R2;
	m_position = position2 + b2Mul(R2, m_localCentroid);

	if (m_proxyId == b2_nullProxy)
	{	
		return;
	}

	b2AABB aabb1, aabb2;

	{
		b2Mat22 obbR = b2Mul(R1, m_localOBB.R);
		b2Mat22 absR = b2Abs(obbR);
		b2Vec2 h = b2Mul(absR, m_localOBB.extents);
		b2Vec2 center = position1 + b2Mul(R1, m_localCentroid + m_localOBB.center);
		aabb1.minVertex = center - h;
		aabb1.maxVertex = center + h;
	}

	{
		b2Mat22 obbR = b2Mul(R2, m_localOBB.R);
		b2Mat22 absR = b2Abs(obbR);
		b2Vec2 h = b2Mul(absR, m_localOBB.extents);
		b2Vec2 center = position2 + b2Mul(R2, m_localCentroid + m_localOBB.center);
		aabb2.minVertex = center - h;
		aabb2.maxVertex = center + h;
	}

	b2AABB aabb;
	aabb.minVertex = b2Min(aabb1.minVertex, aabb2.minVertex);
	aabb.maxVertex = b2Max(aabb1.maxVertex, aabb2.maxVertex);

	b2BroadPhase* broadPhase = m_body->m_world->m_broadPhase;
	if (broadPhase->InRange(aabb))
	{
		broadPhase->MoveProxy(m_proxyId, aabb);
	}
	else
	{
		m_body->Freeze();
	}
}

void b2PolyShape::QuickSync(const b2Vec2& position, const b2Mat22& R)
{
	m_R = R;
	m_position = position + b2Mul(R, m_localCentroid);
}

b2Vec2 b2PolyShape::Support(const b2Vec2& d) const
{
	b2Vec2 dLocal = b2MulT(m_R, d);

	int32 bestIndex = 0;
	float32 bestValue = b2Dot(m_coreVertices[0], dLocal);
	for (int32 i = 1; i < m_vertexCount; ++i)
	{
		float32 value = b2Dot(m_coreVertices[i], dLocal);
		if (value > bestValue)
		{
			bestIndex = i;
			bestValue = value;
		}
	}

	return m_position + b2Mul(m_R, m_coreVertices[bestIndex]);
}

bool b2PolyShape::TestPoint(const b2Vec2& p)
{
	b2Vec2 pLocal = b2MulT(m_R, p - m_position);

	for (int32 i = 0; i < m_vertexCount; ++i)
	{
		float32 dot = b2Dot(m_normals[i], pLocal - m_vertices[i]);
		if (dot > 0.0f)
		{
			return false;
		}
	}

	return true;
}

void b2PolyShape::ResetProxy(b2BroadPhase* broadPhase)
{
	if (m_proxyId == b2_nullProxy)
	{	
		return;
	}

	b2Proxy* proxy = broadPhase->GetProxy(m_proxyId);

	broadPhase->DestroyProxy(m_proxyId);
	proxy = NULL;

	b2Mat22 R = b2Mul(m_R, m_localOBB.R);
	b2Mat22 absR = b2Abs(R);
	b2Vec2 h = b2Mul(absR, m_localOBB.extents);
	b2Vec2 position = m_position + b2Mul(m_R, m_localOBB.center);
	b2AABB aabb;
	aabb.minVertex = position - h;
	aabb.maxVertex = position + h;

	if (broadPhase->InRange(aabb))
	{
		m_proxyId = broadPhase->CreateProxy(aabb, this);
	}
	else
	{
		m_proxyId = b2_nullProxy;
	}

	if (m_proxyId == b2_nullProxy)
	{
		m_body->Freeze();
	}
}


