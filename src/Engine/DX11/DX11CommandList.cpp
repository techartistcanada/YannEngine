#include "../pch.h"
#ifdef USE_DX11


#include "DX11CommandList.h"
#include "DX11Texture.h"
#include "DX11Buffer.h"

DX11CommandList::DX11CommandList(ID3D11DeviceContext* _pContext)
	: m_pContext(_pContext)
{
}

void DX11CommandList::SetRenderTargets(IRHITexture* const* _ppRTVs, UINT _numRTVs, IRHITexture* _pDSV)
{
	ID3D11RenderTargetView* rtvs[8] = {};
	for (UINT i = 0; i < _numRTVs; ++i)
	{
		if(_ppRTVs[i])
			rtvs[i] = static_cast<DX11Texture*>(_ppRTVs[i])->GetRTV();
	}

	ID3D11DepthStencilView* dsv = _pDSV ? static_cast<DX11Texture*>(_pDSV)->GetDSV() : nullptr;

	m_pContext->OMSetRenderTargets(_numRTVs, rtvs, dsv);
}

void DX11CommandList::ClearRenderTarget(IRHITexture* _pRTV, const FLOAT _clearColor[4])
{
	if (_pRTV)
	{
		m_pContext->ClearRenderTargetView(static_cast<DX11Texture*>(_pRTV)->GetRTV(), _clearColor);
	}
}

void DX11CommandList::ClearDepthStencil(IRHITexture* _pDSV, FLOAT _depthClearValue, UINT8 _stencilClearValue)
{
	if (_pDSV)
	{
		m_pContext->ClearDepthStencilView(static_cast<DX11Texture*>(_pDSV)->GetDSV(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, _depthClearValue, _stencilClearValue);
	}
}

void DX11CommandList::SetViewport(float _topLeftX, float _topLeftY, float _width, float _height, float _minDepth, float _maxDepth)
{
	D3D11_VIEWPORT vp = {};
	vp.TopLeftX = _topLeftX;
	vp.TopLeftY = _topLeftY;
	vp.Width    = _width;
	vp.Height   = _height;
	vp.MinDepth = _minDepth;
	vp.MaxDepth = _maxDepth;
	m_pContext->RSSetViewports(1, &vp);
}


void DX11CommandList::SetVertexBuffer(IRHIBuffer* _pVB, UINT _stride, UINT _offset)
{
	ID3D11Buffer* vb = static_cast<DX11Buffer*>(_pVB)->GetBuffer();
	m_pContext->IASetVertexBuffers(0, 1, &vb, &_stride, &_offset);
}

void DX11CommandList::SetIndexBuffer(IRHIBuffer* _pIB, DXGI_FORMAT _format, UINT _offset)
{
	ID3D11Buffer* ib = static_cast<DX11Buffer*>(_pIB)->GetBuffer();
	m_pContext->IASetIndexBuffer(ib, _format, _offset);
}

void DX11CommandList::SetConstantBuffer(UINT slot, IRHIBuffer* pCB)
{
    ID3D11Buffer* buf = pCB ? static_cast<DX11Buffer*>(pCB)->GetBuffer() : nullptr;
    m_pContext->VSSetConstantBuffers(slot, 1, &buf);
    m_pContext->HSSetConstantBuffers(slot, 1, &buf);
    m_pContext->DSSetConstantBuffers(slot, 1, &buf);
    m_pContext->GSSetConstantBuffers(slot, 1, &buf);
    m_pContext->PSSetConstantBuffers(slot, 1, &buf);
}

void DX11CommandList::SetConstantBuffer_CS(UINT slot, IRHIBuffer* pCB)
{
    ID3D11Buffer* buf = pCB ? static_cast<DX11Buffer*>(pCB)->GetBuffer() : nullptr;
    m_pContext->CSSetConstantBuffers(slot, 1, &buf);
}

void DX11CommandList::SetStructuredBufferSRV(UINT _slot, IRHIBuffer* _pBuffer)
{
	ID3D11ShaderResourceView* srv = _pBuffer ? static_cast<DX11Buffer*>(_pBuffer)->GetSRV() : nullptr;
	m_pContext->VSSetShaderResources(_slot, 1, &srv);
	m_pContext->HSSetShaderResources(_slot, 1, &srv);
	m_pContext->DSSetShaderResources(_slot, 1, &srv);
	m_pContext->GSSetShaderResources(_slot, 1, &srv);
	m_pContext->PSSetShaderResources(_slot, 1, &srv);
}

void DX11CommandList::SetStructuredBufferSRV_CS(UINT _slot, IRHIBuffer * _pBuffer)
{
	ID3D11ShaderResourceView* srv = _pBuffer ? static_cast<DX11Buffer*>(_pBuffer)->GetSRV() : nullptr;
	m_pContext->CSSetShaderResources(_slot, 1, &srv);
}

void DX11CommandList::SetStructuredBufferUAV_CS(UINT _slot, IRHIBuffer * _pBuffer)
{
	ID3D11UnorderedAccessView* uav = _pBuffer ? static_cast<DX11Buffer*>(_pBuffer)->GetUAV() : nullptr;
	m_pContext->CSSetUnorderedAccessViews(_slot, 1, &uav, nullptr);
}

void DX11CommandList::ClearStructuredBufferSRV(UINT _slot)
{
	ID3D11ShaderResourceView* srv = nullptr;
	m_pContext->VSSetShaderResources(_slot, 1, &srv);
	m_pContext->HSSetShaderResources(_slot, 1, &srv);
	m_pContext->DSSetShaderResources(_slot, 1, &srv);
	m_pContext->GSSetShaderResources(_slot, 1, &srv);
	m_pContext->PSSetShaderResources(_slot, 1, &srv);
}

void DX11CommandList::ClearStructuredBufferSRV_CS(UINT _slot)
{
	ID3D11ShaderResourceView* srv = nullptr;
	m_pContext->CSSetShaderResources(_slot, 1, &srv);
}

void DX11CommandList::ClearStructuredBufferUAV_CS(UINT _slot)
{
	ID3D11UnorderedAccessView* uav = nullptr;
	m_pContext->CSSetUnorderedAccessViews(_slot, 1, &uav, nullptr);
}

void DX11CommandList::SetTextureSRV(UINT _slot, IRHITexture* _pTexture)
{
	ID3D11ShaderResourceView* srv = _pTexture ? static_cast<DX11Texture*>(_pTexture)->GetSRV() : nullptr;
	m_pContext->PSSetShaderResources(_slot, 1, &srv);
}

void DX11CommandList::SetTextureSRV_CS(UINT _slot, IRHITexture* _pTexture)
{
	ID3D11ShaderResourceView* srv = _pTexture ? static_cast<DX11Texture*>(_pTexture)->GetSRV() : nullptr;
	m_pContext->CSSetShaderResources(_slot, 1, &srv);
}

void DX11CommandList::SetTextureUAV_CS(UINT _slot, IRHITexture* _pTexture)
{
	ID3D11UnorderedAccessView* uav = _pTexture ? static_cast<DX11Texture*>(_pTexture)->GetUAV() : nullptr;
	m_pContext->CSSetUnorderedAccessViews(_slot, 1, &uav, nullptr);
}

void DX11CommandList::SetTextureSRV_CS_Mip(UINT _slot, IRHITexture* _pTexture, UINT _mipLevel)
{
    if (!_pTexture)
    {
        ID3D11ShaderResourceView* nullSRV = nullptr;
        m_pContext->CSSetShaderResources(_slot, 1, &nullSRV);
        return;
    }

    auto* dx11Tex = static_cast<DX11Texture*>(_pTexture);
    ID3D11Resource* resource = dx11Tex->GetResource();

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = dx11Tex->GetDXGIFormat();

    if (dx11Tex->IsCubemap())
    {
        // Cubemap: bind a single mip slice as Texture2DArray (matches HLSL Texture2DArray<float4>)
        srvDesc.ViewDimension                  = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
        srvDesc.Texture2DArray.MostDetailedMip = _mipLevel;
        srvDesc.Texture2DArray.MipLevels       = 1;
        srvDesc.Texture2DArray.FirstArraySlice = 0;
        srvDesc.Texture2DArray.ArraySize       = 6;
    }
    else
    {
        srvDesc.ViewDimension          = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = _mipLevel;
        srvDesc.Texture2D.MipLevels       = 1;
    }

    // TODO: cache per-mip SRVs to avoid creating a view every dispatch
    ID3D11Device* device = nullptr;
    resource->GetDevice(&device);

    ComPtr<ID3D11ShaderResourceView> tempSRV;
    HRESULT hr = device->CreateShaderResourceView(resource, &srvDesc, tempSRV.GetAddressOf());
    device->Release();
    if (FAILED(hr))
        return;

    ID3D11ShaderResourceView* srv = tempSRV.Get();
    m_pContext->CSSetShaderResources(_slot, 1, &srv);
}

void DX11CommandList::SetTextureUAV_CS_Mip(UINT _slot, IRHITexture* _pTexture, UINT _mipLevel)
{
	if(!_pTexture)
	{
		ID3D11UnorderedAccessView* uav = nullptr;
		m_pContext->CSSetUnorderedAccessViews(_slot, 1, &uav, nullptr);
		return;
	}

	auto* dx11Tex = static_cast<DX11Texture*>(_pTexture);
	ID3D11Resource* resource = dx11Tex->GetResource();

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = dx11Tex->GetDXGIFormat();

	if(dx11Tex->IsCubemap())
	{
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
		uavDesc.Texture2DArray.ArraySize = 6; // cubemap
		uavDesc.Texture2DArray.FirstArraySlice = 0;
		uavDesc.Texture2DArray.MipSlice = _mipLevel;
	}
	else
	{
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = _mipLevel;
	}

	// TODO:
	// FIXME: weird
	ID3D11Device* device = nullptr;
	resource->GetDevice(&device);

	// TODO:
	// FIXME: shouldn't create every time, cache
	ComPtr<ID3D11UnorderedAccessView> tempUAV;
    HRESULT hr = device->CreateUnorderedAccessView(resource, &uavDesc, tempUAV.GetAddressOf());
    device->Release();

    if (FAILED(hr))
        return;

    ID3D11UnorderedAccessView* uav = tempUAV.Get();
    m_pContext->CSSetUnorderedAccessViews(_slot, 1, &uav, nullptr);
    // Note: tempUAV released after dispatch since context holds ref
}

void DX11CommandList::ClearTextureSRV(UINT _slot)
{
	ID3D11ShaderResourceView* srv = nullptr;
	m_pContext->PSSetShaderResources(_slot, 1, &srv);
}

void DX11CommandList::ClearTextureSRV_CS(UINT _slot)
{
	ID3D11ShaderResourceView* srv = nullptr;
	m_pContext->CSSetShaderResources(_slot, 1, &srv);
}

void DX11CommandList::ClearTextureUAV_CS(UINT _slot)
{
	ID3D11UnorderedAccessView* uav = nullptr;
	m_pContext->CSSetUnorderedAccessViews(_slot, 1, &uav, nullptr);
}

// ---------------------------------------------------------------
// Draw
// ---------------------------------------------------------------
void DX11CommandList::DrawIndexed(UINT indexCount, UINT startIndex,
                                  INT baseVertex, UINT /*startInstance*/)
{
    // DX11 DrawIndexed 无 startInstance 参数，忽略（DX12 后端会用到）
    m_pContext->DrawIndexed(indexCount, startIndex, baseVertex);
}

void DX11CommandList::DrawIndexedInstanced(UINT indexCountPerInstance,
                                           UINT instanceCount,
                                           UINT startIndex, INT baseVertex,
                                           UINT startInstance)
{
    m_pContext->DrawIndexedInstanced(indexCountPerInstance, instanceCount,
                                     startIndex, baseVertex, startInstance);
}


// ---------------------------------------------------------------
// Resource / Misc
// ---------------------------------------------------------------
void DX11CommandList::CopyTexture(IRHITexture* pDst, IRHITexture* pSrc)
{
    m_pContext->CopyResource(
        static_cast<DX11Texture*>(pDst)->GetResource(),
        static_cast<DX11Texture*>(pSrc)->GetResource());
}


void DX11CommandList::Flush()
{
    m_pContext->Flush();
}

void DX11CommandList::Dispatch(UINT _grpCountX, UINT _grpCountY, UINT _grpCountZ)
{
	m_pContext->Dispatch(_grpCountX, _grpCountY, _grpCountZ);
}

void DX11CommandList::CopyBuffer(IRHIBuffer* _pDst, IRHIBuffer* _pSrc)
{
	m_pContext->CopyResource(
		static_cast<DX11Buffer*>(_pDst)->GetBuffer(),
		static_cast<DX11Buffer*>(_pSrc)->GetBuffer());
}

void DX11CommandList::CopySubresourceMip(IRHITexture* _pDst, UINT _dstMip, IRHITexture* _pSrc, UINT _srcMip, UINT _arraySize)
{
    auto* dst = static_cast<DX11Texture*>(_pDst);
    auto* src = static_cast<DX11Texture*>(_pSrc);
    for (UINT slice = 0; slice < _arraySize; ++slice)
    {
        UINT dstSub = D3D11CalcSubresource(_dstMip, slice, dst->GetMipLevels());
        UINT srcSub = D3D11CalcSubresource(_srcMip, slice, src->GetMipLevels());
        m_pContext->CopySubresourceRegion(
            dst->GetResource(), dstSub, 0, 0, 0,
            src->GetResource(), srcSub, nullptr);
    }
}

#endif // USE_DX11
