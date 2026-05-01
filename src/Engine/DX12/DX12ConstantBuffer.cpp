#include "dx12pch.h"
#include "DX12ConstantBuffer.h"
#include "DX12Device.h"

#include "d3dx12.h"

CDX12ConstantBuffer::CDX12ConstantBuffer(const std::wstring& name)
	: Buffer(name)
	, m_SizeInBytes(0)
{
	//m_ConstantBufferView = Application::Get().AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_ConstantBufferView = DX12Device::GetInst()->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

CDX12ConstantBuffer::~CDX12ConstantBuffer()
{
}

void CDX12ConstantBuffer::CreateViews(size_t numElements, size_t elementSize)
{
	m_SizeInBytes = numElements * elementSize;

	D3D12_CONSTANT_BUFFER_VIEW_DESC d3d12ConstantBufferViewDesc;
	d3d12ConstantBufferViewDesc.BufferLocation = m_d3d12Resource->GetGPUVirtualAddress();
	d3d12ConstantBufferViewDesc.SizeInBytes = static_cast<UINT>(Math::AlignUp(m_SizeInBytes, 16));

	//auto device = Application::Get().GetDevice();
	auto device = DX12Device::GetInst()->GetD3D12Device();
	device->CreateConstantBufferView(&d3d12ConstantBufferViewDesc, m_ConstantBufferView.GetDescriptorHandle());

}

D3D12_CPU_DESCRIPTOR_HANDLE CDX12ConstantBuffer::GetShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc) const
{
    throw std::exception("ConstantBuffer::GetShaderResourceView should not be called.");
}

D3D12_CPU_DESCRIPTOR_HANDLE CDX12ConstantBuffer::GetUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc) const
{
    throw std::exception("ConstantBuffer::GetUnorderedAccessView should not be called.");
}


