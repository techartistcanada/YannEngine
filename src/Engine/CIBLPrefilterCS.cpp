#include "pch.h"
#include "CIBLPrefilterCS.h"
#include "CPathMgr.h"

CIBLPrefilterCS::CIBLPrefilterCS()
    : CComputeShader(16, 16, 1)
    , m_MipSize(256)
    , m_MipSlice(0)
    , m_Roughness(0.f)
    , m_EnvMapSize(1024)
{
}

CIBLPrefilterCS::~CIBLPrefilterCS()
{
}

int CIBLPrefilterCS::Binding()
{
    if (!m_SrcCubemap.Get() || !m_DstPrefilter.Get())
        return E_FAIL;

    m_SrcCubemap->Binding_CS_SRV(0);                       // t0
    m_DstPrefilter->Binding_CS_UAV_Mip(0, m_MipSlice);    // u0 at specific mip

    m_MaterialConst.iArr[0] = (int)m_MipSize;
    m_MaterialConst.fArr[0] = m_Roughness;
    m_MaterialConst.iArr[1] = (int)m_EnvMapSize;  // source cubemap resolution

    return S_OK;
}

void CIBLPrefilterCS::CalculateNumGroups()
{
    m_NumGroupX = (m_MipSize + m_NumThreadPerGroupX - 1) / m_NumThreadPerGroupX;
    m_NumGroupY = (m_MipSize + m_NumThreadPerGroupY - 1) / m_NumThreadPerGroupY;
    m_NumGroupZ = 6;
}

void CIBLPrefilterCS::Clear()
{
    m_SrcCubemap->Clear_CS_SRV(0);
    CTexture::Clear_CS_UAV(0);
}