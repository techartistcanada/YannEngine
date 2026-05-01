#include "dx12pch.h"
#include "DX12StructuredBuffer.h" 

#include "DX12Device.h"
#include "DX12ResourceStateTracker.h"


StructuredBuffer::StructuredBuffer(const std::wstring& name)
	: Buffer(name)
	, m_CounterBuffer(CD3DX12_RESOURCE_DESC::Buffer(4, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), 1, 4, name + L" Counter")
	, m_NumElements(0)
	, m_ElementSize(0)
{
	m_SRV = DX12Device::GetInst()->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_UAV = DX12Device::GetInst()->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

StructuredBuffer::StructuredBuffer(const D3D12_RESOURCE_DESC& resDesc,
	size_t numElements,
	size_t elementSize,
	const std::wstring& name)
	: Buffer(resDesc, numElements, elementSize, name)
	, m_CounterBuffer(CD3DX12_RESOURCE_DESC::Buffer(4, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), 1, 4, name + L" Counter")
	, m_NumElements(numElements)
	, m_ElementSize(elementSize)
{
	m_SRV = DX12Device::GetInst()->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_UAV = DX12Device::GetInst()->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void StructuredBuffer::CreateViews(size_t numElements, size_t elementSize)
{
	auto device = DX12Device::GetInst()->GetD3D12Device();

	m_NumElements = numElements;
	m_ElementSize = elementSize;

	// * 同一块 GPU 内存，可以有不同的"想象方式"：
    // * 内存: [□□□□□□□□□□□□□□□□]
	// * 想象成Buffer(1D线性): [0][1][2][3][4]...    ← D3D12_SRV_DIMENSION_BUFFER
	// * 想象成 Texture1D:         |----width----|        ← D3D12_SRV_DIMENSION_TEXTURE1D  
	// * 想象成 Texture2D:         width × height         ← D3D12_SRV_DIMENSION_TEXTURE2D
	// * 想象成 Texture3D:         width × height × depth ← D3D12_SRV_DIMENSION_TEXTURE3D
	// * 想象成 TextureCube:       6个面的立方体          ← D3D12_SRV_DIMENSION_TEXTURECUBE
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.NumElements = static_cast<UINT>(m_NumElements);
	srvDesc.Buffer.StructureByteStride = static_cast<UINT>(m_ElementSize);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	device->CreateShaderResourceView(m_d3d12Resource.Get(),
		&srvDesc,
		m_SRV.GetDescriptorHandle());

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.Buffer.CounterOffsetInBytes = 0;
	uavDesc.Buffer.NumElements = static_cast<UINT>(m_NumElements);
	uavDesc.Buffer.StructureByteStride = static_cast<UINT>(m_ElementSize);
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	device->CreateUnorderedAccessView(
		m_d3d12Resource.Get(),
		m_CounterBuffer.GetD3D12Resource().Get(), // * counter计数器
		&uavDesc,
		m_UAV.GetDescriptorHandle());
}

