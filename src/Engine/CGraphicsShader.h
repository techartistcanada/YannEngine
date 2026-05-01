#pragma once
#include "CShader.h"

#include "RHI/IRHIGraphicsShader.h"
#include "RHI/IRHIPipelineState.h"

class CGraphicsShader :
    public CShader
{
private:
	IRHIGraphicsShader*		m_pRHIShader;
	IRHIPipelineState*		m_pPipelineState;
	RHIGraphicsPipelineDesc m_PipelineDesc;


public:
    int CreateVertexShader(const wstring& _strFilePath, const string& _VSFuncName);
	int CreateHullShader(const wstring& _strFilePath, const string& _HSFuncName);
	int CreateDomainShader(const wstring& _strFilePath, const string& _DSFuncName);
    int CreateGeometryShader(const wstring& _strFilePath, const string& _GSFuncName);
    int CreatePixelShader(const wstring& _strFilePath, const string& _PSFuncName);
    void SetTopology(D3D11_PRIMITIVE_TOPOLOGY _Topology)
    {
		m_PipelineDesc.PrimitiveTopology = static_cast<RHI_PRIMITIVE_TOPOLOGY>(_Topology);
    }

    // FIXME: DX12, not a good place
    void SetNumRenderTargets(UINT _Count)              { m_PipelineDesc.NumRenderTargets = _Count; }
    void SetRTVFormat(UINT _Slot, DXGI_FORMAT _Format) { m_PipelineDesc.RTVFormats[_Slot] = _Format; }
    void SetDSVFormat(DXGI_FORMAT _Format)             { m_PipelineDesc.DSVFormat = _Format; }

    void SetRSType(RS_TYPE _Type) { m_PipelineDesc.RasterizerState   = _Type; }
    void SetDSType(DS_TYPE _Type) { m_PipelineDesc.DepthStencilState = _Type; }
    void SetBSType(BS_TYPE _Type) { m_PipelineDesc.BlendState        = _Type; }

    void          SetDomain(SHADER_DOMAIN _Domain) { m_pRHIShader->SetDomain(_Domain); }
    SHADER_DOMAIN GetDomain()                const  { return m_pRHIShader->GetDomain(); }

    virtual int Binding() override;

public:
    CGraphicsShader();
    ~CGraphicsShader();
};


