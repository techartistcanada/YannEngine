#include "../pch.h"
#ifdef USE_DX11

#include "DX11Device.h"

#include "../CConstantBuffer.h"
#include "../CAssetMgr.h"

#include "DX11CommandList.h"
#include "DX11GraphicsShader.h" 
#include "DX11ComputeShader.h"
#include "DX11PipelineState.h"
#include "DX11Buffer.h"
#include "DX11Texture.h"


DX11Device::DX11Device()
	: m_hMainWnd(nullptr)
	, m_Sampler{}
	, m_CB{nullptr}
	, m_bVSync(false)
	, m_bResizing(false)
	, m_pCommandList(nullptr)
{
}

DX11Device::~DX11Device()
{
	Safe_Del_Array<CConstantBuffer, 4>(m_CB);
	delete m_pCommandList;
}

int DX11Device::CreateSwapChain()
{
	DXGI_SWAP_CHAIN_DESC Desc = {};

	Desc.OutputWindow = m_hMainWnd;
	Desc.Windowed = true;

	Desc.BufferCount = 1;
	Desc.BufferDesc.Width = (UINT)m_RenderResolution.x;
	Desc.BufferDesc.Height = (UINT)m_RenderResolution.y;
	Desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	Desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	Desc.BufferDesc.RefreshRate.Denominator = 1;
	Desc.BufferDesc.RefreshRate.Numerator = 60;
	Desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	Desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	Desc.SampleDesc.Count = 1;
	Desc.SampleDesc.Quality = 0;

	// TODO: DXGI_SWAP_EFFECT_FLIP_DISCARD
	Desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	Desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ComPtr<IDXGIDevice> pDXGIDevice = nullptr;
	ComPtr<IDXGIAdapter> pDXGIAdapter = nullptr;
	ComPtr<IDXGIFactory> pDXGIFactory = nullptr;

	m_Device->QueryInterface(__uuidof(IDXGIDevice), (void**)pDXGIDevice.GetAddressOf());
	pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void**)pDXGIAdapter.GetAddressOf());
	pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), (void**)pDXGIFactory.GetAddressOf());

	if (FAILED(pDXGIFactory->CreateSwapChain(m_Device.Get(), &Desc, m_SwapChain.GetAddressOf())))
	{
		return E_FAIL;
	}


	pDXGIFactory->MakeWindowAssociation(m_hMainWnd, DXGI_MWA_NO_ALT_ENTER);
	return S_OK;
}

int DX11Device::CreateRenderTargetTexAndView()
{
	// 1. 获取交换链的后台缓冲区纹理 get swap chain's back buffer texture
	ComPtr<ID3D11Texture2D> Tex2D = nullptr;
	m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)Tex2D.GetAddressOf());

	// 2. 交由AssetMgr管理并创建相应视图 handle it to AssetMgr and create corresponding views
	CAssetMgr::GetInst()->CreateTexture(L"RenderTargetTex", Tex2D);

	return S_OK;
}

int DX11Device::CreateConstantBuffer()
{
	// 这四个常量缓冲区的生命期和设备一样长, 数据每帧更新
	// The lifetime of these four constant buffers is as long as the device, and their data is updated every frame
	m_CB[(UINT)CB_TYPE::TRANSFORM] = new CConstantBuffer;
	m_CB[(UINT)CB_TYPE::TRANSFORM]->Create(sizeof(tTransform), CB_TYPE::TRANSFORM);

	m_CB[(UINT)CB_TYPE::MATERIAL] = new CConstantBuffer;
	m_CB[(UINT)CB_TYPE::MATERIAL]->Create(sizeof(tMaterialConst), CB_TYPE::MATERIAL);

	m_CB[(UINT)CB_TYPE::ANIMATION] = new CConstantBuffer;
	m_CB[(UINT)CB_TYPE::ANIMATION]->Create(sizeof(tAnim2DInfo), CB_TYPE::ANIMATION);

	m_CB[(UINT)CB_TYPE::GLOBAL] = new CConstantBuffer;
	m_CB[(UINT)CB_TYPE::GLOBAL]->Create(sizeof(tGlobalData), CB_TYPE::GLOBAL);


	return S_OK;
}

int DX11Device::CreateSamplerState()
{
	D3D11_SAMPLER_DESC Desc[3] = {};

	Desc[0].AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	Desc[0].AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	Desc[0].AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	Desc[0].Filter = D3D11_FILTER_ANISOTROPIC;
	Desc[0].MaxAnisotropy = 16;
	Desc[0].MaxLOD = D3D11_FLOAT32_MAX;
	DEVICE->CreateSamplerState(Desc, m_Sampler[0].GetAddressOf());
	CONTEXT->PSSetSamplers(0, 1, m_Sampler[0].GetAddressOf());
	CONTEXT->CSSetSamplers(0, 1, m_Sampler[0].GetAddressOf());

	// TODO: remove duplicate
	Desc[1].AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	Desc[1].AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	Desc[1].AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	Desc[1].Filter = D3D11_FILTER_ANISOTROPIC;
	Desc[1].MaxAnisotropy = 16;
	Desc[1].MaxLOD = D3D11_FLOAT32_MAX;
	DEVICE->CreateSamplerState(Desc + 1, m_Sampler[1].GetAddressOf());
	CONTEXT->PSSetSamplers(1, 1, m_Sampler[1].GetAddressOf());
	CONTEXT->CSSetSamplers(1, 1, m_Sampler[1].GetAddressOf());

	Desc[2].AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	Desc[2].AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	Desc[2].AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	Desc[2].Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	Desc[2].MaxAnisotropy = 16;
	Desc[2].MaxLOD = D3D11_FLOAT32_MAX;
	DEVICE->CreateSamplerState(Desc + 2, m_Sampler[2].GetAddressOf());
	CONTEXT->PSSetSamplers(2, 1, m_Sampler[2].GetAddressOf());
	CONTEXT->CSSetSamplers(2, 1, m_Sampler[2].GetAddressOf());

	return S_OK;
}

int DX11Device::CreateRasterizerState()
{
	// Cull back is the default state
	m_RS[(UINT)RS_TYPE::CULL_BACK] = nullptr;

	// CULL FRONT
	D3D11_RASTERIZER_DESC Desc = {};
	Desc.CullMode = D3D11_CULL_FRONT;
	Desc.FillMode = D3D11_FILL_SOLID;
	DEVICE->CreateRasterizerState(&Desc, m_RS[(UINT)RS_TYPE::CULL_FRONT].GetAddressOf());

	// CULL NONE
	Desc.CullMode = D3D11_CULL_NONE;
	Desc.FillMode = D3D11_FILL_SOLID;
	DEVICE->CreateRasterizerState(&Desc, m_RS[(UINT)RS_TYPE::CULL_NONE].GetAddressOf());

	// WIREFRAME
	Desc.CullMode = D3D11_CULL_NONE;
	Desc.FillMode = D3D11_FILL_WIREFRAME;
	DEVICE->CreateRasterizerState(&Desc, m_RS[(UINT)RS_TYPE::WIRE_FRAME].GetAddressOf());

	return 0;
}


int DX11Device::init(HWND _hWnd, Vec2 _Resolution)
{
	m_hMainWnd = _hWnd;
	m_RenderResolution = _Resolution;
	g_GlobalData.vResolution = _Resolution;

	UINT iFlag = 0;
#ifdef _DEBUG
	iFlag = D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

	// 1.创建设备和设备上下文
	// 1.Create Device and Context
	// 设备是用来管理GPU资源和渲染管线的对象
	// 上下文是用来记录和执行渲染命令的对象
	if (FAILED(D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		iFlag,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		m_Device.GetAddressOf(),
		&featureLevel,
		m_Context.GetAddressOf())))
	{
		return E_FAIL;
	}
	g_pRHIDevice = this;

	// 2.创建swapchain 交换链
	// 交换链是用来管理前后台缓冲区的对象
	if(FAILED(CreateSwapChain()))
	{
		return E_FAIL;
	}

	// 3.创建命令列表 create command list
	m_pCommandList = new DX11CommandList(m_Context.Get());

	// 4.创建back buffer的纹理和视图 create back buffer texture and view
	if(FAILED(CreateRenderTargetTexAndView()))
	{
		return E_FAIL;
	}

	// 4.设置视口
	D3D11_VIEWPORT viewport = {};

	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = (float)m_RenderResolution.x;
	viewport.Height = (float)m_RenderResolution.y;

	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	m_Context->RSSetViewports(1, &viewport);

	// 5.创建常量缓冲区
	if (FAILED(CreateConstantBuffer()))
	{
		return E_FAIL;
	}
	// 6.创建采样器状态
	if (FAILED(CreateSamplerState()))
	{
		return E_FAIL;
	}

	// 7.创建光栅化器状态
	if (FAILED(CreateRasterizerState()))
	{
		return E_FAIL;
	}

	// 8.创建深度模板状态
	if (FAILED(CreateDepthStencilState()))
	{
		return E_FAIL;
	}
	// 9.创建混合状态
	if (FAILED(CreateBlendState()))
	{
		return E_FAIL;
	}

	return S_OK;
}


void DX11Device::ResizeSwapChain(UINT _Width, UINT _Height)
{
	if (!m_SwapChain || _Width == 0 || _Height == 0)
	return;


	// ResizeBuffers会丢失之前的渲染目标视图和深度模板视图, 需要重新创建
	HRESULT hr = m_SwapChain->ResizeBuffers(
		0,
		_Width,
		_Height,
		DXGI_FORMAT_UNKNOWN,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
	);

	if (FAILED(hr))
		return;

	// 5. 更新分辨率
	m_RenderResolution = Vec2((float)_Width, (float)_Height);
	g_GlobalData.vResolution = m_RenderResolution;

	// 6. 重新创建视图（会在 AssetMgr 中重新注册纹理）
	CreateRenderTargetTexAndView();

	// 7. 更新视口
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = (float)_Width;
	viewport.Height = (float)_Height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	m_Context->RSSetViewports(1, &viewport);

}

int DX11Device::CreateDepthStencilState()
{
	m_DS[(UINT)DS_TYPE::LESS] = nullptr;

	// ==========
	// Less Equal
	// ==========
	D3D11_DEPTH_STENCIL_DESC Desc = {};
	Desc.DepthEnable = true;
	Desc.StencilEnable = false;
	Desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	Desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; // 通过测试就把深度写进深度缓冲 

	DEVICE->CreateDepthStencilState(&Desc, m_DS[(UINT)DS_TYPE::LESS_EQUAL].GetAddressOf());
	// ==========
	// Greater 
	// ==========
	Desc.DepthEnable = true;
	Desc.StencilEnable = false;
	Desc.DepthFunc = D3D11_COMPARISON_GREATER;
	Desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; // 通过测试就把深度写进深度缓冲 

	DEVICE->CreateDepthStencilState(&Desc, m_DS[(UINT)DS_TYPE::GREATER].GetAddressOf());
	// ==========
	// No Write
	// ==========
	Desc.DepthEnable = true;
	Desc.StencilEnable = false;
	Desc.DepthFunc = D3D11_COMPARISON_LESS;
	Desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; 

	DEVICE->CreateDepthStencilState(&Desc, m_DS[(UINT)DS_TYPE::NO_WRITE].GetAddressOf());
	// ==========
	// No Test
	// ==========
	Desc.DepthEnable = true;
	Desc.StencilEnable = false;
	Desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	Desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; 

	DEVICE->CreateDepthStencilState(&Desc, m_DS[(UINT)DS_TYPE::NO_TEST].GetAddressOf());
	// ==========
	// No Test No Write
	// ==========
	Desc.DepthEnable = true;
	Desc.StencilEnable = false;
	Desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	Desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; 

	DEVICE->CreateDepthStencilState(&Desc, m_DS[(UINT)DS_TYPE::NO_TEST_NO_WRITE].GetAddressOf());

	// ==========
	// Backface & FrontFace Check, RS_TYPE == CULL_NONE
	// ==========
	Desc.DepthEnable = true;
	Desc.StencilEnable = true;
	Desc.DepthFunc = D3D11_COMPARISON_GREATER; // light volume surface is behind scene geometry
	Desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; 
	Desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	Desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	Desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	Desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_DECR;
	Desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	Desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_INCR;

	Desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	Desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_INCR;
	Desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	Desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_DECR;

	DEVICE->CreateDepthStencilState(&Desc, m_DS[(UINT)DS_TYPE::BACKFACE_CHECK].GetAddressOf());


	// ==========
	// Stencil Check (for Volume Mesh for Deferred Point Light)
	// ==========
	Desc.DepthEnable = true;
	Desc.StencilEnable = true;
	Desc.DepthFunc = D3D11_COMPARISON_NEVER; 
	Desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; 
	Desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	Desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	Desc.FrontFace.StencilFunc = D3D11_COMPARISON_GREATER;
	Desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_ZERO;
	Desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	Desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_ZERO;

	Desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	Desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	Desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	Desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;

	DEVICE->CreateDepthStencilState(&Desc, m_DS[(UINT)DS_TYPE::STENCIL_CHECK].GetAddressOf());

	return S_OK;
}

int DX11Device::CreateBlendState()
{
	m_BS[(UINT)BS_TYPE::DEFAULT] = nullptr;

	D3D11_BLEND_DESC Desc = {};
	// ==========
	// ALPHA BLEND
	// ==========
	Desc.AlphaToCoverageEnable = false; 
	Desc.IndependentBlendEnable = false; // 这个参数的意思是是否允许对多个渲染目标使用不同的混合状态
	Desc.RenderTarget[0].BlendEnable = true;

	Desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	Desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	Desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	Desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;

	Desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	Desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	Desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;

	HRESULT hr = DEVICE->CreateBlendState(&Desc, m_BS[(UINT)BS_TYPE::ALPHA_BLEND].GetAddressOf());

	// ==========
	// ALPHA TO COVERAGE
	// ==========
	Desc.AlphaToCoverageEnable = true; // 这需要求多重采样开启(MSAA)
	Desc.IndependentBlendEnable = false; // 这个参数的意思是是否允许对多个渲染目标使用不同的混合状态
	Desc.RenderTarget[0].BlendEnable = true;

	Desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	Desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	Desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	Desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;

	Desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	Desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	Desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;

	hr = DEVICE->CreateBlendState(&Desc, m_BS[(UINT)BS_TYPE::ALPHA_TO_COVERAGE].GetAddressOf());
	// ==========
	// ONE_ONE
	// ==========
	Desc.AlphaToCoverageEnable = false;
	Desc.IndependentBlendEnable = false; // 这个参数的意思是是否允许对多个渲染目标使用不同的混合状态
	Desc.RenderTarget[0].BlendEnable = true;

	Desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	Desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	Desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	Desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	Desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	Desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	Desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;

	hr = DEVICE->CreateBlendState(&Desc, m_BS[(UINT)BS_TYPE::ONE_ONE].GetAddressOf());

	// ==========
	// DECAL_BLEND
	// ==========
	Desc.AlphaToCoverageEnable = false;
	Desc.IndependentBlendEnable = true; // 这个参数的意思是是否允许对多个渲染目标使用不同的混合状态

	Desc.RenderTarget[0].BlendEnable = true;
	Desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	Desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	Desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	Desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	Desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	Desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	Desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;

	Desc.RenderTarget[1].BlendEnable = true;
	Desc.RenderTarget[1].BlendOp = D3D11_BLEND_OP_ADD;
	Desc.RenderTarget[1].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	Desc.RenderTarget[1].SrcBlend = D3D11_BLEND_ONE;
	Desc.RenderTarget[1].DestBlend = D3D11_BLEND_ONE;
	Desc.RenderTarget[1].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	Desc.RenderTarget[1].SrcBlendAlpha = D3D11_BLEND_ONE;
	Desc.RenderTarget[1].DestBlendAlpha = D3D11_BLEND_ZERO;

	hr = DEVICE->CreateBlendState(&Desc, m_BS[(UINT)BS_TYPE::DECAL_BLEND].GetAddressOf());

	return hr;
}

IRHIGraphicsShader* DX11Device::CreateGraphicsShader() { return new DX11GraphicsShader(); }
IRHIComputeShader*  DX11Device::CreateComputeShader()  { return new DX11ComputeShader();  }
IRHIPipelineState*  DX11Device::CreatePipelineState()  { return new DX11PipelineState();  }
IRHIBuffer*         DX11Device::CreateBuffer()         { return new DX11Buffer();         }
IRHITexture*        DX11Device::CreateTexture()        { return new DX11Texture();        }

#endif // USE_DX11
