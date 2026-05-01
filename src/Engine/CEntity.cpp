#include "pch.h"
#include "CEntity.h"

UINT CEntity::g_NextID = 0;

CEntity::CEntity()
	: m_ID(g_NextID++)
	, m_Name(L"None")
{
}

CEntity::CEntity(const CEntity& _Orgin)
	: m_Name(_Orgin.m_Name)
	, m_ID(g_NextID++)
{
}

CEntity::~CEntity()
{
}
