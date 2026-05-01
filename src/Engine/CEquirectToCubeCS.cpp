#include "pch.h"
#include "CEquirectToCubeCS.h"
#include "CPathMgr.h"

CEquirectToCubeCS::CEquirectToCubeCS()
    : CComputeShader(16, 16, 1)
    , m_CubeMapSize(1024)
{
}

CEquirectToCubeCS::~CEquirectToCubeCS()
{
}

int CEquirectToCubeCS::Binding()
{
    if (!m_SrcEquirectTex.Get() || !m_DstCubemapTex.Get())
        return E_FAIL;

    m_SrcEquirectTex->Binding_CS_SRV(0);   // t0
    m_DstCubemapTex->Binding_CS_UAV(0);    // u0

    m_MaterialConst.iArr[0] = (int)m_CubeMapSize;

    return S_OK;
}

void CEquirectToCubeCS::CalculateNumGroups()
{
    m_NumGroupX = (m_CubeMapSize + m_NumThreadPerGroupX - 1) / m_NumThreadPerGroupX;
    m_NumGroupY = (m_CubeMapSize + m_NumThreadPerGroupY - 1) / m_NumThreadPerGroupY;
    m_NumGroupZ = 6; // 6 cubemap faces
}

void CEquirectToCubeCS::Clear()
{
    m_SrcEquirectTex->Clear_CS_SRV(0);
    m_DstCubemapTex->Clear_CS_UAV(0);
}