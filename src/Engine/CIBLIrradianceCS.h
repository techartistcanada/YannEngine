#pragma once
#include "CComputeShader.h"
#include "CTexture.h"
#include "Ptr.h"

class CIBLIrradianceCS :
    public CComputeShader
{
private:
    Ptr<CTexture> m_SrcCubemap;
    Ptr<CTexture> m_DstIrradianceTex;
    UINT          m_IrradianceSize;

public:
    void SetSourceCubemap(Ptr<CTexture> _Cubemap) { m_SrcCubemap = _Cubemap; }
    void SetOutputIrradiance(Ptr<CTexture> _Irradiance, UINT _Size) { m_DstIrradianceTex = _Irradiance; m_IrradianceSize = _Size; }

    virtual int Binding() override;
    virtual void CalculateNumGroups() override;
    virtual void Clear() override;

public:
    CIBLIrradianceCS();
    ~CIBLIrradianceCS();
};