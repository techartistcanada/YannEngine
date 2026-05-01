#include "pch.h"
#include "CBRDFLutCS.h"
#include "CPathMgr.h"

CBRDFLutCS::CBRDFLutCS()
    : CComputeShader(16, 16, 1)
    , m_LutSize(512)
{
}

CBRDFLutCS::~CBRDFLutCS()
{
}

int CBRDFLutCS::Binding()
{
    if (!m_DstLutTex.Get())
        return E_FAIL;

    m_DstLutTex->Binding_CS_UAV(0);    // u0

    m_MaterialConst.iArr[0] = (int)m_LutSize;

    return S_OK;
}

void CBRDFLutCS::CalculateNumGroups()
{
    m_NumGroupX = (m_LutSize + m_NumThreadPerGroupX - 1) / m_NumThreadPerGroupX;
    m_NumGroupY = (m_LutSize + m_NumThreadPerGroupY - 1) / m_NumThreadPerGroupY;
    m_NumGroupZ = 1;
}

void CBRDFLutCS::Clear()
{
    m_DstLutTex->Clear_CS_UAV(0);
}