#include "../pch.h"
#ifdef USE_DX11


#include "DX11Buffer.h"
#include "DX11Device.h"


int DX11Buffer::CreateVertex(UINT _stride, UINT _count, const void* _pData)
{
	m_Type = RHI_BUFFER_TYPE::VERTEX_BUFFER;
	m_ElementSize = _stride;
	m_ElementCount = _count;
	m_ByteSize = m_ElementSize * m_ElementCount;

	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = m_ByteSize;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	if (_pData)
	{
		D3D11_SUBRESOURCE_DATA subData = {_pData};
		return FAILED(DEVICE->CreateBuffer(&desc, &subData, m_Buffer.GetAddressOf())) ? E_FAIL : S_OK;
	}
	return FAILED(DEVICE->CreateBuffer(&desc, nullptr, m_Buffer.GetAddressOf())) ? E_FAIL : S_OK;
}

int DX11Buffer::CreateIndex(DXGI_FORMAT format, UINT count, const void* pSysMem)
{
    m_Type = RHI_BUFFER_TYPE::INDEX_BUFFER;
    m_IndexFormat = format;
    m_ElementCount = count;
    m_ElementSize = (format == DXGI_FORMAT_R32_UINT) ? 4u : 2u;
    m_ByteSize = m_ElementSize * m_ElementCount;

    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = m_ByteSize;
    desc.Usage     = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    if (pSysMem)
    {
        D3D11_SUBRESOURCE_DATA sub = { pSysMem };
        return FAILED(DEVICE->CreateBuffer(&desc, &sub,
                      m_Buffer.GetAddressOf())) ? E_FAIL : S_OK;
    }
    return FAILED(DEVICE->CreateBuffer(&desc, nullptr,
                  m_Buffer.GetAddressOf())) ? E_FAIL : S_OK;
}

int DX11Buffer::CreateConstant(UINT byteSize)
{
    m_Type      = RHI_BUFFER_TYPE::CONSTANT_BUFFER;
    m_ByteSize  = byteSize;
    m_ElementSize  = byteSize;
    m_ElementCount = 1;

    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth      = byteSize;
    desc.Usage          = D3D11_USAGE_DYNAMIC;
    desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    return FAILED(DEVICE->CreateBuffer(&desc, nullptr,
                  m_Buffer.GetAddressOf())) ? E_FAIL : S_OK;
}

int DX11Buffer::CreateStructured(UINT _elemSize, UINT _elemCount,
                                 bool _bCPUReadback, bool _bUAV, const void* _pSysMem)
{
	m_Type         = RHI_BUFFER_TYPE::STRUCTURED_BUFFER;
	m_ElementSize  = _elemSize;
	m_ElementCount = _elemCount;
	m_ByteSize     = _elemSize * _elemCount;

	m_Buffer = m_WriteBuffer = m_ReadBuffer = nullptr;
	m_SRV    = nullptr;
	m_UAV    = nullptr;

	// ── 主缓冲区 (GPU only, DEFAULT) ──────────────────────────────────────
	D3D11_BUFFER_DESC mainDesc           = {};
	mainDesc.ByteWidth                   = m_ByteSize;
	mainDesc.Usage                       = D3D11_USAGE_DEFAULT;
	mainDesc.BindFlags                   = D3D11_BIND_SHADER_RESOURCE;
	if (_bUAV)
		mainDesc.BindFlags              |= D3D11_BIND_UNORDERED_ACCESS;
	mainDesc.MiscFlags                   = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	mainDesc.StructureByteStride         = _elemSize;

	if (_pSysMem)
	{
		D3D11_SUBRESOURCE_DATA sub = { _pSysMem };
		if (FAILED(DEVICE->CreateBuffer(&mainDesc, &sub, m_Buffer.GetAddressOf())))
			return E_FAIL;
	}
	else
	{
		if (FAILED(DEVICE->CreateBuffer(&mainDesc, nullptr, m_Buffer.GetAddressOf())))
			return E_FAIL;
	}

	// ── SRV ──────────────────────────────────────────────────────────────
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension                   = D3D_SRV_DIMENSION_BUFFER;
	srvDesc.BufferEx.NumElements            = _elemCount;
	if (FAILED(DEVICE->CreateShaderResourceView(m_Buffer.Get(), &srvDesc, m_SRV.GetAddressOf())))
		return E_FAIL;

	// ── UAV (SB_TYPE::SRV_UAV 时) ────────────────────────────────────────
	if (_bUAV)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.ViewDimension                    = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.NumElements               = _elemCount;
		if (FAILED(DEVICE->CreateUnorderedAccessView(m_Buffer.Get(), &uavDesc, m_UAV.GetAddressOf())))
			return E_FAIL;
	}

	// ── CPU 读写中间缓冲区 (bCPUReadback / SetData / GetData) ─────────────
	if (_bCPUReadback)
	{
		// Write buffer : CPU -> GPU
		D3D11_BUFFER_DESC writeDesc      = mainDesc;
		writeDesc.Usage                  = D3D11_USAGE_DYNAMIC;
		writeDesc.CPUAccessFlags         = D3D11_CPU_ACCESS_WRITE;
		writeDesc.BindFlags              = D3D11_BIND_SHADER_RESOURCE; // 不需要 UAV
		if (FAILED(DEVICE->CreateBuffer(&writeDesc, nullptr, m_WriteBuffer.GetAddressOf())))
			return E_FAIL;

		// Read buffer : GPU -> CPU
		D3D11_BUFFER_DESC readDesc       = {};
		readDesc.ByteWidth               = m_ByteSize;
		readDesc.Usage                   = D3D11_USAGE_STAGING;
		readDesc.CPUAccessFlags          = D3D11_CPU_ACCESS_READ;
		readDesc.MiscFlags               = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		readDesc.StructureByteStride     = _elemSize;
		if (FAILED(DEVICE->CreateBuffer(&readDesc, nullptr, m_ReadBuffer.GetAddressOf())))
			return E_FAIL;
	}

	return S_OK;
}

void DX11Buffer::SetData(const void* pData, UINT byteSize)
{
    if (byteSize == 0) byteSize = m_ByteSize;

    switch (m_Type)
    {
    case RHI_BUFFER_TYPE::CONSTANT_BUFFER:
    {
        D3D11_MAPPED_SUBRESOURCE mapped = {};
        CONTEXT->Map(m_Buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        memcpy(mapped.pData, pData, byteSize);
        CONTEXT->Unmap(m_Buffer.Get(), 0);
        break;
    }
    case RHI_BUFFER_TYPE::STRUCTURED_BUFFER:
    {
		assert(m_WriteBuffer && "SetData for STRUCTURED_BUFFER requires bCPUReadback = true at creation");
        D3D11_MAPPED_SUBRESOURCE mapped = {};
        CONTEXT->Map(m_WriteBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        memcpy(mapped.pData, pData, byteSize);
        CONTEXT->Unmap(m_WriteBuffer.Get(), 0);
        CONTEXT->CopyResource(m_Buffer.Get(), m_WriteBuffer.Get());
        break;
    }
    default:  // VERTEX / INDEX
        CONTEXT->UpdateSubresource(m_Buffer.Get(), 0, nullptr, pData, 0, 0);
        break;
    }
}

void DX11Buffer::GetData(void* pDest, UINT byteSize)
{
    assert(m_ReadBuffer && "GetData requires bCPUReadback = true at creation");
    if (byteSize == 0) byteSize = m_ByteSize;

    CONTEXT->CopyResource(m_ReadBuffer.Get(), m_Buffer.Get());

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    CONTEXT->Map(m_ReadBuffer.Get(), 0, D3D11_MAP_READ, 0, &mapped);
    memcpy(pDest, mapped.pData, byteSize);
    CONTEXT->Unmap(m_ReadBuffer.Get(), 0);
}

#endif // USE_DX11
