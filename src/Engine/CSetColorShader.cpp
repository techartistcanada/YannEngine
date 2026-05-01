#include "pch.h"
#include "CSetColorShader.h"

CSetColorShader::CSetColorShader()
	: CComputeShader(32, 32, 1)
{
}

CSetColorShader::~CSetColorShader()
{
}
int CSetColorShader::Binding()
{
	if (nullptr == m_TargetTexture || !m_TargetTexture->HasUAV())
	{
		return E_FAIL;
	}

	m_TargetTexture->Binding_CS_UAV(0);
	m_MaterialConst.v4Arr[0] = m_ClearColor;
	return S_OK;
}

void CSetColorShader::CalculateNumGroups()
{
	// NOTE: 这里假设纹理的宽度和高度都是线程组大小的整数倍
	m_NumGroupX = (UINT)m_TargetTexture->GetWidth() / m_NumThreadPerGroupX;
	m_NumGroupY = (UINT)m_TargetTexture->GetHeight() / m_NumThreadPerGroupY;
	m_NumGroupZ = 1;
}

void CSetColorShader::Clear()
{
	m_TargetTexture->Clear_CS_UAV(0);
}

