#pragma once
#include "CComputeShader.h"
#include "CTexture.h"
#include "Ptr.h"

class CIBLPrefilterCS :
    public CComputeShader
{
private:
    Ptr<CTexture> m_SrcCubemap;
    Ptr<CTexture> m_DstPrefilter;
    UINT          m_MipSize;
    UINT          m_MipSlice;
    float         m_Roughness;
	UINT          m_EnvMapSize;

public:
    void SetSourceCubemap(Ptr<CTexture> _Cubemap) { m_SrcCubemap = _Cubemap; }
    void SetOutputPrefilter(Ptr<CTexture> _Prefilter) { m_DstPrefilter = _Prefilter; }
    void SetMipLevel(UINT _MipSize, UINT _MipSlice, float _Roughness) { m_MipSize = _MipSize; m_MipSlice = _MipSlice; m_Roughness = _Roughness; }
	void SetEnvMapSize(UINT _EnvMapSize) { m_EnvMapSize = _EnvMapSize; }

    virtual int Binding() override;
    virtual void CalculateNumGroups() override;
    virtual void Clear() override;

public:
    CIBLPrefilterCS();
    ~CIBLPrefilterCS();
};