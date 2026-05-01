#include "pch.h"
#include "CFrustum.h"
#include "CCamera.h"


CFrustum::CFrustum(CCamera* _Camera)
	: m_Owner(_Camera)
{
	// NOTE: frustum corners in NDC space
	// x -> [-1, 1]
	// y -> [-1, 1]
	// z -> [ 0, 1] DirectX(OpenGL是[-1,1])

	//   4 --- 5
	//  /|  \  | Far Plane
	// / 7 --/ 6
	// 0 -- 1/ /
	// | \  | / Near Plane
	// 3 -- 2
	m_arrNDCCorners[0] = Vec3(-1.f, 1.f, 0.f);  // near top left
	m_arrNDCCorners[1] = Vec3(1.f, 1.f, 0.f);   // near top right
	m_arrNDCCorners[2] = Vec3(1.f, -1.f, 0.f);  // near bottom right
	m_arrNDCCorners[3] = Vec3(-1.f, -1.f, 0.f); // near bottom left

	m_arrNDCCorners[4] = Vec3(-1.f, 1.f, 1.f);  // far top left
	m_arrNDCCorners[5] = Vec3(1.f, 1.f, 1.f);   // far top right
	m_arrNDCCorners[6] = Vec3(1.f, -1.f, 1.f);  // far bottom right
	m_arrNDCCorners[7] = Vec3(-1.f, -1.f, 1.f); // far bottom left
}


CFrustum::~CFrustum()
{
}

bool CFrustum::IsPointInFrustum(Vec3 _WorldPos)
{
	for (int i = 0; i < 6; ++i)
	{
		// a, b, c
		Vec3 vPlaneNormal = m_arrFace[i];
		
		// Ax + By + Cz + D > 0
		if (vPlaneNormal.Dot(_WorldPos) + m_arrFace[i].w > 0.f)
			return false;
	}

	return true;
}

bool CFrustum::IsSphereInFrustum(Vec3 _WorldPos, float _Radius)
{
	for (int i = 0; i < 6; ++i)
	{
		// a, b, c
		Vec3 vPlaneNormal = m_arrFace[i];

		// Ax + By + Cz + D > r
		if (vPlaneNormal.Dot(_WorldPos) + m_arrFace[i].w > _Radius)
			return false;
	}

	return true;
}

bool CFrustum::IsAABBInFrustum(Vec3 _AABBMin, Vec3 _AABBMax)
{
	for (int i = 0; i < 6; ++i)
	{
		Vec3 vNormal = m_arrFace[i];
		float fD = m_arrFace[i].w;

		// N-vertex: the AABB corner LEAST aligned with the outward normal
		// (i.e. the corner most INSIDE the frustum for this plane).
		// If even this corner is on the outside, the entire AABB is outside.
		Vec3 vNegative;
		vNegative.x = (vNormal.x >= 0.f) ? _AABBMin.x : _AABBMax.x;
		vNegative.y = (vNormal.y >= 0.f) ? _AABBMin.y : _AABBMax.y;
		vNegative.z = (vNormal.z >= 0.f) ? _AABBMin.z : _AABBMax.z;

		// If the most-inside corner is still outside → entire AABB is outside
		if (vNormal.Dot(vNegative) + fD > 0.f)
			return false;
	}

	return true;
}

void CFrustum::finaltick()
{
	// NOTE: 从投影空间转换到世界空
	const Matrix& matViewInv = m_Owner->GetViewMatInv();
	const Matrix& matProjInv = m_Owner->GetProjMatInv();
	Matrix matInv = matProjInv * matViewInv;

	// 8 frustum corner in world space
	Vec3 arrWorldCorners[8] = {};
	for (int i = 0; i < 8; ++i)
	{
		arrWorldCorners[i] = XMVector3TransformCoord(m_arrNDCCorners[i], matInv);
	}

	// 6 planes
	//   4 --- 5
	//  /|  \  | Far Plane
	// / 7 --/ 6
	// 0 -- 1/ /
	// | \  | / Near Plane
	// 3 -- 2
	m_arrFace[FACE_NEAR] = XMPlaneFromPoints(arrWorldCorners[0], arrWorldCorners[1], arrWorldCorners[2]);
	m_arrFace[FACE_FAR] = XMPlaneFromPoints(arrWorldCorners[5], arrWorldCorners[4], arrWorldCorners[7]);

	m_arrFace[FACE_LEFT] = XMPlaneFromPoints(arrWorldCorners[7], arrWorldCorners[4], arrWorldCorners[0]);
	m_arrFace[FACE_RIGHT] = XMPlaneFromPoints(arrWorldCorners[1], arrWorldCorners[5], arrWorldCorners[6]);

	m_arrFace[FACE_TOP] = XMPlaneFromPoints(arrWorldCorners[0], arrWorldCorners[4], arrWorldCorners[5]);
	m_arrFace[FACE_BOTTOM] = XMPlaneFromPoints(arrWorldCorners[2], arrWorldCorners[6], arrWorldCorners[7]);
}
