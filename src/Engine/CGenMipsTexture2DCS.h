#pragma once
#include "CComputeShader.h"
#include "CTexture.h"

// Generates all mip levels for a flat Texture2D using Karis-weighted downsampling.
class CGenMipsTexture2DCS : public CComputeShader
{
private:
    CTexture* m_pTexture = nullptr;
    UINT      m_DstMip   = 0;

public:
    void SetTexture(CTexture* _pTex) { m_pTexture = _pTex; }
    void SetDstMip(UINT _Mip)        { m_DstMip = _Mip;    }

public:
    virtual int  Binding()            override;
    virtual void CalculateNumGroups() override;
    virtual void Clear()              override;

public:
    CGenMipsTexture2DCS();
    ~CGenMipsTexture2DCS() = default;
};