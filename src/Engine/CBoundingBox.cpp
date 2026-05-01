#include "pch.h"
#include "CBoundingBox.h"

#include "CTransform.h"
#include "CRenderMgr.h"



CBoundingBox::CBoundingBox()
	: CComponent(COMPONENT_TYPE::BOUNDINGBOX)
	, m_Absolute(false)
	, m_Radius(0.f)
	// TODO: temporary
	, m_bShowDebug((rand() % 100) < 10) // Randomly show debug for ~10% of bounding boxes
{
}

CBoundingBox::~CBoundingBox()
{
}

void CBoundingBox::finaltick()
{
		const Matrix& matWorld = Transform()->GetWorldMat();

	// =========================================
	// 1. Transform local bounding center to world space
	// =========================================
	Vec3 worldCenter = XMVector3TransformCoord(m_OffsetPos, matWorld);

	// =========================================
	// 2. Compute world AABB from rotated/scaled local extents
	// =========================================
	Vec3 halfExt = m_LocalHalfExtents;
	if (halfExt.x <= 0.f && halfExt.y <= 0.f && halfExt.z <= 0.f)
	{
		halfExt = Vec3(m_Radius, m_Radius, m_Radius);
	}

	Vec3 worldHalfExt;
	if (m_Absolute)
	{
		worldHalfExt = halfExt;
	}
	else
	{
		// Standard OBB → AABB: project each local axis onto world axes
		// using the upper-3x3 of the world matrix (rotation + scale)
		worldHalfExt.x = fabsf(matWorld._11) * halfExt.x
		               + fabsf(matWorld._21) * halfExt.y
		               + fabsf(matWorld._31) * halfExt.z;
		worldHalfExt.y = fabsf(matWorld._12) * halfExt.x
		               + fabsf(matWorld._22) * halfExt.y
		               + fabsf(matWorld._32) * halfExt.z;
		worldHalfExt.z = fabsf(matWorld._13) * halfExt.x
		               + fabsf(matWorld._23) * halfExt.y
		               + fabsf(matWorld._33) * halfExt.z;
	}

	m_WorldAABBMin = worldCenter - worldHalfExt;
	m_WorldAABBMax = worldCenter + worldHalfExt;

	// =========================================
	// 3. Rebuild sphere world matrix (for debug draw & GetWorldRadius/GetWorldPos)
	// =========================================
	Vec3 ObjectScale = Transform()->GetWorldScale();
	float MaxScale = ObjectScale.x;
	if (MaxScale < ObjectScale.y)
		MaxScale = ObjectScale.y;
	if (MaxScale < ObjectScale.z)
		MaxScale = ObjectScale.z;

	float worldRadius = m_Radius * MaxScale;
	m_WorldMat = XMMatrixScaling(worldRadius, worldRadius, worldRadius)
	           * XMMatrixTranslation(worldCenter.x, worldCenter.y, worldCenter.z);

	if (CRenderMgr::GetInst()->IsShowBoundingBox() && m_bShowDebug)
	{
		// Draw green wireframe AABB — this is the EXACT box used for frustum culling
		// Only render ~10% of bounding boxes to reduce draw calls and visual clutter
		// 只渲染约10%的包围盒，减少draw call和视觉干扰
		Vec3 aabbCenter = (m_WorldAABBMin + m_WorldAABBMax) * 0.5f;
		Vec3 aabbSize   = m_WorldAABBMax - m_WorldAABBMin;
		DrawDebugCube(aabbCenter, aabbSize, Vec3(0.f, 0.f, 0.f), Vec4(0.f, 1.f, 0.f, 1.f), false, 0.f);
	}
}

void CBoundingBox::SaveToLevelFile(FILE* _File)
{
}

void CBoundingBox::LoadFromLevelFile(FILE* _File)
{
}
