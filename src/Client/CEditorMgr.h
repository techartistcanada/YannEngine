#pragma once
#include <Singleton.h>

class CEditorMgr :
	public CSingleton<CEditorMgr>
{
	SINGLE(CEditorMgr)
private:
	vector<CGameObject*> m_vecEditorObjects;
public:
	void init();
	void tick();

};

