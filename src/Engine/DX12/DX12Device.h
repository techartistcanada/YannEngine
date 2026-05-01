#pragma once

#include "DX12DescriptorAllocation.h"
#include "DX12Texture.h"
#include "DX12RenderTarget.h"
#include "DX12RootSignature.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <memory>

#include "../define.h"
#include "../Singleton.h"
#include "../enum.h"
#include "../RHI/IRHIDevice.h"

using namespace Microsoft::WRL;

class CConstantBuffer;

class DX12CommandQueue;
class DX12DescriptorAllocator;
class DX12RHICommandList;
class DX12CommandList;

class DX12Device
    : public CSingleton<DX12Device>, public IRHIDevice
{
    SINGLE(DX12Device)

private:
    HWND                        m_hMainWnd;
    Vec2                        m_RenderResolution;

    ComPtr<ID3D12Device2>       m_Device;
    ComPtr<IDXGISwapChain4>     m_SwapChain;
    bool                        m_bVSync;
    bool                        m_bResizing;
    bool                        m_bTearingSupported;

    static const UINT           BufferCount = 3;
    UINT                        m_CurrentBackBufferIndex;
    uint64_t                    m_FenceValues[BufferCount] = { 0 };
    // Back buffer 的 Texture 数组（Texture 构造时自动创建 RTV descriptor）
    DX12Texture                m_BackBufferTextures[BufferCount];
    DX12Texture                m_DepthStencilTexture;
    // 当前帧的 RenderTarget（每次 Present 后更新 Color0 指向当前 back buffer）
    mutable RenderTarget                m_RenderTarget;

    D3D12_RASTERIZER_DESC       m_RS[(UINT)RS_TYPE::END];
    D3D12_BLEND_DESC            m_BS[(UINT)BS_TYPE::END];
    D3D12_DEPTH_STENCIL_DESC    m_DS[(UINT)DS_TYPE::END];

	RootSignature			    m_GraphicsRootSignature;
	RootSignature			    m_ComputeRootSignature;
    CConstantBuffer*            m_CB[(UINT)CB_TYPE::END] = {};

    DX12RHICommandList*              m_pCommandList = nullptr;
    std::shared_ptr<DX12CommandList> m_CurrentCmdList;

    std::shared_ptr<DX12CommandQueue>        m_DirectCommandQueue;
    std::shared_ptr<DX12CommandQueue>        m_ComputeCommandQueue;
    std::shared_ptr<DX12CommandQueue>        m_CopyCommandQueue;

    std::unique_ptr<DX12DescriptorAllocator> m_DescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

    // NOTE: deferred shading
    DescriptorAllocation m_NullDescriptors;

    // ImGui shader-visible SRV descriptor heap
    // Slot 0 is reserved for the ImGui font texture.
    // Remaining slots are allocated for user textures displayed in ImGui.
    static const UINT                ImGuiSrvHeapSize = 256;
    ComPtr<ID3D12DescriptorHeap>     m_ImGuiSrvHeap;
    UINT                             m_ImGuiSrvDescriptorSize = 0;
    UINT                             m_ImGuiSrvNextFreeSlot   = 1;

    static uint64_t ms_FrameCount;

public:
    int  init(HWND _hWnd, Vec2 _Resolution);

    void ClearTarget(float(&_ArrColor)[4]);
    void Present();
    void PrepareBackBufferForImGui();
    void Flush();
    void ResizeSwapChain(UINT _Width, UINT _Height);
    bool IsResizing()              const { return m_bResizing; }
    void SetResizing(bool _b)            { m_bResizing = _b; }
    bool IsTearingSupported()      const { return m_bTearingSupported; }
	Vec2 GetRenderResolution()     const { return m_RenderResolution; }

	IRHICommandList*    GetCommandList() override;
	IRHIGraphicsShader* CreateGraphicsShader() override;
	IRHIComputeShader*  CreateComputeShader() override;
	IRHIPipelineState*  CreatePipelineState() override;
	IRHIBuffer*         CreateBuffer() override;
	IRHITexture*        CreateTexture() override;

    ID3D12Device2*      GetD3D12Device() { return m_Device.Get(); }
    std::shared_ptr<DX12CommandQueue> GetCommandQueue(D3D12_COMMAND_LIST_TYPE _Type = D3D12_COMMAND_LIST_TYPE_DIRECT) const;
    const RenderTarget& GetCurrentRenderTarget();

    const D3D12_RASTERIZER_DESC&    GetRS(RS_TYPE _Type)  const { return m_RS[(UINT)_Type]; }
    const D3D12_DEPTH_STENCIL_DESC& GetDS(DS_TYPE _Type)  const { return m_DS[(UINT)_Type]; }
    const D3D12_BLEND_DESC&         GetBS(BS_TYPE _Type)  const { return m_BS[(UINT)_Type]; }

	CConstantBuffer* GetConstantBuffer(CB_TYPE _Type) const { return m_CB[(UINT)_Type]; }
	const RootSignature& GetGraphicsRootSignature() const { return m_GraphicsRootSignature; }
	const RootSignature& GetComputeRootSignature() const { return m_ComputeRootSignature; }
	std::shared_ptr<DX12CommandList> GetCurrentCommandList() const { return m_CurrentCmdList; }


    D3D12_CPU_DESCRIPTOR_HANDLE GetNullSRV() const { return m_NullDescriptors.GetDescriptorHandle(0); }
    D3D12_CPU_DESCRIPTOR_HANDLE GetNullUAV() const { return m_NullDescriptors.GetDescriptorHandle(1); }


    DXGI_SAMPLE_DESC GetMultisampleQualityLevels(DXGI_FORMAT _Format, UINT _NumSamples,
        D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS _Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE) const;

    DescriptorAllocation AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE _Type, uint32_t _NumDescriptors = 1);
    void                 ReleaseStaleDescriptors(uint64_t _FinishedFrame);

    ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(UINT _NumDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE _Type);
    UINT GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE _Type) const;

        // Current OMSetRenderTargets state — used by DX12PipelineState::Create to
    // auto-derive PSO RTV/DSV formats so they always match the bound RTs.
    void SetBoundRTFormats(const DXGI_FORMAT* pRTVFormats, UINT numRTs, DXGI_FORMAT dsvFormat)
    {
        m_BoundNumRTVs   = numRTs;
        m_BoundDSVFormat = dsvFormat;
        for (UINT i = 0; i < 8; ++i)
            m_BoundRTVFormats[i] = (i < numRTs) ? pRTVFormats[i] : DXGI_FORMAT_UNKNOWN;
    }
    UINT        GetBoundNumRTVs()        const { return m_BoundNumRTVs; }
    DXGI_FORMAT GetBoundRTVFormat(UINT i) const { return m_BoundRTVFormats[i]; }
    DXGI_FORMAT GetBoundDSVFormat()      const { return m_BoundDSVFormat; }

    // ImGui shader-visible SRV heap
    int  InitImGuiSrvHeap();
    UINT AllocateImGuiSrvSlot();
    void CopyToImGuiSrvHeap(UINT _SlotIndex, D3D12_CPU_DESCRIPTOR_HANDLE _SrcCpuHandle);
    ID3D12DescriptorHeap*       GetImGuiSrvHeap()      const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetImGuiSrvCpuHandle(UINT _Index) const;
    D3D12_GPU_DESCRIPTOR_HANDLE GetImGuiSrvGpuHandle(UINT _Index) const;

    static uint64_t GetFrameCount() { return ms_FrameCount; }

    void BeginFrame();
    void EndFrame();

private:
    ComPtr<IDXGISwapChain4> CreateSwapChain();
    int CreateRenderTargetTexAndView();
    int CreateRasterizerState();
    int CreateDepthStencilState();
    int CreateBlendState();
	int CreateRootSignatures();
	int CreateConstantBuffer();

    ComPtr<IDXGIAdapter4>  GetAdapter(bool _bUseWarp);
    ComPtr<ID3D12Device2>  CreateDevice(ComPtr<IDXGIAdapter4> _Adapter);
    bool                   CheckTearingSupport();

     // Track current OMSetRenderTargets formats (for PSO auto-format)
    DXGI_FORMAT m_BoundRTVFormats[8] = {};
    UINT        m_BoundNumRTVs       = 0;
    DXGI_FORMAT m_BoundDSVFormat     = DXGI_FORMAT_UNKNOWN;
};