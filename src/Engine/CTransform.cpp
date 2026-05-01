#include "pch.h"
#include "CTransform.h"

#ifdef USE_DX11
#include "DX11/DX11Device.h"
#else
#include "DX12/DX12Device.h"
#endif

#include "CConstantBuffer.h"


CTransform::CTransform()
	: CComponent(COMPONENT_TYPE::TRANSFORM)
	, m_RelativeScale{1.0f, 1.0f, 1.0f}
	, m_Absolute(false)
{
}

CTransform::~CTransform()
{
}

void CTransform::finaltick()
{
	// ================================================
	// Local 
	// ================================================
	m_matWorld = XMMatrixIdentity();

	// Scale 缩放
	//| sx  0   0   0 |
	//| 0   sy  0   0 |
	//| 0   0   sz  0 |
	//| 0   0   0   1 |
	//Matrix matScale = XMMatrixIdentity();
	//matScale._11 = m_RelativeScale.x;
	//matScale._22 = m_RelativeScale.y;
	//matScale._33 = m_RelativeScale.z;
	Matrix matScale = XMMatrixScaling(m_RelativeScale.x, m_RelativeScale.y, m_RelativeScale.z);



	// Rotation around Z Z旋转
	//|  cosθ   sinθ   0   0 |
	//| -sinθ   cosθ   0   0 |
	//|   0       0    1   0 |
	//|   0       0    0   1 |
	//Matrix matZRot = XMMatrixIdentity();
	//matZRot._11 = cosf(m_RelativeRotation.z);	matZRot._12 = sinf(m_RelativeRotation.z);
	//matZRot._21 = -sinf(m_RelativeRotation.z);  matZRot._22 = cosf(m_RelativeRotation.z);
	Matrix matRot = XMMatrixRotationX(m_RelativeRotation.x);
	matRot *= XMMatrixRotationY(m_RelativeRotation.y);
	matRot *= XMMatrixRotationZ(m_RelativeRotation.z);

	// Translation 平移
	//| 1  0  0  0 |
	//| 0  1  0  0 |
	//| 0  0  1  0 |
	//| tx ty tz 1 |
	//Matrix matTranslation = XMMatrixIdentity();
	//matTranslation._41 = m_RelativePos.x;
	//matTranslation._42 = m_RelativePos.y;
	//matTranslation._43 = m_RelativePos.z;
	Matrix matTranslation = XMMatrixTranslation(m_RelativePos.x, m_RelativePos.y, m_RelativePos.z);

	m_matWorld = matScale * matRot * matTranslation;

	m_WorldDir[(UINT)DIR_TYPE::RIGHT] = m_RelativeDir[(UINT)DIR_TYPE::RIGHT] = XAxis;
	m_WorldDir[(UINT)DIR_TYPE::UP] = m_RelativeDir[(UINT)DIR_TYPE::UP] = YAxis;
	m_WorldDir[(UINT)DIR_TYPE::FRONT] = m_RelativeDir[(UINT)DIR_TYPE::FRONT] = ZAxis;
	
	// calculate relative direction
	for (int i = 0; i < 3; ++i)
	{
		//XMVector3TransformCoord(); // NOTE: (x, y, z, 1) × Matrix
		m_RelativeDir[i] = XMVector3TransformNormal(m_RelativeDir[i], matRot); // (x, y, z, 0) × Matrix
		m_RelativeDir[i].Normalize();
	}
	// ================================================
	// Parent
	// ================================================
	if(GetOwner()->GetParent())
	{
		const Matrix& matParentWorldMat = GetOwner()->GetParent()->Transform()->GetWorldMat();

		if(m_Absolute)
		{
			// 抵消父缩放, 因为父缩放会影响子物体的缩放
			Vec3 vParentScale = GetOwner()->GetParent()->Transform()->GetWorldScale();
			Matrix matScaleInv = XMMatrixInverse(nullptr, XMMatrixScaling(vParentScale.x, vParentScale.y, vParentScale.z));
			// 继承父的旋转和平移，但不继承父缩放
			m_matWorld = m_matWorld * matScaleInv * matParentWorldMat;
		}
		else
		{
			m_matWorld *= matParentWorldMat;
		}

		for (int i = 0; i < 3; ++i)
		{
			// XMVector3TransformNormal ignores translation
			// XMVector3TransformNormal 忽略位移
			m_WorldDir[i] = XMVector3TransformNormal(m_WorldDir[i], m_matWorld);
			m_WorldDir[i].Normalize();
		}
	}
	else
	{
		for (int i = 0; i < 3; ++i)
		{
			m_WorldDir[i] = m_RelativeDir[i];
		}
	}

	// calc world inverse matrix
	m_matWorldInv = XMMatrixInverse(nullptr, m_matWorld);

	  
}

void CTransform::Binding()
{
	// NOTE: 从RAM传送到显存
	CConstantBuffer* pCB = RHI_DEVICE->GetConstantBuffer(CB_TYPE::TRANSFORM);

	g_Trans.matWorld = m_matWorld;
	g_Trans.matWorldInv = m_matWorldInv;
	g_Trans.matWV = g_Trans.matWorld * g_Trans.matView;
	g_Trans.matWVP = g_Trans.matWV * g_Trans.matProj;

	pCB->SetData(&g_Trans);
	pCB->Binding();
}

Vec3 CTransform::GetWorldPos()
{
	// NOTE: matWorld.41, matWorld.4.2, matWorld.4.3
	return m_matWorld.Translation();
}

Vec3 CTransform::GetWorldScale()
{
	CGameObject* pObject = GetOwner();
	Vec3 vWorldScale = Vec3(1.f, 1.f, 1.f);

	while (pObject)
	{
		vWorldScale *= pObject->Transform()->GetRelativeScale();
		if (pObject->Transform()->IsAbsolute())
			break; // 如果是绝对变换，则不再继续向上累积父物体的缩放

		pObject = pObject->GetParent();
	}
	return vWorldScale;
}

Vec3 CTransform::GetWorldRotation()
{
	return Vec3();
}

void CTransform::SaveToLevelFile(FILE* _File)
{
	fwrite(&m_RelativePos, sizeof(Vec3), 1, _File);
	fwrite(&m_RelativeScale, sizeof(Vec3), 1, _File);
	fwrite(&m_RelativeRotation, sizeof(Vec3), 1, _File);
	fwrite(&m_Absolute, sizeof(bool), 1, _File);
}

void CTransform::LoadFromLevelFile(FILE* _File)
{
	fread(&m_RelativePos, sizeof(Vec3), 1, _File);
	fread(&m_RelativeScale, sizeof(Vec3), 1, _File);
	fread(&m_RelativeRotation, sizeof(Vec3), 1, _File);
	fread(&m_Absolute, sizeof(bool), 1, _File);
}

CTransform& CTransform::operator=(const CTransform& _Other)
{
    if (this == &_Other)
        return *this;

    m_RelativePos = _Other.m_RelativePos;
    m_RelativeScale = _Other.m_RelativeScale;
    m_RelativeRotation = _Other.m_RelativeRotation;
    m_RelativeDir[0] = _Other.m_RelativeDir[0];
    m_RelativeDir[1] = _Other.m_RelativeDir[1];
    m_RelativeDir[2] = _Other.m_RelativeDir[2];
    m_WorldDir[0] = _Other.m_WorldDir[0];
    m_WorldDir[1] = _Other.m_WorldDir[1];
    m_WorldDir[2] = _Other.m_WorldDir[2];
    m_matWorld = _Other.m_matWorld;
    m_matWorldInv = _Other.m_matWorldInv;
    m_Absolute = _Other.m_Absolute;

    return *this;
}
