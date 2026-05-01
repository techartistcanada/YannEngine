#include "pch.h"
#include "CCollisionManager.h"

#include "CLevelMgr.h"
#include "CLevel.h"
#include "CLayer.h"
#include "CGameObject.h"
#include "CCollider2D.h"

CCollisionManager::CCollisionManager()
	: m_Matrix{}
{
}

CCollisionManager::~CCollisionManager()
{
}

void CCollisionManager::tick()
{
	if (!CLevelMgr::GetInst()->GetCurrentLevel())
		return;

	// TODO: shouldn't do this while level state is .....
	// NOTE:
	for (UINT Row = 0; Row < MAX_LAYER; ++Row)
	{
		for (UINT Col = Row; Col < MAX_LAYER; ++Col)
		{
			if (m_Matrix[Row] & (1 << Col))
			{
				CollisionBtwLayers(Row, Col);
			}
		}
	}
}

void CCollisionManager::LayerCheck(UINT _LayerIdx, UINT _LayerRightIdx)
{
	// NOTE:
	UINT Row = _LayerIdx;
	UINT Col = _LayerRightIdx;

	if (Col < Row)
	{
		Row = _LayerRightIdx;
		Col = _LayerIdx;
	}
	UINT CheckBit = (1 << Col);
	if (m_Matrix[Row] & CheckBit)
	{
		// 可以检测碰撞
		m_Matrix[Row] &= ~CheckBit;
	}
	else
	{
		// 不检测碰撞
		m_Matrix[Row] |= CheckBit;
	}
}

void CCollisionManager::SaveCollisionInfosToLevelFile(FILE* _File)
{
	fwrite(&m_Matrix, sizeof(UINT), MAX_LAYER, _File);
}

void CCollisionManager::LoadCollisionInfosFromLevelFile(FILE* _File)
{
	fread(&m_Matrix, sizeof(UINT), MAX_LAYER, _File);
}

void CCollisionManager::CollisionBtwLayers(UINT _Left, UINT _Right)
{
	CLevel* pCurLevel = CLevelMgr::GetInst()->GetCurrentLevel();

	CLayer* pLeftLayer = pCurLevel->GetLayer(_Left);
	CLayer* pRightLayer = pCurLevel->GetLayer(_Right);

	const vector<CGameObject*>& vecLeftObjs = pLeftLayer->GetObjects();
	const vector<CGameObject*>& vecRightObjs = pRightLayer->GetObjects();
	if (_Left != _Right)
	{
		// 两个不同图层
		for (size_t i = 0; i < vecLeftObjs.size(); ++i)
		{
			if (nullptr == vecLeftObjs[i]->Collider2D())
				continue;

			for (size_t j = 0; j < vecRightObjs.size(); ++j)
			{
				if (nullptr == vecRightObjs[j]->Collider2D())
					continue;

				CollisionBtwCollider2D(vecLeftObjs[i]->Collider2D(), vecRightObjs[j]->Collider2D());
			}
		}
	}
	else
	{
		// 同一图层
		for (size_t i = 0; i < vecLeftObjs.size(); ++i)
		{
			if (nullptr == vecLeftObjs[i]->Collider2D())
				continue;

			for (size_t j = i+1; j < vecRightObjs.size(); ++j)
			{
				if (nullptr == vecRightObjs[j]->Collider2D())
					continue;

				CollisionBtwCollider2D(vecLeftObjs[i]->Collider2D(), vecRightObjs[j]->Collider2D());
			}
		}
	}

}

void CCollisionManager::CollisionBtwCollider2D(CCollider2D* _LeftCol, CCollider2D* _RightCol)
{
	COL_ID id;
	id.LeftID = _LeftCol->GetID();
	id.RightID = _RightCol->GetID();

	map<LONGLONG, bool>::iterator iter = m_ColInfo.find(id.ID);
	if (iter == m_ColInfo.end())
	{
		m_ColInfo.insert(make_pair(id.ID, false));
		iter = m_ColInfo.find(id.ID);
	}


	bool IsDead = _LeftCol->GetOwner()->IsDead() || _RightCol->GetOwner()->IsDead();
	bool IsActive = _LeftCol->IsActive() && _RightCol->IsActive();
	bool IsSemiInactive = _LeftCol->IsSemiInactive() || _RightCol->IsSemiInactive();

	if(!IsActive)
		return;

	if (IsCollision(_LeftCol, _RightCol))
	{
		if (iter->second)
		{
			if (IsDead || IsSemiInactive)
			{
				// NOTE: 如果有对象下一帧将被删除，那么结束碰撞
				_LeftCol->EndOverlap(_RightCol);
				_RightCol->EndOverlap(_LeftCol);
				iter->second = false;
			}
			else
			{
				// overlap
				// 上一帧有碰撞，这一帧也有
				_LeftCol->Overlap(_RightCol);
				_RightCol->Overlap(_LeftCol);
			}
		}
		else
		{
			if(!IsDead && !IsSemiInactive)
			{
				// begin overlap
				// 上一帧没有碰撞，这一帧有
				_LeftCol->BeginOverlap(_RightCol);
				_RightCol->BeginOverlap(_LeftCol);
				iter->second = true;
			}
			// NOTE: 如果有对象下一帧将被删除，那么无视BeginOverlap
		}
	}
	else
	{
		if (iter->second)
		{
			// 上一帧有碰撞，这一帧没有
			_LeftCol->EndOverlap(_RightCol);
			_RightCol->EndOverlap(_LeftCol);
		}

		iter->second = false;
	}
}

bool CCollisionManager::IsCollision(CCollider2D* _LeftCol, CCollider2D* _RightCol)
{
	// OBB
	// 0 -- 1
	// |    |
	// 3 -- 2
	Vec3 vLocal[4] =
	{
		Vec3(-0.5f, 0.5f, 0.f),
		Vec3(0.5f, 0.5f, 0.f),
		Vec3(0.5f, -0.5f, 0.f),
		Vec3(-0.5f, -0.5f, 0.f),
	};

	Vec3 vLeftCol[3] = {};
	Vec3 vRightCol[3] = {};
	for (int i = 0; i < 3; ++i)
	{
		// XMVector3TransformCoord -> (x, y, z, 1)
		vLeftCol[i] = XMVector3TransformCoord(vLocal[i], _LeftCol->GetWorldMatrix());
		vRightCol[i] = XMVector3TransformCoord(vLocal[i], _RightCol->GetWorldMatrix());
	}
	Vec3 vLeftCenter = XMVector3TransformCoord(Vec3(0.f, 0.f, 0.f), _LeftCol->GetWorldMatrix());
	Vec3 vRightCenter = XMVector3TransformCoord(Vec3(0.f, 0.f, 0.f), _RightCol->GetWorldMatrix());
	// 从左中心指向右中心的向量
	Vec3 vCenter = vRightCenter - vLeftCenter;


	Vec3 arrProj[4] =
	{
		vLeftCol[1] - vLeftCol[0],
		vLeftCol[2] - vLeftCol[1],
		vLeftCol[1] - vLeftCol[0],
		vLeftCol[2] - vLeftCol[1],
	};

	for (int i = 0; i < 4; ++i)
	{
		Vec3 vProj = arrProj[i];
		vProj.Normalize();

		float fProjLen = 0.f;
		for (int j = 0; j < 4; ++j)
		{
			fProjLen += fabs(arrProj[i].Dot(vProj));
		}
		fProjLen *= 0.5f;
		float fCenter = fabs(vCenter.Dot(vProj));

		if (fProjLen < fCenter)
			return false;
	}

	return true;
}
