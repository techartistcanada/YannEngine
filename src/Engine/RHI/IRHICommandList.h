#pragma once
#include "RHIPrereqs.h"

class IRHIBuffer;
class IRHITexture;


class IRHICommandList
{
public:
	virtual ~IRHICommandList() = default;

	// Render Target
	virtual void SetRenderTargets(IRHITexture* const* ppRTVs, UINT numRTVs, IRHITexture* pDSV) = 0;
	virtual void ClearRenderTarget(IRHITexture* pRTV, const FLOAT clearColor[4]) = 0;
	virtual void ClearDepthStencil(IRHITexture* pDSV, FLOAT depthClearValue, UINT8 stencilClearValue) = 0;

	// viewport / scissor
	virtual void SetViewport(float topLeftX, float topLeftY, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f) = 0;

	// Input Assembler
	virtual void SetVertexBuffer(IRHIBuffer* _pVB, UINT _stride, UINT _offset) = 0;
	virtual void SetIndexBuffer(IRHIBuffer* _pIB, DXGI_FORMAT _format, UINT _offset) = 0;

	// Constant Buffers
	virtual void SetConstantBuffer(UINT _slot, IRHIBuffer* _pCB) = 0;
	virtual void SetConstantBuffer_CS(UINT _slot, IRHIBuffer* _pCB) = 0;

	// Structured Buffers SRV/UAV
	virtual void SetStructuredBufferSRV(UINT _slot, IRHIBuffer* _pBuffer) = 0;
	virtual void SetStructuredBufferSRV_CS(UINT _slot, IRHIBuffer* _pBuffer) = 0;
	virtual void SetStructuredBufferUAV_CS(UINT _slot, IRHIBuffer* _pBuffer) = 0;
	virtual void ClearStructuredBufferSRV(UINT _slot) = 0;
	virtual void ClearStructuredBufferSRV_CS(UINT _slot) = 0;
	virtual void ClearStructuredBufferUAV_CS(UINT _slot) = 0;

	// Textures
	virtual void SetTextureSRV(UINT _slot, IRHITexture* _pTexture) = 0;
	virtual void SetTextureSRV_CS(UINT _slot, IRHITexture* _pTexture) = 0;
	virtual void SetTextureUAV_CS(UINT _slot, IRHITexture* _pTexture) = 0;
	virtual void SetTextureSRV_CS_Mip(UINT _slot, IRHITexture* _pTexture, UINT _mipLevel) = 0;
	virtual void SetTextureUAV_CS_Mip(UINT _slot, IRHITexture* _pTexture, UINT _mipLevel) = 0;
	virtual void ClearTextureSRV(UINT _slot) = 0;
	virtual void ClearTextureSRV_CS(UINT _slot) = 0;
	virtual void ClearTextureUAV_CS(UINT _slot) = 0;

	// Draw/Render
	virtual void DrawIndexed(UINT _indexCount, UINT _startIndex = 0, INT _baseVertex = 0, UINT _startInstance = 0) = 0;
	virtual void DrawIndexedInstanced(UINT _indexCountPerInstance, UINT _instanceCount, UINT _startIndex = 0, INT _baseVertex = 0, UINT _startInstance = 0) = 0;

	// Dispatch
	virtual void Dispatch(UINT _grpCountX, UINT _grpCountY, UINT _grpCountZ) = 0;

	// Copy Resource
	virtual void CopyTexture(IRHITexture* _pDst, IRHITexture* _pSrc) = 0;
	virtual void CopyBuffer(IRHIBuffer* _pDst, IRHIBuffer* _pSrc) = 0;
	virtual void CopySubresourceMip(IRHITexture* _pDst, UINT _dstMip, IRHITexture* _pSrc, UINT _srcMip, UINT _arraySize) = 0;

	virtual void Flush() = 0;

};