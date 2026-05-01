#include "pch.h"
#include "CIBLIrradianceCS.h"

CIBLIrradianceCS::CIBLIrradianceCS()
	: CComputeShader(16, 16, 1)
	, m_IrradianceSize(32)
{
}

CIBLIrradianceCS::~CIBLIrradianceCS()
{}

int CIBLIrradianceCS::Binding()
{
	if(!m_SrcCubemap.Get() || !m_DstIrradianceTex.Get())
	{
		return E_FAIL;
	}

	m_SrcCubemap->Binding_CS_SRV(0);
	m_DstIrradianceTex->Binding_CS_UAV(0);

	m_MaterialConst.iArr[0] = m_IrradianceSize;

	return S_OK;
}

void CIBLIrradianceCS::CalculateNumGroups()
{
	m_NumGroupX = (m_IrradianceSize + m_NumThreadPerGroupX - 1) / m_NumThreadPerGroupX;
    m_NumGroupY = (m_IrradianceSize + m_NumThreadPerGroupY - 1) / m_NumThreadPerGroupY;
    m_NumGroupZ = 6;
}

void CIBLIrradianceCS::Clear()
{
	m_SrcCubemap->Clear_CS_SRV(0);
    m_DstIrradianceTex->Clear_CS_UAV(0);
}

