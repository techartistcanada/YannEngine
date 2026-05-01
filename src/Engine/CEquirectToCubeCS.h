#pragma once
#include "CComputeShader.h"
#include "CTexture.h"
#include "Ptr.h"

class CEquirectToCubeCS :
    public CComputeShader
{
private:
    Ptr<CTexture> m_SrcEquirectTex;
    Ptr<CTexture> m_DstCubemapTex;
    UINT          m_CubeMapSize;

public:
	void SetEquirectTexture(Ptr<CTexture> _EquirectTex) { m_SrcEquirectTex = _EquirectTex; }
    void SetOutputCubeMapTex(Ptr<CTexture> _CubeMapTex, UINT _Size) { m_DstCubemapTex = _CubeMapTex; m_CubeMapSize = _Size; }

    virtual int Binding() override;
	virtual void CalculateNumGroups() override;
	virtual void Clear() override;
public:
    CEquirectToCubeCS();
    ~CEquirectToCubeCS();
};

