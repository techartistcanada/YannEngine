#pragma once
#include "Singleton.h"

class CGameObject;

class CDbgRenderMgr :
    public CSingleton<CDbgRenderMgr>
{
    SINGLE(CDbgRenderMgr)
private:
    list<tDebugShapeInfo> m_ShapeInfos;

    CGameObject*          m_DebugRenderObj;
public:
    void AddDebugShapeInfo(const tDebugShapeInfo& _info)
    {
        m_ShapeInfos.push_back(_info);
    }

public:
    void render();
};

