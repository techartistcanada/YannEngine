#include "pch.h"
#include "CGenMipsCubemapCS.h"

CGenMipsCubemapCS::CGenMipsCubemapCS()
    : CComputeShader(16, 16, 1)
{
}

int CGenMipsCubemapCS::Binding()
{
    if (!m_pSrcMip || !m_pDstCubemap || !m_pDstCubemap->HasUAV())
        return E_FAIL;

    UINT dstSize = max(1u, (UINT)m_pDstCubemap->GetWidth() >> m_DstMip);
    m_MaterialConst.iArr[0] = (int)dstSize;

    m_pSrcMip->Binding_CS_SRV_Mip(0, m_SrcMip);      // ← read correct mip
    m_pDstCubemap->Binding_CS_UAV_Mip(0, m_DstMip);

    return S_OK;
}

void CGenMipsCubemapCS::CalculateNumGroups()
{
    UINT dstSize = max(1u, (UINT)m_pDstCubemap->GetWidth() >> m_DstMip);
    m_NumGroupX = (dstSize + 15) / 16;
    m_NumGroupY = (dstSize + 15) / 16;
    m_NumGroupZ = 6;
}

void CGenMipsCubemapCS::Clear()
{
    CTexture::Clear_CS_SRV(0);
    CTexture::Clear_CS_UAV(0);
    m_pSrcMip = m_pDstCubemap = nullptr;
    m_DstMip = 0;
}
