#pragma once
#include "../RHI/IRHICommandList.h"

class DX11Texture;
class DX11Buffer;


class DX11CommandList : public IRHICommandList
{
private:
	ID3D11DeviceContext* m_pContext;

public:
	explicit DX11CommandList(ID3D11DeviceContext* _pContext);
	~DX11CommandList() = default;

	void SetRenderTargets(IRHITexture* const* _ppRTVs, UINT _numRTVs, IRHITexture* _pDSV) override;
	void ClearRenderTarget(IRHITexture* _pRTV, const FLOAT _clearColor[4]) override;
	void ClearDepthStencil(IRHITexture* _pDSV, FLOAT _depthClearValue, UINT8 _stencilClearValue) override;

	void SetViewport(float _topLeftX, float _topLeftY, float _width, float _height, float _minDepth, float _maxDepth) override;

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
};
