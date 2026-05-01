#include "pch.h"
#include "CLevelMgr.h"
#include "CAssetMgr.h"
#include "CCollisionManager.h"

#include "CLevel.h"
#include "CLayer.h"

CLevelMgr::CLevelMgr()
	: m_CurLevel(nullptr)
{
}

CLevelMgr::~CLevelMgr()
{
	if (nullptr != m_CurLevel)
	{
		delete m_CurLevel;
	}
}

void CLevelMgr::init()
{

}

void CLevelMgr::tick()
{
	if (nullptr != m_CurLevel)
	{
		// TODO: TEMP REMOVE
		// FIXME: LEVEL_STATE::PLAY线路代码落后太久了
		//if (LEVEL_STATE::PLAY == m_CurLevel->GetState())
		//{
			m_CurLevel->tick();
		//}

		// 即使关卡处于暂停状态，也要调用finaltick以保证渲染等功能正常进行, 尤其编辑器模式下
		// 比如CTransform::finaltick中会更新矩阵常量缓冲区
		m_CurLevel->ClearRegisteredObjects();
		m_CurLevel->finaltick();
	}
}

void CLevelMgr::ChangeLevel(CLevel* _NextLevel)
{
	if (nullptr != m_CurLevel)
	{
		delete m_CurLevel;
		m_CurLevel = nullptr;
	}

	m_CurLevel = _NextLevel;
}

