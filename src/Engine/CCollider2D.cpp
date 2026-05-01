#include "pch.h"
#include "CCollider2D.h"


#include "CTransform.h"
#include "CScript.h"
#include "CTaskMgr.h"

CCollider2D::CCollider2D()
	: CComponent(COMPONENT_TYPE::COLLIDER2D)
	, m_Scale(Vec3(1.f, 1.f, 1.f))
	, m_Absolute(false)
	, m_Active(true)
	, m_SemiInactive(false)
{
}

CCollider2D::CCollider2D(const CCollider2D& _Other)
	: CComponent(_Other)
	, m_Offset(_Other.m_Offset)
	, m_FinalPos(_Other.m_FinalPos)
	, m_Scale(_Other.m_Scale)
	, m_Rotation(_Other.m_Rotation)
	, m_Absolute(_Other.m_Absolute)
	, m_OverlapCount(0)
	, m_Active(_Other.m_Active)
	, m_SemiInactive(false)
{
}

CCollider2D::~CCollider2D()
{
}

void CCollider2D::Activate()
{
	m_Active = true;
	m_SemiInactive = false;
}

void CCollider2D::Deactivate()
{
	tTask task = {};
	task.Type = TASK_TYPE::COLLIDER_DEACTIVATE;
	task.dwParam_0 = (DWORD_PTR)this;
	CTaskMgr::GetInst()->AddTask(task);
}

void CCollider2D::finaltick()
{
	if (m_SemiInactive)
	{
		CTaskMgr::GetInst()->AddTask(tTask{ TASK_TYPE::COLLIDER_DEACTIVATE, (DWORD_PTR)this });
	}
	else if (!m_Active)
		return;



	Vec3 vObjPos = Transform()->GetRelativePos();

	m_FinalPos = vObjPos + m_Offset;

	if (false == m_Absolute)
	{
		Matrix matScale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
		Matrix matRot = XMMatrixRotationZ(m_Rotation.z);
		Matrix matTrans = XMMatrixTranslation(m_Offset.x, m_Offset.y, m_Offset.z);

		// ==========
		// Final World Matrix
		// 最终的世界矩阵
		// ==========
		m_matColWorld = matScale * matRot * matTrans;
		m_matColWorld *= Transform()->GetWorldMat();

	}
	else
	{
		// NOTE: 计算绝对坐标的碰撞体矩阵时，需要将物体自身的缩放反向应用到碰撞体上,
		// 因为物体的缩放会影响到碰撞体
		Matrix matScale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
		Matrix matRot = XMMatrixRotationZ(m_Rotation.z);
		Matrix matTrans = XMMatrixTranslation(m_Offset.x, m_Offset.y, m_Offset.z);

		m_matColWorld = matScale * matRot * matTrans;
		Matrix matObjScaleInv = XMMatrixIdentity();
		Vec3 vObjScale = Transform()->GetRelativeScale();
		matObjScaleInv = XMMatrixScaling(vObjScale.x, vObjScale.y, vObjScale.z);
		matObjScaleInv = XMMatrixInverse(nullptr, matObjScaleInv);

		m_matColWorld = m_matColWorld * matObjScaleInv * Transform()->GetWorldMat();
	}

	if (m_OverlapCount == 0)
	{
		DrawDebugRect(m_matColWorld, Vec4(0.f, 1.f, 0.f, 1.f), false, 0.f);
	}
	else if (1 <= m_OverlapCount)
	{
		DrawDebugRect(m_matColWorld, Vec4(1.f, 0.f, 0.f, 1.f), false, 0.f);
	}
	else
		assert(nullptr);

}

void CCollider2D::BeginOverlap(CCollider2D* _OtherCollider)
{
	++m_OverlapCount;
	const vector<CScript*>& vecScripts = GetOwner()->GetScripts();
	for (size_t i = 0; i < vecScripts.size(); ++i)
	{
		vecScripts[i]->BeginOverlap(this, _OtherCollider->GetOwner(), _OtherCollider);
	}

}

void CCollider2D::Overlap(CCollider2D* _OtherCollider)
{
	const vector<CScript*>& vecScripts = GetOwner()->GetScripts();
	for (size_t i = 0; i < vecScripts.size(); ++i)
	{
		vecScripts[i]->Overlap(this, _OtherCollider->GetOwner(), _OtherCollider);
	}
}

void CCollider2D::EndOverlap(CCollider2D* _OtherCollider)
{
	--m_OverlapCount;

	const vector<CScript*>& vecScripts = GetOwner()->GetScripts();

	for (size_t i = 0; i < vecScripts.size(); ++i)
	{
		vecScripts[i]->EndOverlap(this, _OtherCollider->GetOwner(), _OtherCollider);
	}
}

void CCollider2D::SaveToLevelFile(FILE* _File)
{
	fwrite(&m_Offset, sizeof(Vec3), 1, _File);
	fwrite(&m_Scale, sizeof(Vec3), 1, _File);
	fwrite(&m_Rotation, sizeof(Vec3), 1, _File);
	fwrite(&m_Absolute, sizeof(bool), 1, _File);
	fwrite(&m_Active, sizeof(bool), 1, _File);
	fwrite(&m_SemiInactive, sizeof(bool), 1, _File);
}

void CCollider2D::LoadFromLevelFile(FILE* _File)
{
	fread(&m_Offset, sizeof(Vec3), 1, _File);
	fread(&m_Scale, sizeof(Vec3), 1, _File);
	fread(&m_Rotation, sizeof(Vec3), 1, _File);
	fread(&m_Absolute, sizeof(bool), 1, _File);
	fread(&m_Active, sizeof(bool), 1, _File);
	fread(&m_SemiInactive, sizeof(bool), 1, _File);
}
