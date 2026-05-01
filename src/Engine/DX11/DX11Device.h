#pragma once
#include "../define.h"
#include "../CTexture.h"
#include "../RHI/IRHIDevice.h"


class DX11CommandList;
class CConstantBuffer;
class DX11Device
	: public CSingleton<DX11Device>, public IRHIDevice
{
	SINGLE(DX11Device)
private:
	HWND							m_hMainWnd;
	Vec2							m_RenderResolution;

	ComPtr<ID3D11Device>			m_Device;
	ComPtr<ID3D11DeviceContext>		m_Context;

	ComPtr<IDXGISwapChain>			m_SwapChain;
	bool							m_bVSync;


	ComPtr<ID3D11SamplerState>      m_Sampler[3];
	ComPtr<ID3D11RasterizerState>   m_RS[(UINT)RS_TYPE::END];
	ComPtr<ID3D11BlendState>        m_BS[(UINT)BS_TYPE::END];
	ComPtr<ID3D11DepthStencilState>	m_DS[(UINT)DS_TYPE::END];
	CConstantBuffer*			    m_CB[(UINT)CB_TYPE::END];

	bool						    m_bResizing;
	IRHICommandList*				m_pCommandList;

public:
	int init(HWND _hWnd, Vec2 _Resolution);

	void ClearTarget(float(&_ArrColor)[4]);

	void Present(){ m_SwapChain->Present(m_bVSync ? 1 : 0, 0);}
	void ResizeSwapChain(UINT _Width, UINT _Height);
	bool IsResizing() const { return m_bResizing; }
	void SetResizing(bool _Resizing) { m_bResizing = _Resizing; }


	ID3D11Device* GetDevice() { return m_Device.Get(); }
	ID3D11DeviceContext* GetContext() { return m_Context.Get(); }
	IRHICommandList* GetCommandList() override { return m_pCommandList; }

	CConstantBuffer* GetConstantBuffer(CB_TYPE _Type) { return m_CB[(UINT)_Type]; }
	ComPtr<ID3D11RasterizerState> GetRS(RS_TYPE _Type) { return m_RS[(UINT)_Type]; }
	ComPtr<ID3D11DepthStencilState> GetDS(DS_TYPE _Type) { return m_DS[(UINT)_Type]; }
	ComPtr<ID3D11BlendState> GetBS(BS_TYPE _Type) { return m_BS[(UINT)_Type]; }

	Vec2 GetRenderResolution() const { return m_RenderResolution; }

private:
	int CreateSwapChain();
	// dx11渲染管线操作的是资源的View(视图),而不是资源本身，所以需要创建视图
	// view用来资源抽象, 因为一个资源可以有多个不同用途的view
	// 
	int CreateRenderTargetTexAndView();
	int CreateConstantBuffer();
	int CreateSamplerState();
	int CreateRasterizerState();
	int CreateDepthStencilState();
	int CreateBlendState();

public:
	IRHIGraphicsShader* CreateGraphicsShader() override;
	IRHIComputeShader* CreateComputeShader() override;
	IRHIPipelineState* CreatePipelineState() override;
	IRHIBuffer* CreateBuffer() override;
	IRHITexture* CreateTexture() override;
};

