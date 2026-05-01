#pragma once
#include "../RHI/IRHICommandList.h"
#include "DX12CommandList.h"
#include <memory>

class DX12RHICommandList : public IRHICommandList
{
private:
    // The underlying DX12 command list (obtained each frame from CDX12Device)
    // This pointer is updated per-frame via SetCommandList()
    std::shared_ptr<DX12CommandList> m_CmdList;

public:
    void SetCommandList(std::shared_ptr<DX12CommandList> cmdList) { m_CmdList = cmdList; }
    std::shared_ptr<DX12CommandList> GetInternalCommandList() const { return m_CmdList; }

    // ── IRHICommandList ──────────────────────────────────────────────
    void SetRenderTargets(IRHITexture* const* ppRTVs, UINT numRTVs, IRHITexture* pDSV) override;
    void ClearRenderTarget(IRHITexture* pRTV, const FLOAT clearColor[4]) override;
    void ClearDepthStencil(IRHITexture* pDSV, FLOAT depthClearValue, UINT8 stencilClearValue) override;

	void SetViewport(float _topLeftX, float _topLeftY, float _width, float _height, float _minDepth = 0.0f, float _maxDepth = 1.0f) override;

    void SetVertexBuffer(IRHIBuffer* _pVB, UINT _stride, UINT _offset) override;
    void SetIndexBuffer(IRHIBuffer* _pIB, DXGI_FORMAT _format, UINT _offset) override;

    void SetConstantBuffer(UINT _slot, IRHIBuffer* _pCB) override;
    void SetConstantBuffer_CS(UINT _slot, IRHIBuffer* _pCB) override;

    void SetStructuredBufferSRV(UINT _slot, IRHIBuffer* _pBuffer) override;
    void SetStructuredBufferSRV_CS(UINT _slot, IRHIBuffer* _pBuffer) override;
    void SetStructuredBufferUAV_CS(UINT _slot, IRHIBuffer* _pBuffer) override;
    void ClearStructuredBufferSRV(UINT _slot) override;
    void ClearStructuredBufferSRV_CS(UINT _slot) override;
    void ClearStructuredBufferUAV_CS(UINT _slot) override;

    void SetTextureSRV(UINT _slot, IRHITexture* _pTexture) override;
    void SetTextureSRV_CS(UINT _slot, IRHITexture* _pTexture) override;
    void SetTextureUAV_CS(UINT _slot, IRHITexture* _pTexture) override;
	void SetTextureSRV_CS_Mip(UINT _slot, IRHITexture* _pTexture, UINT _mipLevel) override;
    void SetTextureUAV_CS_Mip(UINT _slot, IRHITexture* _pTexture, UINT _mipLevel) override;
    void ClearTextureSRV(UINT _slot) override;
    void ClearTextureSRV_CS(UINT _slot) override;
    void ClearTextureUAV_CS(UINT _slot) override;

    void DrawIndexed(UINT _indexCount, UINT _startIndex, INT _baseVertex, UINT _startInstance) override;
    void DrawIndexedInstanced(UINT _indexCountPerInstance, UINT _instanceCount, UINT _startIndex, INT _baseVertex, UINT _startInstance) override;

    void Dispatch(UINT _grpCountX, UINT _grpCountY, UINT _grpCountZ) override;

    void CopyTexture(IRHITexture* _pDst, IRHITexture* _pSrc) override;
    void CopyBuffer(IRHIBuffer* _pDst, IRHIBuffer* _pSrc) override;
	void CopySubresourceMip(IRHITexture* _pDst, UINT _dstMip, IRHITexture* _pSrc, UINT _srcMip, UINT _arraySize) override;

    void Flush() override;

public:
    DX12RHICommandList()  = default;
    ~DX12RHICommandList() = default;
};