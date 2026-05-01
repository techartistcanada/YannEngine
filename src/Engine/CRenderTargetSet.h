#pragma once
#include "CEntity.h"

class CRenderTargetSet :
    public CEntity
{
private:
	Ptr<CTexture> m_RTTextures[8];
	Vec4          m_ClearColors[8];
	UINT          m_RTCount;

	Ptr<CTexture> m_DSTexture;

	// Viewport
	float m_ViewportWidth;
	float m_ViewportHeight;

public:
	void OMSet();
	void ClearTargets();
	void ClearDepthStencil();
	void SetClearColor(UINT _Index, Vec4 _Color) { if (_Index < m_RTCount) m_ClearColors[_Index] = _Color; }
	void Init(Ptr<CTexture>* _RTTextures, UINT _RTCount, Ptr<CTexture> _DSTexture);

	Ptr<CTexture> GetRTTexture(UINT _Index) const { if (_Index < m_RTCount) return m_RTTextures[_Index]; return nullptr; }

	CLONE_DISABLED(CRenderTargetSet);
public:
	CRenderTargetSet();
	~CRenderTargetSet();

};

