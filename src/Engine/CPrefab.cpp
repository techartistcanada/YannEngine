#include "pch.h"
#include "CPrefab.h"
#include "CGameObject.h"
#include "CTransform.h"


CPrefab::CPrefab(bool _bEngineAsset)
	: CAsset(ASSET_TYPE::PREFAB, _bEngineAsset)
	, m_ProtoObj(nullptr)
{
}

CPrefab::CPrefab(CGameObject* _ProtoObj)
	: CAsset(ASSET_TYPE::PREFAB, false)
	, m_ProtoObj(_ProtoObj)
{
	m_ProtoObj->Transform()->SetRelativePos(Vec3(0.f, 0.f, 0.f));
}

CPrefab::CPrefab(const CPrefab& _Origin)
	: CAsset(_Origin)
	, m_ProtoObj(nullptr)
{
	m_ProtoObj = _Origin.m_ProtoObj->Clone();
	m_ProtoObj->Transform()->SetRelativePos(Vec3(0.f, 0.f, 0.f));
}

CPrefab::~CPrefab()
{
	if (nullptr != m_ProtoObj)
	{
		delete m_ProtoObj;
	}
}


CGameObject* CPrefab::Instantiate()
{
	return m_ProtoObj->Clone();
}
