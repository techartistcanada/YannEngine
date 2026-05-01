#include "dx12pch.h"
#include "DX12RHITexture.h"
#include "DX12Device.h"
#include "DX12CommandQueue.h"
#include "DX12CommandList.h"
#include "DX12ResourceStateTracker.h"

#include <DirectXTex.h>

int DX12RHITexture::Create(UINT _Width, UINT _Height, DXGI_FORMAT _Format, RHI_BIND_FLAG _BindFlags)
{
    m_Width     = _Width;
    m_Height    = _Height;
    m_Format    = _Format;
    m_BindFlags = _BindFlags;

    D3D12_RESOURCE_FLAGS resFlags = D3D12_RESOURCE_FLAG_NONE;
    if ((_BindFlags & RHI_BIND_FLAG::RENDER_TARGET) != RHI_BIND_FLAG::NONE)
        resFlags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    if ((_BindFlags & RHI_BIND_FLAG::DEPTH_STENCIL) != RHI_BIND_FLAG::NONE)
        resFlags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    if ((_BindFlags & RHI_BIND_FLAG::UNORDERED_ACCESS) != RHI_BIND_FLAG::NONE)
        resFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    D3D12_RESOURCE_DESC texDesc = {};
    texDesc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Width              = _Width;
    texDesc.Height             = _Height;
    texDesc.DepthOrArraySize   = 1;
    texDesc.MipLevels          = 1;
    texDesc.Format             = _Format;
    texDesc.SampleDesc         = { 1, 0 };
    texDesc.Flags              = resFlags;

    D3D12_CLEAR_VALUE  clearValue  = {};
    D3D12_CLEAR_VALUE* pClearValue = nullptr;

    if ((_BindFlags & RHI_BIND_FLAG::DEPTH_STENCIL) != RHI_BIND_FLAG::NONE)
    {
        clearValue.Format               = _Format;
        clearValue.DepthStencil.Depth   = 1.0f;
        clearValue.DepthStencil.Stencil = 0;
        pClearValue = &clearValue;
    }
    else if ((_BindFlags & RHI_BIND_FLAG::RENDER_TARGET) != RHI_BIND_FLAG::NONE)
    {
        clearValue.Format   = _Format;
        clearValue.Color[0] = 0.0f;
        clearValue.Color[1] = 0.0f;
        clearValue.Color[2] = 0.0f;
        clearValue.Color[3] = 0.0f;
        pClearValue = &clearValue;
    }

    TextureUsage usage = TextureUsage::Albedo;
    if ((_BindFlags & RHI_BIND_FLAG::DEPTH_STENCIL) != RHI_BIND_FLAG::NONE)
        usage = TextureUsage::Depth;
    else if ((_BindFlags & RHI_BIND_FLAG::RENDER_TARGET) != RHI_BIND_FLAG::NONE)
        usage = TextureUsage::RenderTarget;

    // Create the committed resource directly, then hand it to m_Texture via SetD3D12Resource.
    // This avoids the buggy DX12Texture move-assignment path (which silently calls the
    // copy ctor on Resource and dereferences a null clear-value unique_ptr when
    // pClearValue == nullptr).
    auto device = DX12Device::GetInst()->GetD3D12Device();
    Microsoft::WRL::ComPtr<ID3D12Resource> resource;
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_COMMON,
        pClearValue,
        IID_PPV_ARGS(&resource)));

    ResourceStateTracker::AddGlobalResourceState(resource.Get(), D3D12_RESOURCE_STATE_COMMON);

    m_Texture = DX12Texture(usage);          // empty ctor — no resource creation, no buggy assignment path
    m_Texture.SetD3D12Resource(resource, pClearValue);
    m_Texture.CreateViews();

    // ↓↓↓ 加这两行
    assert(m_Texture.GetD3D12Resource().Get() != nullptr && "Create末尾就丢了！");
    OutputDebugStringA(("[DX12RHITexture::Create] this=0x" + std::to_string((uintptr_t)this)
        + " &m_Texture=0x" + std::to_string((uintptr_t)&m_Texture)
        + " resource=0x" + std::to_string((uintptr_t)m_Texture.GetD3D12Resource().Get()) + "\n").c_str());

    assert(m_Texture.GetD3D12Resource().Get() != nullptr && "BRDFLut resource lost right after Create!");
    assert(resource.Get() == m_Texture.GetD3D12Resource().Get() && "SetD3D12Resource didn't store the same pointer!");

    return S_OK;
}

int DX12RHITexture::CreateCubemap(UINT _Size, DXGI_FORMAT _Format, RHI_BIND_FLAG _BindFlags, UINT _MipLevels)
{
    m_Width     = _Size;
    m_Height    = _Size;
    m_Format    = _Format;
    m_BindFlags = _BindFlags;

    D3D12_RESOURCE_FLAGS resFlags = D3D12_RESOURCE_FLAG_NONE;
    if ((_BindFlags & RHI_BIND_FLAG::RENDER_TARGET)    != RHI_BIND_FLAG::NONE)
        resFlags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    if ((_BindFlags & RHI_BIND_FLAG::DEPTH_STENCIL)    != RHI_BIND_FLAG::NONE)
        resFlags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    if ((_BindFlags & RHI_BIND_FLAG::UNORDERED_ACCESS) != RHI_BIND_FLAG::NONE)
        resFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    D3D12_RESOURCE_DESC texDesc = {};
    texDesc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Width              = _Size;
    texDesc.Height             = _Size;
    texDesc.DepthOrArraySize   = 6;                 // ← cubemap = 6 faces
    texDesc.MipLevels          = static_cast<UINT16>(_MipLevels);
    texDesc.Format             = _Format;
    texDesc.SampleDesc         = { 1, 0 };
    texDesc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags              = resFlags;

    D3D12_CLEAR_VALUE  clearValue  = {};
    D3D12_CLEAR_VALUE* pClearValue = nullptr;
    if ((_BindFlags & RHI_BIND_FLAG::RENDER_TARGET) != RHI_BIND_FLAG::NONE)
    {
        clearValue.Format   = _Format;
        clearValue.Color[0] = clearValue.Color[1] = clearValue.Color[2] = clearValue.Color[3] = 0.0f;
        pClearValue = &clearValue;
    }

    TextureUsage usage = TextureUsage::Albedo;
    if ((_BindFlags & RHI_BIND_FLAG::RENDER_TARGET) != RHI_BIND_FLAG::NONE)
        usage = TextureUsage::RenderTarget;

    auto device = DX12Device::GetInst()->GetD3D12Device();
    Microsoft::WRL::ComPtr<ID3D12Resource> resource;
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_COMMON,
        pClearValue,
        IID_PPV_ARGS(&resource)));

    ResourceStateTracker::AddGlobalResourceState(resource.Get(), D3D12_RESOURCE_STATE_COMMON);

    m_Texture = DX12Texture(usage);
    m_Texture.SetD3D12Resource(resource, pClearValue);
    m_Texture.CreateViews();

    return S_OK;
}

int DX12RHITexture::Load(const wstring& _FilePath)
{
    auto commandQueue = DX12Device::GetInst()->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
    auto commandList  = commandQueue->GetCommandList();

    commandList->LoadTextureFromFile(m_Texture, _FilePath, TextureUsage::Albedo);

    commandQueue->ExecuteCommandList(commandList);
    commandQueue->Flush();

    auto desc   = m_Texture.GetD3D12ResourceDesc();
    m_Width     = (UINT)desc.Width;
    m_Height    = desc.Height;
    m_Format    = desc.Format;
    m_BindFlags = RHI_BIND_FLAG::SHADER_RESOURCE;

    return S_OK;
}

int DX12RHITexture::CreateFromExisting(Microsoft::WRL::ComPtr<ID3D12Resource> resource, RHI_BIND_FLAG bindFlags)
{
    m_BindFlags = bindFlags;
    m_Texture   = DX12Texture(resource, TextureUsage::RenderTarget);
    m_Texture.CreateViews();

    auto desc = m_Texture.GetD3D12ResourceDesc();
    m_Width   = (UINT)desc.Width;
    m_Height  = desc.Height;
    m_Format  = desc.Format;

    return S_OK;
}