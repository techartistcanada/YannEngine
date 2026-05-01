#include "pch.h"
#include "CGraphicsShader.h"

#ifdef USE_DX11
#include "DX11/DX11Device.h"
#else
#include "DX12/DX12Device.h"
#endif



CGraphicsShader::CGraphicsShader()
	: CShader(ASSET_TYPE::GRAPHICS_SHADER)
	, m_pRHIShader(g_pRHIDevice->CreateGraphicsShader())
	, m_pPipelineState(g_pRHIDevice->CreatePipelineState())
	, m_PipelineDesc{}
{
	m_PipelineDesc.pShader = m_pRHIShader;
	m_PipelineDesc.PrimitiveTopology = RHI_PRIMITIVE_TOPOLOGY::TRIANGLE_LIST;
	m_PipelineDesc.RasterizerState = RS_TYPE::CULL_BACK;
	m_PipelineDesc.DepthStencilState = DS_TYPE::LESS;
	m_PipelineDesc.BlendState = BS_TYPE::DEFAULT;
}

CGraphicsShader::~CGraphicsShader()
{
	delete m_pRHIShader;
	delete m_pPipelineState;

}

int CGraphicsShader::CreateVertexShader(const wstring& _strFilePath, const string& _VSFuncName)
{
	return m_pRHIShader->CreateVertexShader(_strFilePath, _VSFuncName);
}

int CGraphicsShader::CreateHullShader(const wstring& _strFilePath, const string& _HSFuncName)
{
	return m_pRHIShader->CreateHullShader(_strFilePath, _HSFuncName);
}

int CGraphicsShader::CreatePixelShader(const wstring& _strFilePath, const string& _PSFuncName)
{
	return m_pRHIShader->CreatePixelShader(_strFilePath, _PSFuncName);
}

int CGraphicsShader::CreateDomainShader(const wstring& _strFilePath, const string& _DSFuncName)
{
	return m_pRHIShader->CreateDomainShader(_strFilePath, _DSFuncName);
}

int CGraphicsShader::CreateGeometryShader(const wstring& _strFilePath, const string& _GSFuncName)
{
	return m_pRHIShader->CreateGeometryShader(_strFilePath, _GSFuncName);
}

int CGraphicsShader::Binding()
{
	m_PipelineDesc.pShader = m_pRHIShader;
	m_pPipelineState->Create(m_PipelineDesc);
	m_pPipelineState->Bind();
	return S_OK;
}
