#include "pch.h"
#include "CGenMipsTexture2DCS.h"

CGenMipsTexture2DCS::CGenMipsTexture2DCS()
    : CComputeShader(16, 16, 1)
{
}

int CGenMipsTexture2DCS::Binding()
{
    if (!m_pTexture || !m_pTexture->HasUAV())
        return E_FAIL;

    UINT dstSize = max(1u, (UINT)m_pTexture->GetWidth() >> m_DstMip);
    m_MaterialConst.iArr[0] = (int)dstSize;   // g_int_0 = DstMipSize

    m_pTexture->Binding_CS_SRV_Mip(0, m_DstMip - 1);
    m_pTexture->Binding_CS_UAV_Mip(0, m_DstMip);

    return S_OK;
}

void CGenMipsTexture2DCS::CalculateNumGroups()
{
    UINT dstSize = max(1u, (UINT)m_pTexture->GetWidth() >> m_DstMip);
    m_NumGroupX = (dstSize + 15) / 16;
    m_NumGroupY = (dstSize + 15) / 16;
    m_NumGroupZ = 1;
}

void CGenMipsTexture2DCS::Clear()
{
    CTexture::Clear_CS_SRV(0);
    CTexture::Clear_CS_UAV(0);
    m_pTexture = nullptr;
    m_DstMip   = 0;
}