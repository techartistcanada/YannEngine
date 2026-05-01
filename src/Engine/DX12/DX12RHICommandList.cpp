#include "dx12pch.h"
#include "DX12RHICommandList.h"
#include "DX12RHITexture.h"
#include "DX12RHIBuffer.h"
#include "DX12Device.h"
#include "DX12CommandQueue.h"
#include "DX12RenderTarget.h"
#include "DX12DynamicDescriptorHeap.h"

// ================================================================
// Root signature layout constants (must match CDX12Device creation):
//   [0]  Root CBV  b0  (TRANSFORM)     — ALL visibility
//   [1]  Root CBV  b1  (MATERIAL)      — ALL visibility
//   [2]  Root CBV  b2  (ANIMATION)     — ALL visibility
//   [3]  Root CBV  b3  (GLOBAL)        — ALL visibility
//   [4]  Descriptor Table: 16 SRVs t0-t15  — ALL visibility
//   [5]  Descriptor Table: 8 UAVs  u0-u7   — ALL visibility
// ================================================================
static constexpr UINT ROOT_CBV_START       = 0;  // root params 0..3
static constexpr UINT ROOT_SRV_TABLE       = 4;  // root param 4
static constexpr UINT ROOT_UAV_TABLE       = 5;  // root param 5

// ── Render Target ────────────────────────────────────────────────
void DX12RHICommandList::SetRenderTargets(IRHITexture* const* ppRTVs, UINT numRTVs, IRHITexture* pDSV)
{
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[8] = {};
    DXGI_FORMAT                 rtvFormats[8] = {};   // NEW
    for (UINT i = 0; i < numRTVs; ++i)
    {
        if (ppRTVs[i])
        {
            auto* tex = static_cast<DX12RHITexture*>(ppRTVs[i]);
            m_CmdList->TransitionBarrier(tex->GetDX12Texture(), D3D12_RESOURCE_STATE_RENDER_TARGET);
            rtvHandles[i] = tex->GetRTV();
            rtvFormats[i] = tex->GetDX12Texture().GetD3D12ResourceDesc().Format;   // NEW
        }
    }

    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle  = {};
    DXGI_FORMAT                 dsvFormat  = DXGI_FORMAT_UNKNOWN;   // NEW
    bool hasDSV = false;
    if (pDSV)
    {
        auto* dsTex = static_cast<DX12RHITexture*>(pDSV);
        m_CmdList->TransitionBarrier(dsTex->GetDX12Texture(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
        dsvHandle = dsTex->GetDSV();
        dsvFormat = dsTex->GetDX12Texture().GetD3D12ResourceDesc().Format;   // NEW
        hasDSV = true;
    }

    // Cache bound formats so PSO creation can auto-match them (D3D12 ERROR #613 fix)
    DX12Device::GetInst()->SetBoundRTFormats(rtvFormats, numRTVs, dsvFormat);

    m_CmdList->FlushResourceBarriers();
    m_CmdList->GetGraphicsCommandList()->OMSetRenderTargets(
        numRTVs, rtvHandles, FALSE, hasDSV ? &dsvHandle : nullptr);
}

void DX12RHICommandList::ClearRenderTarget(IRHITexture* pRTV, const FLOAT clearColor[4])
{
    if (!pRTV) return;
    auto* tex = static_cast<DX12RHITexture*>(pRTV);
    m_CmdList->TransitionBarrier(tex->GetDX12Texture(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, true);
    m_CmdList->GetGraphicsCommandList()->ClearRenderTargetView(tex->GetRTV(), clearColor, 0, nullptr);
}

void DX12RHICommandList::ClearDepthStencil(IRHITexture* pDSV, FLOAT depthClearValue, UINT8 stencilClearValue)
{
    if (!pDSV) return;
    auto* tex = static_cast<DX12RHITexture*>(pDSV);
    m_CmdList->TransitionBarrier(tex->GetDX12Texture(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, true);
    m_CmdList->GetGraphicsCommandList()->ClearDepthStencilView(
        tex->GetDSV(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
        depthClearValue, stencilClearValue, 0, nullptr);
}

void DX12RHICommandList::SetViewport(float _topLeftX, float _topLeftY, float _width, float _height, float _minDepth, float _maxDepth)
{
    D3D12_VIEWPORT vp = {};
    vp.TopLeftX = _topLeftX;
    vp.TopLeftY = _topLeftY;
    vp.Width    = _width;
    vp.Height   = _height;
    vp.MinDepth = _minDepth;
    vp.MaxDepth = _maxDepth;

    D3D12_RECT scissor = {};
    scissor.left   = static_cast<LONG>(_topLeftX);
    scissor.top    = static_cast<LONG>(_topLeftY);
    scissor.right  = static_cast<LONG>(_topLeftX + _width);
    scissor.bottom = static_cast<LONG>(_topLeftY + _height);

    auto d3dCmdList = m_CmdList->GetGraphicsCommandList();
    d3dCmdList->RSSetViewports(1, &vp);
    d3dCmdList->RSSetScissorRects(1, &scissor);
}

// ── Input Assembler ──────────────────────────────────────────────
void DX12RHICommandList::SetVertexBuffer(IRHIBuffer* _pVB, UINT _stride, UINT _offset)
{
    if (!_pVB) return;
    auto* buf = static_cast<DX12RHIBuffer*>(_pVB);
    auto vbv  = buf->GetVertexBufferView();
    m_CmdList->TransitionBarrier(buf->GetD3D12Resource(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    m_CmdList->FlushResourceBarriers();
    m_CmdList->GetGraphicsCommandList()->IASetVertexBuffers(0, 1, &vbv);
}

void DX12RHICommandList::SetIndexBuffer(IRHIBuffer* _pIB, DXGI_FORMAT _format, UINT _offset)
{
    if (!_pIB) return;
    auto* buf = static_cast<DX12RHIBuffer*>(_pIB);
    auto ibv  = buf->GetIndexBufferView();
    m_CmdList->TransitionBarrier(buf->GetD3D12Resource(), D3D12_RESOURCE_STATE_INDEX_BUFFER);
    m_CmdList->FlushResourceBarriers();
    m_CmdList->GetGraphicsCommandList()->IASetIndexBuffer(&ibv);
}

// ── Constant Buffers ─────────────────────────────────────────────
void DX12RHICommandList::SetConstantBuffer(UINT _slot, IRHIBuffer* _pCB)
{
    if (!_pCB) return;
    auto* buf = static_cast<DX12RHIBuffer*>(_pCB);
    // Allocate a fresh slot from the per-frame ring buffer so each draw call
    // gets its own unique GPU VA — fixes the shared persistent-buffer race condition
    // where all draws in a frame would read the last-written CB value.
    m_CmdList->SetGraphicsDynamicConstantBuffer(
        ROOT_CBV_START + _slot,
        buf->GetByteSize(),
        buf->GetMappedData());
}

void DX12RHICommandList::SetConstantBuffer_CS(UINT _slot, IRHIBuffer* _pCB)
{
	if (!_pCB) return;
    auto* buf = static_cast<DX12RHIBuffer*>(_pCB);
    m_CmdList->SetComputeDynamicConstantBuffer(
        ROOT_CBV_START + _slot,
        buf->GetByteSize(),
        buf->GetMappedData());
}

// ── Structured Buffers ───────────────────────────────────────────
void DX12RHICommandList::SetStructuredBufferSRV(UINT _slot, IRHIBuffer* _pBuffer)
{
    if (!_pBuffer) return;
    auto* buf = static_cast<DX12RHIBuffer*>(_pBuffer);
    m_CmdList->TransitionBarrier(buf->GetD3D12Resource(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    // Stage into SRV descriptor table at the given slot offset
    m_CmdList->GetDynamicDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
        ->StageDescriptors(ROOT_SRV_TABLE, _slot, 1, buf->GetSRV());
}

void DX12RHICommandList::SetStructuredBufferSRV_CS(UINT _slot, IRHIBuffer* _pBuffer)
{
    // Same staging — committed differently at dispatch time
    SetStructuredBufferSRV(_slot, _pBuffer);
}

void DX12RHICommandList::SetStructuredBufferUAV_CS(UINT _slot, IRHIBuffer* _pBuffer)
{
    if (!_pBuffer) return;
    auto* buf = static_cast<DX12RHIBuffer*>(_pBuffer);
    m_CmdList->TransitionBarrier(buf->GetD3D12Resource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    m_CmdList->GetDynamicDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
        ->StageDescriptors(ROOT_UAV_TABLE, _slot, 1, buf->GetUAV());
}

void DX12RHICommandList::ClearStructuredBufferSRV(UINT _slot)
{
    // ! NOTE: Use a valid null descriptor instead of {0}
    D3D12_CPU_DESCRIPTOR_HANDLE nullSRV = DX12Device::GetInst()->GetNullSRV();
    m_CmdList->GetDynamicDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
        ->StageDescriptors(ROOT_SRV_TABLE, _slot, 1, nullSRV);
}

void DX12RHICommandList::ClearStructuredBufferSRV_CS(UINT _slot)
{
    ClearStructuredBufferSRV(_slot);
}

void DX12RHICommandList::ClearStructuredBufferUAV_CS(UINT _slot)
{
    D3D12_CPU_DESCRIPTOR_HANDLE nullUAV = DX12Device::GetInst()->GetNullUAV();
    m_CmdList->GetDynamicDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
        ->StageDescriptors(ROOT_UAV_TABLE, _slot, 1, nullUAV);
}

// ── Texture SRV/UAV ──────────────────────────────────────────────
void DX12RHICommandList::SetTextureSRV(UINT _slot, IRHITexture* _pTexture)
{
    if (!_pTexture) return;
    auto* tex = static_cast<DX12RHITexture*>(_pTexture);
    m_CmdList->TransitionBarrier(tex->GetDX12Texture(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    m_CmdList->GetDynamicDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
        ->StageDescriptors(ROOT_SRV_TABLE, _slot, 1, tex->GetSRV());
}

void DX12RHICommandList::SetTextureSRV_CS(UINT _slot, IRHITexture* _pTexture)
{
    SetTextureSRV(_slot, _pTexture);
}

void DX12RHICommandList::SetTextureUAV_CS(UINT _slot, IRHITexture* _pTexture)
{
    if (!_pTexture) return;
    auto* tex = static_cast<DX12RHITexture*>(_pTexture);
    m_CmdList->TransitionBarrier(tex->GetDX12Texture(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    m_CmdList->GetDynamicDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
        ->StageDescriptors(ROOT_UAV_TABLE, _slot, 1, tex->GetUAV());
}

void DX12RHICommandList::SetTextureSRV_CS_Mip(UINT _slot, IRHITexture* _pTexture, UINT _mipLevel)
{
    if (!_pTexture) return;
    auto* tex = static_cast<DX12RHITexture*>(_pTexture);
    auto& dx12Tex = tex->GetDX12Texture();
    auto resDesc = dx12Tex.GetD3D12ResourceDesc();

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format                          = resDesc.Format;
    srvDesc.Shader4ComponentMapping         = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    // Cubemap stored as Texture2DArray with 6 slices
    if (resDesc.DepthOrArraySize >= 6 && resDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D
        && !tex->CanRenderTarget() && !tex->CanDepthStencil())
    {
        srvDesc.ViewDimension                         = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
        srvDesc.Texture2DArray.MostDetailedMip        = _mipLevel;
        srvDesc.Texture2DArray.MipLevels              = 1;
        srvDesc.Texture2DArray.FirstArraySlice        = 0;
        srvDesc.Texture2DArray.ArraySize              = resDesc.DepthOrArraySize;
    }
    else
    {
        srvDesc.ViewDimension             = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = _mipLevel;
        srvDesc.Texture2D.MipLevels       = 1;
    }

    m_CmdList->TransitionBarrier(dx12Tex,
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    m_CmdList->GetDynamicDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
        ->StageDescriptors(ROOT_SRV_TABLE, _slot, 1, dx12Tex.GetShaderResourceView(&srvDesc));
}

void DX12RHICommandList::SetTextureUAV_CS_Mip(UINT _slot, IRHITexture* _pTexture, UINT _mipLevel)
{
       if (!_pTexture) return;
    auto* tex = static_cast<DX12RHITexture*>(_pTexture);
    auto& dx12Tex = tex->GetDX12Texture();
    auto resDesc = dx12Tex.GetD3D12ResourceDesc();

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = resDesc.Format;

    // Cubemap stored as Texture2DArray with 6 slices
    if (resDesc.DepthOrArraySize >= 6 && resDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D
        && !tex->CanRenderTarget() && !tex->CanDepthStencil())
    {
        uavDesc.ViewDimension                  = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
        uavDesc.Texture2DArray.MipSlice        = _mipLevel;
        uavDesc.Texture2DArray.FirstArraySlice = 0;
        uavDesc.Texture2DArray.ArraySize       = resDesc.DepthOrArraySize;
    }
    else
    {
        uavDesc.ViewDimension      = D3D12_UAV_DIMENSION_TEXTURE2D;
        uavDesc.Texture2D.MipSlice = _mipLevel;
    }

    m_CmdList->TransitionBarrier(dx12Tex, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    m_CmdList->GetDynamicDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
        ->StageDescriptors(ROOT_UAV_TABLE, _slot, 1, dx12Tex.GetUnorderedAccessView(&uavDesc));
}

void DX12RHICommandList::ClearTextureSRV(UINT _slot)
{
    D3D12_CPU_DESCRIPTOR_HANDLE nullSRV = DX12Device::GetInst()->GetNullSRV();
    m_CmdList->GetDynamicDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
        ->StageDescriptors(ROOT_SRV_TABLE, _slot, 1, nullSRV);
}

void DX12RHICommandList::ClearTextureSRV_CS(UINT _slot)
{
    ClearTextureSRV(_slot);
}

void DX12RHICommandList::ClearTextureUAV_CS(UINT _slot)
{
    D3D12_CPU_DESCRIPTOR_HANDLE nullUAV = DX12Device::GetInst()->GetNullUAV();
    m_CmdList->GetDynamicDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
        ->StageDescriptors(ROOT_UAV_TABLE, _slot, 1, nullUAV);
}

// ── Draw / Dispatch ──────────────────────────────────────────────
void DX12RHICommandList::DrawIndexed(UINT _indexCount, UINT _startIndex,
                                     INT _baseVertex, UINT _startInstance)
{
    m_CmdList->FlushResourceBarriers();
    m_CmdList->GetDynamicDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
        ->CommitStagedDescriptorsForDraw(*m_CmdList);
    m_CmdList->GetGraphicsCommandList()->DrawIndexedInstanced(
        _indexCount, 1, _startIndex, _baseVertex, _startInstance);
}

void DX12RHICommandList::DrawIndexedInstanced(UINT _indexCountPerInstance,
                                              UINT _instanceCount,
                                              UINT _startIndex, INT _baseVertex,
                                              UINT _startInstance)
{
    m_CmdList->FlushResourceBarriers();
    m_CmdList->GetDynamicDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
        ->CommitStagedDescriptorsForDraw(*m_CmdList);
    m_CmdList->GetGraphicsCommandList()->DrawIndexedInstanced(
        _indexCountPerInstance, _instanceCount,
        _startIndex, _baseVertex, _startInstance);
}

void DX12RHICommandList::Dispatch(UINT _grpCountX, UINT _grpCountY, UINT _grpCountZ)
{
    m_CmdList->FlushResourceBarriers();
    m_CmdList->GetDynamicDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
        ->CommitStagedDescriptorsForDispatch(*m_CmdList);
    m_CmdList->GetGraphicsCommandList()->Dispatch(_grpCountX, _grpCountY, _grpCountZ);
}

// ── Copy ─────────────────────────────────────────────────────────
void DX12RHICommandList::CopyTexture(IRHITexture* _pDst, IRHITexture* _pSrc)
{
    auto& dst = static_cast<DX12RHITexture*>(_pDst)->GetDX12Texture();
    auto& src = static_cast<DX12RHITexture*>(_pSrc)->GetDX12Texture();
    m_CmdList->CopyResource(dst, src);
}

void DX12RHICommandList::CopyBuffer(IRHIBuffer* _pDst, IRHIBuffer* _pSrc)
{
    auto* dst = static_cast<DX12RHIBuffer*>(_pDst);
    auto* src = static_cast<DX12RHIBuffer*>(_pSrc);

    m_CmdList->TransitionBarrier(dst->GetD3D12Resource(), D3D12_RESOURCE_STATE_COPY_DEST);
    m_CmdList->TransitionBarrier(src->GetD3D12Resource(), D3D12_RESOURCE_STATE_COPY_SOURCE);
    m_CmdList->FlushResourceBarriers();
    m_CmdList->GetGraphicsCommandList()->CopyResource(dst->GetD3D12Resource(), src->GetD3D12Resource());
}

void DX12RHICommandList::CopySubresourceMip(IRHITexture* _pDst, UINT _dstMip,
                                             IRHITexture* _pSrc, UINT _srcMip,
                                             UINT _arraySize)
{
    auto& texDst = static_cast<DX12RHITexture*>(_pDst)->GetDX12Texture();
    auto& texSrc = static_cast<DX12RHITexture*>(_pSrc)->GetDX12Texture();

    UINT dstMipLevels = static_cast<UINT>(texDst.GetD3D12ResourceDesc().MipLevels);
    UINT srcMipLevels = static_cast<UINT>(texSrc.GetD3D12ResourceDesc().MipLevels);

    m_CmdList->TransitionBarrier(texDst, D3D12_RESOURCE_STATE_COPY_DEST,   D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, false);
    m_CmdList->TransitionBarrier(texSrc, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, true);

    auto pD3DCmdList = m_CmdList->GetGraphicsCommandList();

    for (UINT slice = 0; slice < _arraySize; ++slice)
    {
        D3D12_TEXTURE_COPY_LOCATION dst = {};
        dst.pResource        = texDst.GetD3D12Resource().Get();
        dst.Type             = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dst.SubresourceIndex = _dstMip + slice * dstMipLevels;

        D3D12_TEXTURE_COPY_LOCATION src = {};
        src.pResource        = texSrc.GetD3D12Resource().Get();
        src.Type             = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        src.SubresourceIndex = _srcMip + slice * srcMipLevels;

        pD3DCmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
    }
}

void DX12RHICommandList::Flush()
{
	auto commandQueue = DX12Device::GetInst()->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);

    if (m_CmdList)
    {
        commandQueue->ExecuteCommandList(m_CmdList);
        m_CmdList = nullptr;
    }

    commandQueue->Flush();
}