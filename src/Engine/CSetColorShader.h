#pragma once
#include "CComputeShader.h"

#include "CTexture.h"
class CSetColorShader :
    public CComputeShader
{
private:
	Ptr<CTexture> m_TargetTexture;
	Vec4		  m_ClearColor;
public:
	void SetTargetTexture(Ptr<CTexture> _Texture) { m_TargetTexture = _Texture; }
	void SetClearColor(const Vec3 _Color) { m_ClearColor = _Color; m_ClearColor.w = 1.0f; }

public:
	virtual int Binding() override;
	virtual void CalculateNumGroups() override;
	virtual void Clear() override;
public:
	CSetColorShader();
	~CSetColorShader();
};

