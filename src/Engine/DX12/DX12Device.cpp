#include "dx12pch.h"
#include "DX12Device.h"

#include "../CAssetMgr.h"
#include "../CTexture.h"

#include "DX12ResourceStateTracker.h"
#include "DX12ConstantBuffer.h"
#include "DX12CommandList.h"
#include "DX12CommandQueue.h"
#include "DX12DescriptorAllocator.h"
#include "DX12Texture.h"
#include "DX12RootSignature.h"

#include "DX12RHICommandList.h"
#include "DX12RHIBuffer.h"
#include "DX12RHITexture.h"
#include "DX12GraphicsShader.h"
#include "DX12ComputeShader.h"
#include "DX12PipelineState.h"

#include "../CConstantBuffer.h"

DX12Device::DX12Device()
	: m_hMainWnd(nullptr)
	, m_bVSync(false)
	, m_bResizing(false)
	, m_bTearingSupported(false)
    , m_CurrentBackBufferIndex(0)
	, m_pCommandList(nullptr)
{
}

DX12Device::~DX12Device()
{
    Flush();
	Safe_Del_Array<CConstantBuffer, (UINT)CB_TYPE::END>(m_CB);
    delete m_pCommandList;
}

int DX12Device::init(HWND _hWnd, Vec2 _Resolution)
{
	m_hMainWnd = _hWnd;
	m_RenderResolution = _Resolution;
	g_GlobalData.vResolution = _Resolution;

	// 0. 启用D3D12调试层(如果在Debug模式下)
#if defined(_DEBUG)
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();
			OutputDebugStringA("D3D12 Debug Layer Enabled.\n");
		}
    }
#endif


	// 1.创建设备
	m_Device = CreateDevice(GetAdapter(false));


	// 2.创建命令队列
	m_DirectCommandQueue = std::make_shared<DX12CommandQueue>(D3D12_COMMAND_LIST_TYPE_DIRECT);
	m_ComputeCommandQueue = std::make_shared<DX12CommandQueue>(D3D12_COMMAND_LIST_TYPE_COMPUTE);
	m_CopyCommandQueue = std::make_shared<DX12CommandQueue>(D3D12_COMMAND_LIST_TYPE_COPY);

	// 3.创建swapchain 交换链
	// 交换链是用来管理前后台缓冲区的对象
	m_bTearingSupported = CheckTearingSupport();
	m_SwapChain = CreateSwapChain();

	// 4.创建描述符分配器
	// * 5中的createrendertargettexandview会用到分配器来创建RTV描述符
	for (UINT i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
    {
        m_DescriptorAllocators[i] = std::make_unique<DX12DescriptorAllocator>(static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i));
    }

    // ! NOTE: 
	{
		m_NullDescriptors = AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 2);

		// Null SRV (Texture2D, returns 0 on shader read)
		D3D12_SHADER_RESOURCE_VIEW_DESC nullSrvDesc = {};
		nullSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		nullSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		nullSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		nullSrvDesc.Texture2D.MipLevels = 1;
		m_Device->CreateShaderResourceView(nullptr, &nullSrvDesc, m_NullDescriptors.GetDescriptorHandle(0));

		// Null UAV (Texture2D)
		D3D12_UNORDERED_ACCESS_VIEW_DESC nullUavDesc = {};
		nullUavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		nullUavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		m_Device->CreateUnorderedAccessView(nullptr, nullptr, &nullUavDesc, m_NullDescriptors.GetDescriptorHandle(1));
	}

	// 5.创建back buffer的纹理和视图 create back buffer texture and view
	if(FAILED(CreateRenderTargetTexAndView()))
	{
		return E_FAIL;
	}

	// 6.create root signatures 
    if(FAILED(CreateRootSignatures()))
    {
        return E_FAIL;
	}

    // 7.create constant buffers
    if (FAILED(CreateConstantBuffer()))
    {
        return E_FAIL;
    }

	// 8.RHI command list
	m_pCommandList = new DX12RHICommandList();

	// 9.创建光栅化器状态
	if (FAILED(CreateRasterizerState()))
	{
		return E_FAIL;
	}

	// 10.创建深度模板状态
	if (FAILED(CreateDepthStencilState()))
	{
		return E_FAIL;
	}
	// 11.创建混合状态
	if (FAILED(CreateBlendState()))
	{
		return E_FAIL;
	}

	return S_OK;

}

void DX12Device::ClearTarget(float(&_ArrColor)[4])
{
    if (!m_CurrentCmdList) return;

    auto& backBuffer = m_BackBufferTextures[m_CurrentBackBufferIndex];
    m_CurrentCmdList->TransitionBarrier(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET,
                                        D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, true);

    D3D12_CPU_DESCRIPTOR_HANDLE rtv = backBuffer.GetRenderTargetView();
    m_CurrentCmdList->GetGraphicsCommandList()->ClearRenderTargetView(rtv, _ArrColor, 0, nullptr);
}

void DX12Device::Present()
{
	auto commandQueue = GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);

    // If we have an active command list, transition back buffer and execute
    if (m_CurrentCmdList)
    {
        auto& backBuffer = m_BackBufferTextures[m_CurrentBackBufferIndex];
        m_CurrentCmdList->TransitionBarrier(backBuffer, D3D12_RESOURCE_STATE_PRESENT);
        commandQueue->ExecuteCommandList(m_CurrentCmdList);
        m_CurrentCmdList.reset();

        // FIX: Synchronize the RHI wrapper so its m_CmdList cannot double-submit
        // the already-executed command list (e.g. during Resize->Flush()).
        // Without this, the shared_ptr in m_pCommandList keeps CL alive in the pool
        // while it is also submitted again, causing CL to be pushed to pool twice
        // and later used simultaneously as the active CL and a SetData upload buffer.
        if (m_pCommandList)
            static_cast<DX12RHICommandList*>(m_pCommandList)->SetCommandList(nullptr);
    }

    UINT syncInterval = m_bVSync ? 1 : 0;
    UINT presentFlags = m_bTearingSupported && !m_bVSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
    ThrowIfFailed(m_SwapChain->Present(syncInterval, presentFlags));

    m_FenceValues[m_CurrentBackBufferIndex] = commandQueue->Signal();
    m_CurrentBackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

    commandQueue->WaitForFenceValue(m_FenceValues[m_CurrentBackBufferIndex]);

    ++ms_FrameCount;
    ReleaseStaleDescriptors(ms_FrameCount);
}

// imgui 
void DX12Device::PrepareBackBufferForImGui()
{
    if (!m_CurrentCmdList) return;

	auto& backBuffer = m_BackBufferTextures[m_CurrentBackBufferIndex];
    m_CurrentCmdList->TransitionBarrier(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, true);

    D3D12_CPU_DESCRIPTOR_HANDLE rtv = backBuffer.GetRenderTargetView();
	m_CurrentCmdList->GetGraphicsCommandList()->OMSetRenderTargets(1, &rtv, FALSE, nullptr);

}

ComPtr<IDXGIAdapter4> DX12Device::GetAdapter(bool _bUseWarp)
{
    ComPtr<IDXGIFactory4> dxgiFactory;
    UINT createFactoryFlags = 0;
#if defined(_DEBUG)
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

    ComPtr<IDXGIAdapter1> dxgiAdapter1;
    ComPtr<IDXGIAdapter4> dxgiAdapter4;

    if (_bUseWarp)
    {
        ThrowIfFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1)));
        ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
    }
    else
    {
        SIZE_T maxDedicatedVideoMemory = 0;
        for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
        {
            DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
            dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

            // Check to see if the adapter can create a D3D12 device without actually 
            // creating it. The adapter with the largest dedicated video memory
            // is favored.
            if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
                SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(),
                    D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) &&
                dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory)
            {
                maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
                ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
            }
        }
    }

    return dxgiAdapter4;
}

ComPtr<ID3D12Device2> DX12Device::CreateDevice(Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter)
{
    ComPtr<ID3D12Device2> d3d12Device2;
    ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3d12Device2)));
//    NAME_D3D12_OBJECT(d3d12Device2);

    // Enable debug messages in debug mode.
#if defined(_DEBUG)
    ComPtr<ID3D12InfoQueue> pInfoQueue;
    if (SUCCEEDED(d3d12Device2.As(&pInfoQueue)))
    {
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
        //pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

        // Suppress whole categories of messages
        //D3D12_MESSAGE_CATEGORY Categories[] = {};

        // Suppress messages based on their severity level
        D3D12_MESSAGE_SEVERITY Severities[] =
        {
            D3D12_MESSAGE_SEVERITY_INFO
        };

        // Suppress individual messages by their ID
        D3D12_MESSAGE_ID DenyIds[] = {
			//D3D12_MESSAGE_ID_COPY_DESCRIPTORS_INVALID_RANGES,				// This started happening after updating to an RTX 2080 Ti. I believe this to be an error in the validation layer itself.
            D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
            D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
            D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
        };

        D3D12_INFO_QUEUE_FILTER NewFilter = {};
        //NewFilter.DenyList.NumCategories = _countof(Categories);
        //NewFilter.DenyList.pCategoryList = Categories;
        NewFilter.DenyList.NumSeverities = _countof(Severities);
        NewFilter.DenyList.pSeverityList = Severities;
        NewFilter.DenyList.NumIDs = _countof(DenyIds);
        NewFilter.DenyList.pIDList = DenyIds;

        ThrowIfFailed(pInfoQueue->PushStorageFilter(&NewFilter));
    }
#endif

    return d3d12Device2;
}

DXGI_SAMPLE_DESC DX12Device::GetMultisampleQualityLevels(DXGI_FORMAT _Format, UINT _NumSamples, D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS _Flags) const
{
    DXGI_SAMPLE_DESC sampleDesc = { 1, 0 };

    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS qualityLevels;
    qualityLevels.Format = _Format;
    qualityLevels.SampleCount = 1;
    qualityLevels.Flags = _Flags;
    qualityLevels.NumQualityLevels = 0;

    while ( qualityLevels.SampleCount <= _NumSamples && SUCCEEDED( m_Device->CheckFeatureSupport( D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &qualityLevels, sizeof( D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS ) ) ) && qualityLevels.NumQualityLevels > 0 )
    {
        // That works...
        sampleDesc.Count = qualityLevels.SampleCount;
        sampleDesc.Quality = qualityLevels.NumQualityLevels - 1;

        // But can we do better?
        qualityLevels.SampleCount *= 2;
    }

    return sampleDesc;
}

DescriptorAllocation DX12Device::AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE _Type, uint32_t _NumDescriptors)
{
	return m_DescriptorAllocators[_Type]->Allocate(_NumDescriptors);
}

void DX12Device::ReleaseStaleDescriptors(uint64_t _FinishedFrame)
{
	for ( int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i )
    {
        m_DescriptorAllocators[i]->ReleaseStaleDescriptors( _FinishedFrame );
    }
}

ComPtr<ID3D12DescriptorHeap> DX12Device::CreateDescriptorHeap(UINT _NumDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE _Type)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = _Type;
    desc.NumDescriptors = _NumDescriptors;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    desc.NodeMask = 0;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;
    ThrowIfFailed(m_Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)));

    return descriptorHeap;
}

UINT DX12Device::GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE _Type) const
{
    return m_Device->GetDescriptorHandleIncrementSize(_Type);
}

void DX12Device::BeginFrame()
{
    auto commandQueue = GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    m_CurrentCmdList  = commandQueue->GetCommandList();

    // Set root signature for the frame
	m_CurrentCmdList->SetComputeRootSignature(m_ComputeRootSignature);
    m_CurrentCmdList->SetGraphicsRootSignature(m_GraphicsRootSignature);

    // Set viewport + scissor
    D3D12_VIEWPORT vp = { 0, 0, m_RenderResolution.x, m_RenderResolution.y, 0.0f, 1.0f };
    D3D12_RECT sr     = { 0, 0, (LONG)m_RenderResolution.x, (LONG)m_RenderResolution.y };
    m_CurrentCmdList->SetViewport(vp);
    m_CurrentCmdList->SetScissorRect(sr);

    // Update the RHI command list wrapper
    static_cast<DX12RHICommandList*>(m_pCommandList)->SetCommandList(m_CurrentCmdList);

    // ! NOTE:
    // Update "RenderTargetTex" to current back buffer (triple buffering)
    Ptr<CTexture> pRTTex = CAssetMgr::GetInst()->FindAsset<CTexture>(L"RenderTargetTex");
    if (pRTTex != nullptr)
    {
        ComPtr<ID3D12Resource> currentBB = m_BackBufferTextures[m_CurrentBackBufferIndex].GetD3D12Resource();
        static_cast<DX12RHITexture*>(pRTTex->GetRHITexture())->CreateFromExisting(
            currentBB, RHI_BIND_FLAG::RENDER_TARGET);
    }
}

void DX12Device::EndFrame()
{
	if (!m_CurrentCmdList)
	return;

    auto queue = GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    uint64_t fence = queue->ExecuteCommandList(m_CurrentCmdList);

    m_CurrentCmdList.reset();
    if (m_pCommandList)
        static_cast<DX12RHICommandList*>(m_pCommandList)->SetCommandList(nullptr);

    queue->WaitForFenceValue(fence);  // IBL 预计算需要同步等待
}

ComPtr<IDXGISwapChain4> DX12Device::CreateSwapChain()
{
    ComPtr<IDXGISwapChain4> dxgiSwapChain4;
    ComPtr<IDXGIFactory4> dxgiFactory4;
    UINT createFactoryFlags = 0;
#if defined(_DEBUG)
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = (UINT)m_RenderResolution.x;
    swapChainDesc.Height = (UINT)m_RenderResolution.y;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc = { 1, 0 };
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = BufferCount;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    // It is recommended to always allow tearing if tearing support is available.
    swapChainDesc.Flags = m_bTearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
    // ! 这里拿的是direct queue
    ID3D12CommandQueue* pCommandQueue = m_DirectCommandQueue->GetD3D12CommandQueue().Get();

    ComPtr<IDXGISwapChain1> swapChain1;
    ThrowIfFailed(dxgiFactory4->CreateSwapChainForHwnd(
        pCommandQueue,
        m_hMainWnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain1));

    // Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
    // will be handled manually.
    ThrowIfFailed(dxgiFactory4->MakeWindowAssociation(m_hMainWnd, DXGI_MWA_NO_ALT_ENTER));

    ThrowIfFailed(swapChain1.As(&dxgiSwapChain4));

    m_CurrentBackBufferIndex = dxgiSwapChain4->GetCurrentBackBufferIndex();

    return dxgiSwapChain4;
}

int DX12Device::CreateRenderTargetTexAndView()
{
    for(UINT i = 0; i < BufferCount; ++i)
    {
        ComPtr<ID3D12Resource> backBuffer;
		ThrowIfFailed(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));

		ResourceStateTracker::AddGlobalResourceState(backBuffer.Get(), D3D12_RESOURCE_STATE_COMMON);

		m_BackBufferTextures[i].SetD3D12Resource(backBuffer);
        m_BackBufferTextures[i].CreateViews();
        m_BackBufferTextures[i].SetName(L"BackBuffer[" + std::to_wstring(i) + L"]");

	}

    // ! NOTE:
    // Register current back buffer as "RenderTargetTex" with CAssetMgr
    ComPtr<ID3D12Resource> currentBB;
    ThrowIfFailed(m_SwapChain->GetBuffer(m_CurrentBackBufferIndex, IID_PPV_ARGS(&currentBB)));
    DX12RHITexture* pRHITex = new DX12RHITexture();
    pRHITex->CreateFromExisting(currentBB, RHI_BIND_FLAG::RENDER_TARGET);
    CAssetMgr::GetInst()->CreateTexture(L"RenderTargetTex", pRHITex);

    return S_OK;
}


// 对照 Window::GetRenderTarget()：每帧调用时 lazy 更新 Color0
const RenderTarget& DX12Device::GetCurrentRenderTarget()
{
    m_RenderTarget.AttachTexture(AttachmentPoint::Color0, m_BackBufferTextures[m_CurrentBackBufferIndex]);
    return m_RenderTarget;
}

void DX12Device::Flush()
{
    if(m_DirectCommandQueue) m_DirectCommandQueue->Flush();
    if(m_ComputeCommandQueue) m_ComputeCommandQueue->Flush();
    if(m_CopyCommandQueue) m_CopyCommandQueue->Flush();
}

// 对照 Window::OnResize()
void DX12Device::ResizeSwapChain(UINT _Width, UINT _Height)
{
	if (!m_SwapChain || _Width == 0 || _Height == 0)
	return;

    Flush();

    // 释放对 back buffer 的引用（必须在 ResizeBuffers 前）
    m_RenderTarget.AttachTexture(AttachmentPoint::Color0, DX12Texture());
    for (UINT i = 0; i < BufferCount; ++i)
    {
        ResourceStateTracker::RemoveGlobalResourceState(m_BackBufferTextures[i].GetD3D12Resource().Get());
        m_BackBufferTextures[i].Reset();
    }

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    ThrowIfFailed(m_SwapChain->GetDesc(&swapChainDesc));
    ThrowIfFailed(m_SwapChain->ResizeBuffers(
        BufferCount, _Width, _Height,
        swapChainDesc.BufferDesc.Format,
        swapChainDesc.Flags));

    m_CurrentBackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
    m_RenderResolution = Vec2((float)_Width, (float)_Height);
    g_GlobalData.vResolution  = m_RenderResolution;

    CreateRenderTargetTexAndView();
}

IRHICommandList* DX12Device::GetCommandList()
{
    return m_pCommandList;
}

std::shared_ptr<DX12CommandQueue> DX12Device::GetCommandQueue(D3D12_COMMAND_LIST_TYPE _Type) const
{
	std::shared_ptr<DX12CommandQueue> commandQueue;
    switch (_Type)
    {
    case D3D12_COMMAND_LIST_TYPE_DIRECT:
        commandQueue = m_DirectCommandQueue;
        break;
    case D3D12_COMMAND_LIST_TYPE_COMPUTE:
        commandQueue = m_ComputeCommandQueue;
        break;
    case D3D12_COMMAND_LIST_TYPE_COPY:
        commandQueue = m_CopyCommandQueue;
        break;
    default:
        assert(false && "Invalid command queue type.");
    }

    return commandQueue;
}

// --- (append to the end of the file) ---

// Static member definition
uint64_t DX12Device::ms_FrameCount = 0;

bool DX12Device::CheckTearingSupport()
{
    BOOL allowTearing = FALSE;
    ComPtr<IDXGIFactory4> factory4;
    if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
    {
        ComPtr<IDXGIFactory5> factory5;
        if (SUCCEEDED(factory4.As(&factory5)))
        {
            if (FAILED(factory5->CheckFeatureSupport(
                DXGI_FEATURE_PRESENT_ALLOW_TEARING,
                &allowTearing, sizeof(allowTearing))))
            {
                allowTearing = FALSE;
            }
        }
    }
    return allowTearing == TRUE;
}

int DX12Device::CreateRasterizerState()
{
    // CULL_BACK (default)
    D3D12_RASTERIZER_DESC desc = {};
    desc.FillMode = D3D12_FILL_MODE_SOLID;
    desc.CullMode = D3D12_CULL_MODE_BACK;
    desc.FrontCounterClockwise = FALSE;
    desc.DepthClipEnable = TRUE;
    m_RS[(UINT)RS_TYPE::CULL_BACK] = desc;

    // CULL_FRONT
    desc.CullMode = D3D12_CULL_MODE_FRONT;
    m_RS[(UINT)RS_TYPE::CULL_FRONT] = desc;

    // CULL_NONE
    desc.CullMode = D3D12_CULL_MODE_NONE;
    desc.FillMode = D3D12_FILL_MODE_SOLID;
    m_RS[(UINT)RS_TYPE::CULL_NONE] = desc;

    // WIRE_FRAME
    desc.CullMode = D3D12_CULL_MODE_NONE;
    desc.FillMode = D3D12_FILL_MODE_WIREFRAME;
    m_RS[(UINT)RS_TYPE::WIRE_FRAME] = desc;

    return S_OK;
}

int DX12Device::CreateDepthStencilState()
{
    // Helper: default stencil op (keep/keep/keep, always pass)
    D3D12_DEPTH_STENCILOP_DESC defaultStencilOp = {};
    defaultStencilOp.StencilFailOp      = D3D12_STENCIL_OP_KEEP;
    defaultStencilOp.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    defaultStencilOp.StencilPassOp      = D3D12_STENCIL_OP_KEEP;
    defaultStencilOp.StencilFunc        = D3D12_COMPARISON_FUNC_ALWAYS;

    D3D12_DEPTH_STENCIL_DESC desc = {};
    desc.DepthEnable    = TRUE;
    desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    desc.StencilEnable  = FALSE;
    desc.StencilReadMask  = D3D12_DEFAULT_STENCIL_READ_MASK;
    desc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    desc.FrontFace = defaultStencilOp;
    desc.BackFace  = defaultStencilOp;

    // LESS (default)
    desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    m_DS[(UINT)DS_TYPE::LESS] = desc;

    // LESS_EQUAL
    desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    m_DS[(UINT)DS_TYPE::LESS_EQUAL] = desc;

    // GREATER
    desc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
    m_DS[(UINT)DS_TYPE::GREATER] = desc;

    // NO_WRITE
    desc.DepthFunc      = D3D12_COMPARISON_FUNC_LESS;
    desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    m_DS[(UINT)DS_TYPE::NO_WRITE] = desc;

    // NO_TEST
    desc.DepthFunc      = D3D12_COMPARISON_FUNC_ALWAYS;
    desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    m_DS[(UINT)DS_TYPE::NO_TEST] = desc;

    // NO_TEST_NO_WRITE
    desc.DepthEnable = FALSE;
    desc.DepthFunc      = D3D12_COMPARISON_FUNC_ALWAYS;
    desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    m_DS[(UINT)DS_TYPE::NO_TEST_NO_WRITE] = desc;
	desc.DepthEnable = TRUE; // restore for next ones

    // BACKFACE_CHECK
    {
        D3D12_DEPTH_STENCIL_DESC d = {};
        d.DepthEnable     = TRUE;
        d.DepthWriteMask  = D3D12_DEPTH_WRITE_MASK_ZERO;
        d.DepthFunc       = D3D12_COMPARISON_FUNC_GREATER;
        d.StencilEnable   = TRUE;
        d.StencilReadMask  = D3D12_DEFAULT_STENCIL_READ_MASK;
        d.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
        d.BackFace  = { D3D12_STENCIL_OP_DECR, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_INCR, D3D12_COMPARISON_FUNC_ALWAYS };
        d.FrontFace = { D3D12_STENCIL_OP_INCR, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_DECR, D3D12_COMPARISON_FUNC_ALWAYS };
        m_DS[(UINT)DS_TYPE::BACKFACE_CHECK] = d;
    }

    // FRONTFACE_CHECK
    {
        D3D12_DEPTH_STENCIL_DESC d = {};
        d.DepthEnable     = TRUE;
        d.DepthWriteMask  = D3D12_DEPTH_WRITE_MASK_ZERO;
        d.DepthFunc       = D3D12_COMPARISON_FUNC_NEVER;
        d.StencilEnable   = TRUE;
        d.StencilReadMask  = D3D12_DEFAULT_STENCIL_READ_MASK;
        d.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
        d.FrontFace = { D3D12_STENCIL_OP_ZERO, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_ZERO, D3D12_COMPARISON_FUNC_GREATER };
        d.BackFace  = d.FrontFace;
        m_DS[(UINT)DS_TYPE::FRONTFACE_CHECK] = d;
    }

    // STENCIL_CHECK
    {
        D3D12_DEPTH_STENCIL_DESC d = {};
        d.DepthEnable     = TRUE;
        d.DepthWriteMask  = D3D12_DEPTH_WRITE_MASK_ZERO;
        d.DepthFunc       = D3D12_COMPARISON_FUNC_NEVER;
        d.StencilEnable   = TRUE;
        d.StencilReadMask  = D3D12_DEFAULT_STENCIL_READ_MASK;
        d.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
        d.FrontFace = { D3D12_STENCIL_OP_ZERO, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_ZERO, D3D12_COMPARISON_FUNC_GREATER };
        d.BackFace  = d.FrontFace;
        m_DS[(UINT)DS_TYPE::STENCIL_CHECK] = d;
    }

    return S_OK;
}

int DX12Device::CreateBlendState()
{
    // Helper: default RT blend (blending off)
    D3D12_RENDER_TARGET_BLEND_DESC noBlend = {};
    noBlend.BlendEnable   = FALSE;
    noBlend.LogicOpEnable = FALSE;
    noBlend.SrcBlend      = D3D12_BLEND_ONE;
    noBlend.DestBlend     = D3D12_BLEND_ZERO;
    noBlend.BlendOp       = D3D12_BLEND_OP_ADD;
    noBlend.SrcBlendAlpha  = D3D12_BLEND_ONE;
    noBlend.DestBlendAlpha = D3D12_BLEND_ZERO;
    noBlend.BlendOpAlpha  = D3D12_BLEND_OP_ADD;
    noBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    D3D12_BLEND_DESC desc = {};
    desc.AlphaToCoverageEnable  = FALSE;
    desc.IndependentBlendEnable = FALSE;
    for (int i = 0; i < 8; ++i) desc.RenderTarget[i] = noBlend;

    // DEFAULT (no blend)
    m_BS[(UINT)BS_TYPE::DEFAULT] = desc;

    // ALPHA_BLEND
    desc.RenderTarget[0].BlendEnable    = TRUE;
    desc.RenderTarget[0].SrcBlend       = D3D12_BLEND_SRC_ALPHA;
    desc.RenderTarget[0].DestBlend      = D3D12_BLEND_INV_SRC_ALPHA;
    desc.RenderTarget[0].BlendOp        = D3D12_BLEND_OP_ADD;
    desc.RenderTarget[0].SrcBlendAlpha  = D3D12_BLEND_ONE;
    desc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
    desc.RenderTarget[0].BlendOpAlpha   = D3D12_BLEND_OP_ADD;
    m_BS[(UINT)BS_TYPE::ALPHA_BLEND] = desc;

    // ALPHA_TO_COVERAGE
    desc.AlphaToCoverageEnable = TRUE;
    m_BS[(UINT)BS_TYPE::ALPHA_TO_COVERAGE] = desc;
    desc.AlphaToCoverageEnable = FALSE;

    // ONE_ONE
    desc.RenderTarget[0].SrcBlend  = D3D12_BLEND_ONE;
    desc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
    m_BS[(UINT)BS_TYPE::ONE_ONE] = desc;

    // DECAL_BLEND
    desc.IndependentBlendEnable = TRUE;
    desc.RenderTarget[0].SrcBlend       = D3D12_BLEND_SRC_ALPHA;
    desc.RenderTarget[0].DestBlend      = D3D12_BLEND_INV_SRC_ALPHA;
    desc.RenderTarget[0].SrcBlendAlpha  = D3D12_BLEND_ONE;
    desc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
    desc.RenderTarget[1]                = noBlend;
    desc.RenderTarget[1].BlendEnable    = TRUE;
    desc.RenderTarget[1].SrcBlend       = D3D12_BLEND_ONE;
    desc.RenderTarget[1].DestBlend      = D3D12_BLEND_ONE;
    desc.RenderTarget[1].SrcBlendAlpha  = D3D12_BLEND_ONE;
    desc.RenderTarget[1].DestBlendAlpha = D3D12_BLEND_ZERO;
    desc.RenderTarget[1].BlendOp        = D3D12_BLEND_OP_ADD;
    desc.RenderTarget[1].BlendOpAlpha   = D3D12_BLEND_OP_ADD;
    m_BS[(UINT)BS_TYPE::DECAL_BLEND] = desc;

    return S_OK;
}

int DX12Device::CreateRootSignatures()
{
    // === Graphics Root Signature ===
    // [0] Root CBV b0 (TRANSFORM)
    // [1] Root CBV b1 (MATERIAL)
    // [2] Root CBV b2 (ANIMATION2D)
    // [3] Root CBV b3 (GLOBAL DATA)
    // [4] Descriptor Table: 32 SRVs t0-t31
    // [5] Descriptor Table: 8 UAVs  u0-u7
    // Static Samplers: s0 (aniso/wrap), s1 (point/wrap)

	CD3DX12_ROOT_PARAMETER1 rootParameters[6] = {};
	rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[1].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[2].InitAsConstantBufferView(2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[3].InitAsConstantBufferView(3, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);

	// SRV table: t0-t31
    CD3DX12_DESCRIPTOR_RANGE1 srvRange;
    srvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 32, 0, 0,
        D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
    rootParameters[4].InitAsDescriptorTable(1, &srvRange, D3D12_SHADER_VISIBILITY_ALL);

    // UAV table: u0-u7
    CD3DX12_DESCRIPTOR_RANGE1 uavRange;
    uavRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 8, 0, 0,
        D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
    rootParameters[5].InitAsDescriptorTable(1, &uavRange, D3D12_SHADER_VISIBILITY_ALL);

    // Static samplers
    CD3DX12_STATIC_SAMPLER_DESC samplers[3] = {};
    // s0: anisotropic + wrap
    samplers[0].Init(0, D3D12_FILTER_ANISOTROPIC,
        D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP);
    samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    // s1: point + wrap
    samplers[1].Init(1, D3D12_FILTER_MIN_MAG_MIP_POINT,
        D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP);
    samplers[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    // s2: linear + clamp (g_Sam_Clamp - used by IBL / sky / UI sampling)
    samplers[2].Init(2, D3D12_FILTER_MIN_MAG_MIP_LINEAR,
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
    samplers[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;


    D3D12_ROOT_SIGNATURE_DESC1 rootSigDesc = {};
    rootSigDesc.NumParameters     = _countof(rootParameters);
    rootSigDesc.pParameters       = rootParameters;
    rootSigDesc.NumStaticSamplers = _countof(samplers);
    rootSigDesc.pStaticSamplers   = samplers;
    rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    m_GraphicsRootSignature.SetRootSignatureDesc(rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_1);

    // === Compute Root Signature (same layout, no IA flag) ===
    rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
    m_ComputeRootSignature.SetRootSignatureDesc(rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_1);

    return S_OK;
}

// NOTE: 
int DX12Device::CreateConstantBuffer()
{
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


// ── Factory Methods ──────────────────────────────────────────────────────
IRHIGraphicsShader* DX12Device::CreateGraphicsShader()
{
    return new DX12GraphicsShader();
}

IRHIComputeShader* DX12Device::CreateComputeShader()
{
    return new DX12ComputeShader();
}

IRHIPipelineState* DX12Device::CreatePipelineState()
{
    return new DX12PipelineState();
}

IRHIBuffer* DX12Device::CreateBuffer()
{
    return new DX12RHIBuffer();
}

IRHITexture* DX12Device::CreateTexture()
{
    return new DX12RHITexture();
}

// ============================================================
// ImGui shader-visible SRV descriptor heap
// ============================================================
int DX12Device::InitImGuiSrvHeap()
{
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.NumDescriptors = ImGuiSrvHeapSize;
    desc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    desc.NodeMask       = 0;

    HRESULT hr = m_Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_ImGuiSrvHeap));
    if (FAILED(hr))
        return E_FAIL;

    m_ImGuiSrvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_ImGuiSrvNextFreeSlot   = 1; // slot 0 = ImGui font texture
    return S_OK;
}

UINT DX12Device::AllocateImGuiSrvSlot()
{
    assert(m_ImGuiSrvNextFreeSlot < ImGuiSrvHeapSize && "ImGui SRV heap exhausted!");
    return m_ImGuiSrvNextFreeSlot++;
}

void DX12Device::CopyToImGuiSrvHeap(UINT _SlotIndex, D3D12_CPU_DESCRIPTOR_HANDLE _SrcCpuHandle)
{
    D3D12_CPU_DESCRIPTOR_HANDLE dstHandle = GetImGuiSrvCpuHandle(_SlotIndex);
    m_Device->CopyDescriptorsSimple(1, dstHandle, _SrcCpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

ID3D12DescriptorHeap* DX12Device::GetImGuiSrvHeap() const
{
    return m_ImGuiSrvHeap.Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12Device::GetImGuiSrvCpuHandle(UINT _Index) const
{
    D3D12_CPU_DESCRIPTOR_HANDLE handle = m_ImGuiSrvHeap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += (SIZE_T)_Index * m_ImGuiSrvDescriptorSize;
    return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE DX12Device::GetImGuiSrvGpuHandle(UINT _Index) const
{
    D3D12_GPU_DESCRIPTOR_HANDLE handle = m_ImGuiSrvHeap->GetGPUDescriptorHandleForHeapStart();
    handle.ptr += (UINT64)_Index * m_ImGuiSrvDescriptorSize;
    return handle;
}
