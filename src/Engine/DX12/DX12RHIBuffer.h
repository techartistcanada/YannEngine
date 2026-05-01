#pragma once
#include "../RHI/IRHIBuffer.h"
#include "DX12DescriptorAllocation.h"
#include "DX12Resource.h"

#include <d3d12.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;


class DX12RHIBuffer : public IRHIBuffer
{
private:
	ComPtr<ID3D12Resource> m_Resource;
	ComPtr<ID3D12Resource> m_UploadResource;
	ComPtr<ID3D12Resource> m_ReadbackResource;

	DescriptorAllocation   m_SRV;
	DescriptorAllocation   m_UAV;
	DescriptorAllocation   m_CBV;

	RHI_BUFFER_TYPE		   m_Type = RHI_BUFFER_TYPE::VERTEX_BUFFER;
	UINT 				   m_ByteSize = 0;
	UINT 				   m_ElementSize = 0;
	UINT 				   m_ElementCount = 0;
	DXGI_FORMAT 		   m_IndexFormat = DXGI_FORMAT_UNKNOWN;

	void*				   m_pMappedData = nullptr;

public:
    int CreateVertex(UINT _stride, UINT _count, const void* _pData = nullptr) override;
    int CreateIndex(DXGI_FORMAT _format, UINT _count, const void* _pData = nullptr) override;
    int CreateConstant(UINT _byteSize) override;
    int CreateStructured(UINT _elementSize, UINT _elementCount, bool _bCPUReadback, bool _bUAV, const void* _pData = nullptr) override;

    void SetData(const void* _pData, UINT _byteSize) override;
    void GetData(void* _pDest, UINT _byteSize) override;

    RHI_BUFFER_TYPE GetType()         const override { return m_Type;         }
    UINT            GetByteSize()     const override { return m_ByteSize;     }
    UINT            GetElementSize()  const override { return m_ElementSize;  }
    UINT            GetElementCount() const override { return m_ElementCount; }
    DXGI_FORMAT     GetIndexFormat()  const override { return m_IndexFormat;  }

    // DX12-specific accessors for command list binding
    ID3D12Resource*             GetD3D12Resource()      const { return m_Resource.Get();         }
    D3D12_GPU_VIRTUAL_ADDRESS   GetGPUVirtualAddress()  const { return m_Resource ? m_Resource->GetGPUVirtualAddress() : 0; }
    D3D12_VERTEX_BUFFER_VIEW    GetVertexBufferView()   const;
    D3D12_INDEX_BUFFER_VIEW     GetIndexBufferView()    const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetCBV()  const { return m_CBV.GetDescriptorHandle(); }
    D3D12_CPU_DESCRIPTOR_HANDLE GetSRV()  const { return m_SRV.GetDescriptorHandle(); }
    D3D12_CPU_DESCRIPTOR_HANDLE GetUAV()  const { return m_UAV.GetDescriptorHandle(); }
    void*                       GetMappedData() const { return m_pMappedData; }

public:
    DX12RHIBuffer()  = default;
    ~DX12RHIBuffer();
};