#include "dx12pch.h"
#include "DX12RHIBuffer.h"
#include "DX12Device.h"
#include "DX12CommandList.h"
#include "DX12CommandQueue.h"
#include "DX12ResourceStateTracker.h"

// D3D12 requires constant buffer size aligned to 256 bytes
static inline UINT Align256(UINT size) { return (size + 255) & ~255; }

// ─────────────────────────────────────────────────────────────────
DX12RHIBuffer::~DX12RHIBuffer()
{
    // CB: m_Resource is on upload heap, persistently mapped
    if (m_pMappedData && m_Resource)
    {
        m_Resource->Unmap(0, nullptr);
        m_pMappedData = nullptr;
    }
    // Remove from global resource state tracker
    if (m_Resource)
        ResourceStateTracker::RemoveGlobalResourceState(m_Resource.Get());
}

// ─────────────────────────────────────────────────────────────────
//  VERTEX BUFFER  (Default heap + Upload staging)
// ─────────────────────────────────────────────────────────────────
int DX12RHIBuffer::CreateVertex(UINT _stride, UINT _count, const void* _pData)
{
    m_Type         = RHI_BUFFER_TYPE::VERTEX_BUFFER;
    m_ElementSize  = _stride;
    m_ElementCount = _count;
    m_ByteSize     = _stride * _count;

    auto* d3dDevice = DX12Device::GetInst()->GetD3D12Device();

    // Default heap (GPU-only)
    {
        auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        auto resDesc   = CD3DX12_RESOURCE_DESC::Buffer(m_ByteSize);
        if (FAILED(d3dDevice->CreateCommittedResource(
            &heapProps, D3D12_HEAP_FLAG_NONE,
            &resDesc, D3D12_RESOURCE_STATE_COMMON, nullptr,
            IID_PPV_ARGS(&m_Resource))))
            return E_FAIL;
        ResourceStateTracker::AddGlobalResourceState(m_Resource.Get(), D3D12_RESOURCE_STATE_COMMON);
    }

    // Upload staging buffer (kept for future SetData calls)
    {
        auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto resDesc   = CD3DX12_RESOURCE_DESC::Buffer(m_ByteSize);
        if (FAILED(d3dDevice->CreateCommittedResource(
            &heapProps, D3D12_HEAP_FLAG_NONE,
            &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&m_UploadResource))))
            return E_FAIL;
    }

    // Upload initial data
    if (_pData)
    {
        void* mapped = nullptr;
        m_UploadResource->Map(0, nullptr, &mapped);
        memcpy(mapped, _pData, m_ByteSize);
        m_UploadResource->Unmap(0, nullptr);

        auto cmdQueue = DX12Device::GetInst()->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
        auto cmdList  = cmdQueue->GetCommandList();
        cmdList->TransitionBarrier(m_Resource, D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, true);
        cmdList->GetGraphicsCommandList()->CopyBufferRegion(
            m_Resource.Get(), 0, m_UploadResource.Get(), 0, m_ByteSize);
        cmdQueue->ExecuteCommandList(cmdList);
        cmdQueue->Flush();
    }

    return S_OK;
}

// ─────────────────────────────────────────────────────────────────
//  INDEX BUFFER  (Default heap + Upload staging)
// ─────────────────────────────────────────────────────────────────
int DX12RHIBuffer::CreateIndex(DXGI_FORMAT _format, UINT _count, const void* _pData)
{
    m_Type         = RHI_BUFFER_TYPE::INDEX_BUFFER;
    m_IndexFormat  = _format;
    m_ElementCount = _count;
    m_ElementSize  = (_format == DXGI_FORMAT_R32_UINT) ? 4u : 2u;
    m_ByteSize     = m_ElementSize * m_ElementCount;

    auto* d3dDevice = DX12Device::GetInst()->GetD3D12Device();

    // Default heap
    {
        auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        auto resDesc   = CD3DX12_RESOURCE_DESC::Buffer(m_ByteSize);
        if (FAILED(d3dDevice->CreateCommittedResource(
            &heapProps, D3D12_HEAP_FLAG_NONE,
            &resDesc, D3D12_RESOURCE_STATE_COMMON, nullptr,
            IID_PPV_ARGS(&m_Resource))))
            return E_FAIL;
        ResourceStateTracker::AddGlobalResourceState(m_Resource.Get(), D3D12_RESOURCE_STATE_COMMON);
    }

    // Upload staging
    {
        auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto resDesc   = CD3DX12_RESOURCE_DESC::Buffer(m_ByteSize);
        if (FAILED(d3dDevice->CreateCommittedResource(
            &heapProps, D3D12_HEAP_FLAG_NONE,
            &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&m_UploadResource))))
            return E_FAIL;
    }

    if (_pData)
    {
        void* mapped = nullptr;
        m_UploadResource->Map(0, nullptr, &mapped);
        memcpy(mapped, _pData, m_ByteSize);
        m_UploadResource->Unmap(0, nullptr);

        auto cmdQueue = DX12Device::GetInst()->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
        auto cmdList  = cmdQueue->GetCommandList();
        cmdList->TransitionBarrier(m_Resource, D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, true);
        cmdList->GetGraphicsCommandList()->CopyBufferRegion(
            m_Resource.Get(), 0, m_UploadResource.Get(), 0, m_ByteSize);
        cmdQueue->ExecuteCommandList(cmdList);
        cmdQueue->Flush();
    }

    return S_OK;
}

// ─────────────────────────────────────────────────────────────────
//  CONSTANT BUFFER  (Upload heap, 256-byte aligned, persistently mapped)
// ─────────────────────────────────────────────────────────────────
int DX12RHIBuffer::CreateConstant(UINT _byteSize)
{
    m_Type         = RHI_BUFFER_TYPE::CONSTANT_BUFFER;
    m_ByteSize     = Align256(_byteSize);
    m_ElementSize  = m_ByteSize;
    m_ElementCount = 1;

    auto* d3dDevice = DX12Device::GetInst()->GetD3D12Device();

    // Upload heap — CPU writes directly, GPU reads via root CBV
    auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto resDesc   = CD3DX12_RESOURCE_DESC::Buffer(m_ByteSize);
    if (FAILED(d3dDevice->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE,
        &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&m_Resource))))
        return E_FAIL;

    // Persistently map — stays mapped for the lifetime of the resource
    m_Resource->Map(0, nullptr, &m_pMappedData);

    return S_OK;
}

// ─────────────────────────────────────────────────────────────────
//  STRUCTURED BUFFER  (Default heap + SRV + optional UAV + optional readback)
// ─────────────────────────────────────────────────────────────────
int DX12RHIBuffer::CreateStructured(UINT _elementSize, UINT _elementCount,
                                    bool _bCPUReadback, bool _bUAV,
                                    const void* _pData)
{
    m_Type         = RHI_BUFFER_TYPE::STRUCTURED_BUFFER;
    m_ElementSize  = _elementSize;
    m_ElementCount = _elementCount;
    m_ByteSize     = _elementSize * _elementCount;

    auto* pDev      = DX12Device::GetInst();
    auto* d3dDevice = pDev->GetD3D12Device();

    // ── Main buffer (GPU only, DEFAULT heap) ─────────────────────
    {
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
        if (_bUAV) flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

        auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        auto resDesc   = CD3DX12_RESOURCE_DESC::Buffer(m_ByteSize, flags);
        if (FAILED(d3dDevice->CreateCommittedResource(
            &heapProps, D3D12_HEAP_FLAG_NONE,
            &resDesc, D3D12_RESOURCE_STATE_COMMON, nullptr,
            IID_PPV_ARGS(&m_Resource))))
            return E_FAIL;
        ResourceStateTracker::AddGlobalResourceState(m_Resource.Get(), D3D12_RESOURCE_STATE_COMMON);
    }

    // ── SRV ──────────────────────────────────────────────────────
    m_SRV = pDev->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension              = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Format                     = DXGI_FORMAT_UNKNOWN;
    srvDesc.Shader4ComponentMapping    = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Buffer.NumElements         = _elementCount;
    srvDesc.Buffer.StructureByteStride = _elementSize;
    srvDesc.Buffer.Flags               = D3D12_BUFFER_SRV_FLAG_NONE;
    d3dDevice->CreateShaderResourceView(m_Resource.Get(), &srvDesc,
        m_SRV.GetDescriptorHandle());

    // ── UAV (optional) ───────────────────────────────────────────
    if (_bUAV)
    {
        m_UAV = pDev->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.ViewDimension              = D3D12_UAV_DIMENSION_BUFFER;
        uavDesc.Format                     = DXGI_FORMAT_UNKNOWN;
        uavDesc.Buffer.NumElements         = _elementCount;
        uavDesc.Buffer.StructureByteStride = _elementSize;
        uavDesc.Buffer.Flags               = D3D12_BUFFER_UAV_FLAG_NONE;
        d3dDevice->CreateUnorderedAccessView(m_Resource.Get(), nullptr,
            &uavDesc, m_UAV.GetDescriptorHandle());
    }

    // ── CPU read/write staging buffers (bCPUReadback) ────────────
    if (_bCPUReadback)
    {
        // Upload buffer for SetData (CPU → GPU)
        {
            auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            auto resDesc   = CD3DX12_RESOURCE_DESC::Buffer(m_ByteSize);
            if (FAILED(d3dDevice->CreateCommittedResource(
                &heapProps, D3D12_HEAP_FLAG_NONE,
                &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                IID_PPV_ARGS(&m_UploadResource))))
                return E_FAIL;
        }

        // Readback buffer for GetData (GPU → CPU)
        {
            auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
            auto resDesc   = CD3DX12_RESOURCE_DESC::Buffer(m_ByteSize);
            if (FAILED(d3dDevice->CreateCommittedResource(
                &heapProps, D3D12_HEAP_FLAG_NONE,
                &resDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
                IID_PPV_ARGS(&m_ReadbackResource))))
                return E_FAIL;
        }
    }

    // ── Upload initial data ──────────────────────────────────────
    if (_pData)
    {
        // Use the readback upload buffer if available, otherwise create a temporary one
        ComPtr<ID3D12Resource> uploadBuf = m_UploadResource;
        if (!uploadBuf)
        {
            auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            auto resDesc   = CD3DX12_RESOURCE_DESC::Buffer(m_ByteSize);
            if (FAILED(d3dDevice->CreateCommittedResource(
                &heapProps, D3D12_HEAP_FLAG_NONE,
                &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                IID_PPV_ARGS(&uploadBuf))))
                return E_FAIL;
        }

        void* mapped = nullptr;
        uploadBuf->Map(0, nullptr, &mapped);
        memcpy(mapped, _pData, m_ByteSize);
        uploadBuf->Unmap(0, nullptr);

        auto cmdQueue = pDev->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
        auto cmdList  = cmdQueue->GetCommandList();
        cmdList->TransitionBarrier(m_Resource, D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, true);
        cmdList->GetGraphicsCommandList()->CopyBufferRegion(
            m_Resource.Get(), 0, uploadBuf.Get(), 0, m_ByteSize);
        cmdQueue->ExecuteCommandList(cmdList);
        cmdQueue->Flush();
    }

    return S_OK;
}

// ─────────────────────────────────────────────────────────────────
//  SetData
// ─────────────────────────────────────────────────────────────────
void DX12RHIBuffer::SetData(const void* _pData, UINT _byteSize)
{
    if (_byteSize == 0) _byteSize = m_ByteSize;

    switch (m_Type)
    {
    case RHI_BUFFER_TYPE::CONSTANT_BUFFER:
    {
        // Upload heap is persistently mapped — just memcpy
        assert(m_pMappedData && "CB was not persistently mapped");
        memcpy(m_pMappedData, _pData, _byteSize);
        break;
    }
    case RHI_BUFFER_TYPE::STRUCTURED_BUFFER:
    {
        assert(m_UploadResource && "SetData for STRUCTURED_BUFFER requires bCPUReadback = true");
        void* mapped = nullptr;
        m_UploadResource->Map(0, nullptr, &mapped);
        memcpy(mapped, _pData, _byteSize);
        m_UploadResource->Unmap(0, nullptr);

        auto cmdQueue = DX12Device::GetInst()->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
        auto cmdList  = cmdQueue->GetCommandList();
        cmdList->TransitionBarrier(m_Resource, D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, true);
        cmdList->GetGraphicsCommandList()->CopyBufferRegion(
            m_Resource.Get(), 0, m_UploadResource.Get(), 0, _byteSize);
        cmdQueue->ExecuteCommandList(cmdList);
        cmdQueue->Flush();
        break;
    }
    default: // VERTEX / INDEX
    {
        assert(m_UploadResource && "SetData requires upload staging buffer");
        void* mapped = nullptr;
        m_UploadResource->Map(0, nullptr, &mapped);
        memcpy(mapped, _pData, _byteSize);
        m_UploadResource->Unmap(0, nullptr);

        auto cmdQueue = DX12Device::GetInst()->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
        auto cmdList  = cmdQueue->GetCommandList();
        cmdList->TransitionBarrier(m_Resource, D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, true);
        cmdList->GetGraphicsCommandList()->CopyBufferRegion(
            m_Resource.Get(), 0, m_UploadResource.Get(), 0, _byteSize);
        cmdQueue->ExecuteCommandList(cmdList);
        cmdQueue->Flush();
        break;
    }
    }
}

// ─────────────────────────────────────────────────────────────────
//  GetData  (GPU → CPU, synchronous readback)
// ─────────────────────────────────────────────────────────────────
void DX12RHIBuffer::GetData(void* _pDest, UINT _byteSize)
{
    assert(m_ReadbackResource && "GetData requires bCPUReadback = true at creation");
    if (_byteSize == 0) _byteSize = m_ByteSize;

    auto cmdQueue = DX12Device::GetInst()->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    auto cmdList  = cmdQueue->GetCommandList();

    // Copy GPU resource → readback buffer
    cmdList->TransitionBarrier(m_Resource, D3D12_RESOURCE_STATE_COPY_SOURCE,
        D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, true);
    cmdList->GetGraphicsCommandList()->CopyBufferRegion(
        m_ReadbackResource.Get(), 0, m_Resource.Get(), 0, _byteSize);
    cmdQueue->ExecuteCommandList(cmdList);
    cmdQueue->Flush();

    // Map readback → CPU
    void* mapped = nullptr;
    D3D12_RANGE readRange = { 0, (SIZE_T)_byteSize };
    m_ReadbackResource->Map(0, &readRange, &mapped);
    memcpy(_pDest, mapped, _byteSize);
    D3D12_RANGE writeRange = { 0, 0 };
    m_ReadbackResource->Unmap(0, &writeRange);
}

// ─────────────────────────────────────────────────────────────────
//  View accessors (used by DX12RHICommandList for IA binding)
// ─────────────────────────────────────────────────────────────────
D3D12_VERTEX_BUFFER_VIEW DX12RHIBuffer::GetVertexBufferView() const
{
    D3D12_VERTEX_BUFFER_VIEW vbv = {};
    if (m_Resource)
    {
        vbv.BufferLocation = m_Resource->GetGPUVirtualAddress();
        vbv.SizeInBytes    = m_ByteSize;
        vbv.StrideInBytes  = m_ElementSize;
    }
    return vbv;
}

D3D12_INDEX_BUFFER_VIEW DX12RHIBuffer::GetIndexBufferView() const
{
    D3D12_INDEX_BUFFER_VIEW ibv = {};
    if (m_Resource)
    {
        ibv.BufferLocation = m_Resource->GetGPUVirtualAddress();
        ibv.SizeInBytes    = m_ByteSize;
        ibv.Format         = m_IndexFormat;
    }
    return ibv;
}