#include "pch.h"
#include "CComputeShader.h"
#include "CConstantBuffer.h"

#ifdef USE_DX11
#include "DX11/DX11Device.h"
#else
#include "DX12/DX12Device.h"
#endif

#include "RHI/IRHICommandList.h"


CComputeShader::CComputeShader(UINT _NumThreadPerGroupX, UINT _NumThreadPerGroupY, UINT _NumThreadPerGroupZ)
	: CShader(ASSET_TYPE::COMPUTE_SHADER)
	, m_pRHIShader(g_pRHIDevice->CreateComputeShader())
	, m_NumThreadPerGroupX(_NumThreadPerGroupX)
	, m_NumThreadPerGroupY(_NumThreadPerGroupY)
	, m_NumThreadPerGroupZ(_NumThreadPerGroupZ)
	, m_NumGroupX(1)
	, m_NumGroupY(1)
	, m_NumGroupZ(1)
{
}

CComputeShader::~CComputeShader()
{
	if (m_pRHIShader)
	{
		delete m_pRHIShader;
		m_pRHIShader = nullptr;
	}
}


int CComputeShader::CreateComputeShader(const wstring& _strFilePath, const string& _CSFuncName)
{
	if (FAILED(m_pRHIShader->CreateComputeShader(_strFilePath, _CSFuncName)))
	{
		wstring err = L"Failed to compile compute shader from file: " + _strFilePath;
		MessageBoxW(nullptr, err.c_str(), L"Error", MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	return S_OK;
}

int CComputeShader::Execute()
{
	
	if(FAILED(Binding()))
	{
		return E_FAIL;
	}
	CalculateNumGroups();

	static CConstantBuffer* pCB = RHI_DEVICE->GetConstantBuffer(CB_TYPE::MATERIAL);
	pCB->SetData(&m_MaterialConst);
	pCB->Binding_CS();

	m_pRHIShader->Bind();
	g_pRHIDevice->GetCommandList()->Dispatch(m_NumGroupX, m_NumGroupY, m_NumGroupZ);


	Clear();

	return S_OK;
}
