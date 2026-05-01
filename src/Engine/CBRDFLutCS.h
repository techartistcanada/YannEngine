#pragma once
#include "CComputeShader.h"
#include "CTexture.h"
#include "Ptr.h"

class CBRDFLutCS :
    public CComputeShader
{
private:
    Ptr<CTexture> m_DstLutTex;
    UINT          m_LutSize;

public:
    void SetOutputTex(Ptr<CTexture> _Lut, UINT _Size) { m_DstLutTex = _Lut; m_LutSize = _Size; }
    virtual int Binding() override;
    virtual void CalculateNumGroups() override;
    virtual void Clear() override;

public:
    CBRDFLutCS();
    ~CBRDFLutCS();
};
