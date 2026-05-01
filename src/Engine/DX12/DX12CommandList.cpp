#include "dx12pch.h"
#include "DX12CommandList.h"

#include "DX12Device.h"
#include "DX12ByteAddressBuffer.h"
//#include "DX12ConstantBuffer.h"
#include "DX12CommandQueue.h"
#include "DX12DynamicDescriptorHeap.h"
//#include "DX12GenerateMipsPSO.h"
#include "DX12IndexBuffer.h"
//#include "DX12PanoToCubemapPSO.h"
//#include "DX12RenderTarget.h"
#include "DX12Resource.h"
#include "DX12ResourceStateTracker.h"
#include "DX12RootSignature.h"
#include "DX12StructuredBuffer.h"
#include "DX12Texture.h"
#include "DX12UploadBuffer.h"
#include "DX12VertexBuffer.h"

#include "DX12Helpers.h"
#include <wrl.h>
#include <d3d12.h>

std::map<std::wstring, ID3D12Resource*> DX12CommandList::ms_TextureCache;
std::mutex DX12CommandList::ms_TextureCacheMutex;

// * DX11: 只有一个ImmediateContext,没有类型概念,所有操作共用一条隐式队列
// * DX12: 显式分离三种队列,让GPU的Copy Engine, Compute Engine, 3D Engine真正并行运行,DX11的单队列模型完全无法利用这一点
DX12CommandList::DX12CommandList(D3D12_COMMAND_LIST_TYPE type) :
	m_d3d12CommandListType(type),
	m_RootSignature(nullptr)
{
	// * DX11：ID3D11Device同样负责创建, 但DX11把Device(创建)和Context(执行)捆绑在一起,逻辑上不如DX12清晰
	auto device = DX12Device::GetInst()->GetD3D12Device();

	// * 创建CommandAllocator,它是GPU命令的底层内存池,所有录制进CommandList的GPU指令实际写入这块内存,类型必须与CommandList一致。
    // * DX11：没有对应概念,驱动内部管理命令内存,开发者不可见。
    // * DX12 :
	// * Allocator和GPU执行生命周期解耦: GPU还在执行时,CPU可以用另一个Allocator录制下一帧,实现CPU/GPU并行
	ThrowIfFailed(device->CreateCommandAllocator(type, IID_PPV_ARGS(&m_d3d12CommandAllocator)));
	ThrowIfFailed(device->CreateCommandList(0, m_d3d12CommandListType, m_d3d12CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_d3d12CommandList)));

	// * DX12:
	// * 创建 UploadBuffer,一个CPU可写的线性分配器,专门承载每帧临时数据
	// * SetGraphicsDynamicConstantBuffer/SetDynamicVertexBuffer等写入的数据都来自这里,每次Reset()清空重用。
	// * 开发者完全掌控Upload内存的分配策略和生命周期
	// * DX11: 驱动内部管理Upload内存,开发者看不到分配细节。
	// > m_Desc.ByteWidth = _bufferSize;
	// > m_Desc.MiscFlags = 0;
	// > m_Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	// > m_Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	// > m_Desc.Usage = D3D11_USAGE_DYNAMIC;
	// > if (FAILED(DEVICE->CreateBuffer(&m_Desc, nullptr, m_CB.GetAddressOf())))
	// > {
	// > return E_FAIL;
	// > }
	m_UploadBuffer = std::make_unique<UploadBuffer>();

	// * DX12:
	// * 创建ResourceStateTracker,跟踪此CommandList涉及的所有资源的Barrier状态(COMMON/RENDER_TARGET/COPY_DEST等)
	// * D3D12要求开发者显式插入ResourceBarrier,GPU才能正确处理缓存一致性和布局转换
	// * 驱动不再隐式插入Barrier,消除了大量隐式stall(这是DX11 CPU overhead高的主因之一)
	// * ResourceStateTracker是这个项目的辅助层:它在录制时延迟收集Barrier,等Close()时再统一解析并插入pendingCommandList
	// * 解决了跨CommandList的状态依赖问题

	// * DX11:驱动自动追踪,开发者完全不感知。
	m_ResourceStateTracker = std::make_unique<ResourceStateTracker>();

	// * 为每种DescriptorHeap类型(共4种:CBV_SRV_UAV/SAMPLER/RTV/DSV)创建一个DynamicDescriptorHeap
	// * 它的作用是:调用SetShaderResourceView时先Stage(暂存)Descriptor,,在真正Draw()/Dispatch()前的
	// * CommitStagedDescriptors时才写入GPU可见的Heap并绑定到RootSignature
	// * DX11：
	// * PSSetShaderResources直接绑定,驱动内部处理Descriptor管理,开发者不感知。
	// * DX12:
	// * Shader只能访问shader-visible descriptor heap
	// * DynamicDescriptorHeap动态从有限的GPU Heap中分配,用完自动扩容
	// * RootSignature需要连续的Descriptor Table.
	// * Stage -> Commit两阶段确保提交时Descriptor是连续排列的
	// * 多帧并发时Descriptor不能被CPU修改
	// * 每帧Reset()清空,重新分配,天然无竞争
	for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
	{
		m_DynamicDescriptorHeap[i] = std::make_unique<DynamicDescriptorHeap>(static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i));
		m_DescriptorHeaps[i] = nullptr;
	}
}

DX12CommandList::~DX12CommandList()
{
}

void DX12CommandList::TransitionBarrier(Microsoft::WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES stateAfter, UINT subresource, bool flushBarriers)
{
    if (resource)
    {
        // The "before" state is not important. It will be resolved by the resource state tracker.
        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), D3D12_RESOURCE_STATE_COMMON, stateAfter, subresource);
        m_ResourceStateTracker->ResourceBarrier(barrier);
    }

    if (flushBarriers)
    {
        FlushResourceBarriers();
    }
}

void DX12CommandList::TransitionBarrier(const Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subresource, bool flushBarriers)
{
	TransitionBarrier(resource.GetD3D12Resource(), stateAfter, subresource, flushBarriers);
}

// * 当一个资源已经处于D3D12_RESOURCE_STATE_UNORDERED_ACCESS状态时，TransitionBarrier无法使用(状态没有变化,转换无意义)
// * 但GPU的多个Dispatch调用之间仍然需要同步
void DX12CommandList::UAVBarrier(ComPtr<ID3D12Resource> resource, bool flushBarriers)
{
	auto barrier = CD3DX12_RESOURCE_BARRIER::UAV(resource.Get());
	
	m_ResourceStateTracker->ResourceBarrier(barrier);

	if (flushBarriers)
	{
		FlushResourceBarriers();
	}
}

void DX12CommandList::UAVBarrier(const Resource& resource, bool flushBarriers)
{
	UAVBarrier(resource.GetD3D12Resource(), flushBarriers);
}

// * Aliasing就是DX12允许你在不额外分配内存的前提下,用不同的格式/Flag重新"解释"同一块内存,代价是同时只能激活一个

void DX12CommandList::AliasingBarrier(ComPtr<ID3D12Resource> beforeResource, ComPtr<ID3D12Resource> afterResource, bool flushBarrier)
{
	auto barrier = CD3DX12_RESOURCE_BARRIER::Aliasing(beforeResource.Get(), afterResource.Get());

	m_ResourceStateTracker->ResourceBarrier(barrier);

	if(flushBarrier)
	{
		FlushResourceBarriers();
	}
}


void DX12CommandList::FlushResourceBarriers()
{
	// * 把积攒在ResourceStateTracker里的所有barrier一次性写入CommandList
	m_ResourceStateTracker->FlushResourceBarriers(*this);
}

void DX12CommandList::CopyResource(Microsoft::WRL::ComPtr<ID3D12Resource> dstRes, Microsoft::WRL::ComPtr<ID3D12Resource> srcRes)
{
	TransitionBarrier(dstRes, D3D12_RESOURCE_STATE_COPY_DEST);
	TransitionBarrier(srcRes, D3D12_RESOURCE_STATE_COPY_SOURCE);

	FlushResourceBarriers();

	m_d3d12CommandList->CopyResource(dstRes.Get(), srcRes.Get());

	TrackResource(dstRes);
	TrackResource(srcRes);
}


void DX12CommandList::CopyResource( Resource& dstRes, const Resource& srcRes )
{
    CopyResource(dstRes.GetD3D12Resource(), srcRes.GetD3D12Resource());
}

void DX12CommandList::ResolveSubresource(Resource& dstRes, const Resource& srcRes, uint32_t dstSubresource, uint32_t srcSubresource)
{
	TransitionBarrier(dstRes, D3D12_RESOURCE_STATE_RESOLVE_DEST, dstSubresource);
	TransitionBarrier(srcRes, D3D12_RESOURCE_STATE_RESOLVE_SOURCE, srcSubresource);

	FlushResourceBarriers();

	m_d3d12CommandList->ResolveSubresource(dstRes.GetD3D12Resource().Get(), dstSubresource,
		srcRes.GetD3D12Resource().Get(), srcSubresource,
		dstRes.GetD3D12ResourceDesc().Format);

	TrackResource(srcRes);
	TrackResource(dstRes);
}

void DX12CommandList::CopyBuffer(Buffer& buffer, size_t numElements, size_t elementSize, const void* bufferData, D3D12_RESOURCE_FLAGS flags)
{
	auto device = DX12Device::GetInst()->GetD3D12Device();

	size_t bufferSize = numElements * elementSize;
	ComPtr<ID3D12Resource> d3d12Resource;
	if (bufferSize == 0)
	{
		// 
	}
	else
	{
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags),
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&d3d12Resource)));

		ResourceStateTracker::AddGlobalResourceState(d3d12Resource.Get(), D3D12_RESOURCE_STATE_COMMON);

		if (bufferData != nullptr)
		{
			// * 为什么需要upload heap
			// * Default Heap在显存里,CPU没有访问通道(或者极慢的PCIe映射),
			// * DX12不像DX11那样帮你隐藏这个过程,你必须显式经过Upload Heap中转。
			ComPtr<ID3D12Resource> uploadResource;
			ThrowIfFailed(device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&uploadResource)
			));
			
			D3D12_SUBRESOURCE_DATA subresourceData = {};
			subresourceData.pData = bufferData;
			subresourceData.RowPitch = bufferSize;
			subresourceData.SlicePitch = subresourceData.RowPitch;

			m_ResourceStateTracker->TransitionResource(d3d12Resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST);
			FlushResourceBarriers();

			UpdateSubresources(m_d3d12CommandList.Get(),
				d3d12Resource.Get(),
				uploadResource.Get(),
				0, 0, 1,
				&subresourceData);

			TrackResource(uploadResource);
		}
		TrackResource(d3d12Resource);
	}

	buffer.SetD3D12Resource(d3d12Resource);
	buffer.CreateViews(numElements, elementSize);
}


void DX12CommandList::CopyVertexBuffer(DX12VertexBuffer& vertexBuffer, size_t numVertices, size_t vertexStride, const void* vertexBufferData)
{
	CopyBuffer(vertexBuffer, numVertices, vertexStride, vertexBufferData);
}

void DX12CommandList::CopyIndexBuffer(DX12IndexBuffer& indexBuffer, size_t numIndices, DXGI_FORMAT indexFormat, const void* indexBufferData)
{
	size_t indexSizeInBytes = indexFormat == DXGI_FORMAT_R16_UINT ? 2 : 4;
	CopyBuffer(indexBuffer, numIndices, indexSizeInBytes, indexBufferData);
}

void DX12CommandList::CopyByteAddressBuffer(ByteAddressBuffer& byteAddressBuffer, size_t bufferSize, const void* bufferData)
{
	CopyBuffer(byteAddressBuffer, 1, bufferSize, bufferData, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
}

void DX12CommandList::CopyStructuredBuffer( StructuredBuffer& structuredBuffer, size_t numElements, size_t elementSize, const void* bufferData )
{
    CopyBuffer(structuredBuffer, numElements, elementSize, bufferData, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS );
}

void DX12CommandList::SetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY primitiveTopology )
{
    m_d3d12CommandList->IASetPrimitiveTopology( primitiveTopology );
}

void DX12CommandList::LoadTextureFromFile(DX12Texture& texture, const std::wstring& fileName, TextureUsage textureUsage)
{
	fs::path filePath(fileName);
	if (!fs::exists(filePath))
	{
		throw std::exception("File not found");

	}

	std::lock_guard<std::mutex> lock(ms_TextureCacheMutex);
	auto iter = ms_TextureCache.find(fileName);
	if (iter != ms_TextureCache.end())
	{
		texture.SetTextureUsage(textureUsage);
		texture.SetD3D12Resource(iter->second);
		texture.CreateViews();
		texture.SetName(fileName);
	}
	else
	{

		// ================================
		// * 1. 加载图片数据到CPU Memory
		// ================================
		TexMetadata metadata;
		ScratchImage scratchImage; // * CPU端的图像像素数据容器
		
		if (filePath.extension() == ".dds")
		{
			ThrowIfFailed(LoadFromDDSFile(
				fileName.c_str(),
				DDS_FLAGS_NONE,
				&metadata,
				scratchImage));
		}
		else if (filePath.extension() == ".hdr")
		{
			ThrowIfFailed(LoadFromHDRFile(
				fileName.c_str(),
				&metadata,
				scratchImage));
		}
		   else if ( filePath.extension() == ".tga" )
        {
            ThrowIfFailed( LoadFromTGAFile( 
                fileName.c_str(), 
                &metadata, 
                scratchImage ) );
        }
        else
        {
            ThrowIfFailed( LoadFromWICFile( // * WIC = Windows Imaging Component
                fileName.c_str(), 
                WIC_FLAGS_NONE, 
                &metadata, 
                scratchImage ) );
        }

		// ================================
		// * 2. 创建对应的纹理资源描述符
		// ================================
		D3D12_RESOURCE_DESC textureDesc = {};
		switch (metadata.dimension)
		{
			case TEX_DIMENSION_TEXTURE1D:
				textureDesc = CD3DX12_RESOURCE_DESC::Tex1D(
					metadata.format,
					static_cast<UINT64>(metadata.width),
					static_cast<UINT64>(metadata.arraySize));
				break;
				case TEX_DIMENSION_TEXTURE2D:
                textureDesc = CD3DX12_RESOURCE_DESC::Tex2D( 
                    metadata.format, 
                    static_cast<UINT64>( metadata.width ), 
                    static_cast<UINT>( metadata.height ), 
                    static_cast<UINT16>( metadata.arraySize ),
					static_cast<UINT16>(metadata.mipLevels)
				);
                break;
            case TEX_DIMENSION_TEXTURE3D:
                textureDesc = CD3DX12_RESOURCE_DESC::Tex3D( 
                    metadata.format, 
                    static_cast<UINT64>( metadata.width ), 
                    static_cast<UINT>( metadata.height ), 
                    static_cast<UINT16>( metadata.depth ) );
				break;
            default:
				throw std::exception("Invalid texture dimension.");
				break;
		}

		// ================================
		// * 3. 在GPU创建空的纹理资源
		// ================================
		auto device = DX12Device::GetInst()->GetD3D12Device();
		ComPtr<ID3D12Resource> textureResource;

		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&textureDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&textureResource)));

		// ================================
		// 4. MISC
		// ================================
		texture.SetTextureUsage(textureUsage);
		texture.SetD3D12Resource(textureResource);
		texture.CreateViews();
		texture.SetName(fileName);
		ResourceStateTracker::AddGlobalResourceState(
			textureResource.Get(), D3D12_RESOURCE_STATE_COMMON
		);

		// ================================
		// * 5.上传图片数据到GPU
		// ================================
		std::vector<D3D12_SUBRESOURCE_DATA> subresources(scratchImage.GetImageCount());
		const Image* pImages = scratchImage.GetImages();
		for (int i = 0; i < scratchImage.GetImageCount(); ++i)
		{
			auto& subresource = subresources[i];
			subresource.RowPitch = pImages[i].rowPitch;
			subresource.SlicePitch = pImages[i].slicePitch;
			subresource.pData = pImages[i].pixels;
		}

		CopyTextureSubresource(
			texture,
			0,
			static_cast<uint32_t>(subresources.size()),
			subresources.data());

		// ================================
		// * 6. 创建Mipmaps
		// ================================
		if (subresources.size() < textureResource->GetDesc().MipLevels)
		{
			GenerateMips(texture);
		}

		ms_TextureCache[fileName] = textureResource.Get();

	}
}

void DX12CommandList::GenerateMips(DX12Texture& texture)
{
	/*
	// =====================================

	// 1.Copy Queue只能搬数据,不能跑Compute Shader
	// 所以如果当前是Copy Queue,就借一个Compute CommandList,把任务转发给它,然后直接return。
	// =====================================
	if (m_d3d12CommandListType == D3D12_COMMAND_LIST_TYPE_COPY)
	{
		if (!m_ComputeCommandList)
		{
			m_ComputeCommandList = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE)->GetCommandList();
		}

		m_ComputeCommandList->GenerateMips(texture);
		return;
	}

	// =====================================
	// 2. Checking
	// =====================================
	auto resource = texture.GetD3D12Resource();
	if (!resource) return;

	auto resourceDesc = resource->GetDesc();

	if (resourceDesc.MipLevels == 1) return;
	if (resourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D ||
		resourceDesc.DepthOrArraySize != 1 ||
		resourceDesc.SampleDesc.Count > 1)
	{
		throw std::exception("GenerateMips is only supported for non-multi-sampled 2D Textures.");
	}

	// =====================================
	// 3. Aliasing if needed
	// =====================================
	ComPtr<ID3D12Resource> uavResource = resource;
	ComPtr<ID3D12Resource> aliasResource = aliasResource;
	if (!texture.CheckUAVSupport() ||
		(resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) == 0)
	{
		// * 0.MISC
		auto device = CDX12Device::GetInst()->GetDevice();
		auto aliasDesc = resourceDesc;

		// 开启UAV
		aliasDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		aliasDesc.Flags &= ~(D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

		auto uavDesc = aliasDesc;
		// * 转换为支持UAV的格式
		uavDesc.Format = Texture::GetUAVCompatableFormat(resourceDesc.Format);

		D3D12_RESOURCE_DESC resourceDescs[] = {
			aliasDesc,
			uavDesc,
		};

		// * 1. 创建一个足够大的heap存放原始资源的拷贝
		auto allocationInfo = device->GetResourceAllocationInfo(0, _countof(resourceDescs), resourceDescs);

		D3D12_HEAP_DESC heapDesc = {};
		heapDesc.SizeInBytes = allocationInfo.SizeInBytes;
		heapDesc.Alignment = allocationInfo.Alignment;
		heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
		heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;

		ComPtr<ID3D12Heap> heap;
		ThrowIfFailed(device->CreateHeap(&heapDesc, IID_PPV_ARGS(&heap)));

		TrackResource(heap);
		ThrowIfFailed(device->CreatePlacedResource(
			heap.Get(), // * 在哪块显存?
			0,          // * 偏移量
			&aliasDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,    // * ClearValue
			IID_PPV_ARGS(&aliasResource)
		));

		ResourceStateTracker::AddGlobalResourceState(aliasResource.Get(), D3D12_RESOURCE_STATE_COMMON);
		TrackResource(aliasResource);


		ThrowIfFailed(device->CreatePlacedResource(
			heap.Get(),
			0,
			&uavDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&uavResource)
		));

		ResourceStateTracker::AddGlobalResourceState(uavResource.Get(), D3D12_RESOURCE_STATE_COMMON);
		TrackResource(uavResource);

		AliasingBarrier(nullptr, aliasResource);
		CopyResource(aliasResource, resource);

		AliasingBarrier(aliasResource, uavResource);
	}

	// =====================================
	// 4. Actually generating the mipmaps
	// =====================================
	// Generate mips with the UAV compatible resource.
    GenerateMips_UAV(Texture(uavResource, texture.GetTextureUsage()), resourceDesc.Format );

    if (aliasResource)
    {
        AliasingBarrier(uavResource, aliasResource);
        // Copy the alias resource back to the original resource.
        CopyResource(resource, aliasResource);
    }
	*/

}

void DX12CommandList::GenerateMips_UAV( DX12Texture& texture, DXGI_FORMAT format )
{
	/*
    if ( !m_GenerateMipsPSO )
    {
        m_GenerateMipsPSO = std::make_unique<GenerateMipsPSO>();
    }

    m_d3d12CommandList->SetPipelineState( m_GenerateMipsPSO->GetPipelineState().Get() );
    SetComputeRootSignature( m_GenerateMipsPSO->GetRootSignature() );

    GenerateMipsCB generateMipsCB;
    generateMipsCB.IsSRGB = Texture::IsSRGBFormat(format);

    auto resource = texture.GetD3D12Resource();
    auto resourceDesc = resource->GetDesc();

    // Create an SRV that uses the format of the original texture.
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;  // Only 2D textures are supported (this was checked in the calling function).
    srvDesc.Texture2D.MipLevels = resourceDesc.MipLevels;

    for ( uint32_t srcMip = 0; srcMip < resourceDesc.MipLevels - 1u; )
    {
        uint64_t srcWidth = resourceDesc.Width >> srcMip;
        uint32_t srcHeight = resourceDesc.Height >> srcMip;
        uint32_t dstWidth = static_cast<uint32_t>( srcWidth >> 1 );
        uint32_t dstHeight = srcHeight >> 1;

        // 0b00(0): Both width and height are even.
        // 0b01(1): Width is odd, height is even.
        // 0b10(2): Width is even, height is odd.
        // 0b11(3): Both width and height are odd.
        generateMipsCB.SrcDimension = ( srcHeight & 1 ) << 1 | ( srcWidth & 1 );

        // How many mipmap levels to compute this pass (max 4 mips per pass)
        DWORD mipCount;

        // The number of times we can half the size of the texture and get
        // exactly a 50% reduction in size.
        // A 1 bit in the width or height indicates an odd dimension.
        // The case where either the width or the height is exactly 1 is handled
        // as a special case (as the dimension does not require reduction).
        _BitScanForward( &mipCount, ( dstWidth == 1 ? dstHeight : dstWidth ) | 
                                    ( dstHeight == 1 ? dstWidth : dstHeight ) );
        // Maximum number of mips to generate is 4.
        mipCount = std::min<DWORD>( 4, mipCount + 1 );
        // Clamp to total number of mips left over.
        mipCount = ( srcMip + mipCount ) >= resourceDesc.MipLevels ? 
            resourceDesc.MipLevels - srcMip - 1 : mipCount;

        // Dimensions should not reduce to 0.
        // This can happen if the width and height are not the same.
        dstWidth = std::max<DWORD>( 1, dstWidth );
        dstHeight = std::max<DWORD>( 1, dstHeight );

        generateMipsCB.SrcMipLevel = srcMip;
        generateMipsCB.NumMipLevels = mipCount;
        generateMipsCB.TexelSize.x = 1.0f / (float)dstWidth;
        generateMipsCB.TexelSize.y = 1.0f / (float)dstHeight;

        SetCompute32BitConstants( GenerateMips::GenerateMipsCB, generateMipsCB );

        SetShaderResourceView( GenerateMips::SrcMip, 0, texture, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, srcMip, 1, &srvDesc );

        for ( uint32_t mip = 0; mip < mipCount; ++mip )
        {
            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format = resourceDesc.Format;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
            uavDesc.Texture2D.MipSlice = srcMip + mip + 1;

            SetUnorderedAccessView(GenerateMips::OutMip, mip, texture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, srcMip + mip + 1, 1, &uavDesc );
        }

        // Pad any unused mip levels with a default UAV. Doing this keeps the DX12 runtime happy.
        if ( mipCount < 4 )
        {
            m_DynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageDescriptors( GenerateMips::OutMip, mipCount, 4 - mipCount, m_GenerateMipsPSO->GetDefaultUAV() );
        }
        
        Dispatch( Math::DivideByMultiple(dstWidth, 8), Math::DivideByMultiple(dstHeight, 8) );

        UAVBarrier( texture );

        srcMip += mipCount;
    }
	*/
}

/*
void CommandList::PanoToCubemap(Texture& cubemapTexture, const Texture& panoTexture )
{
    if (m_d3d12CommandListType == D3D12_COMMAND_LIST_TYPE_COPY)
    {
        if (!m_ComputeCommandList)
        {
            m_ComputeCommandList = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE)->GetCommandList();
        }
        m_ComputeCommandList->PanoToCubemap(cubemapTexture, panoTexture);
        return;
    }

    if (!m_PanoToCubemapPSO)
    {
        m_PanoToCubemapPSO = std::make_unique<PanoToCubemapPSO>();
    }

    auto device = CDX12Device::GetInst()->GetDevice();

    auto cubemapResource = cubemapTexture.GetD3D12Resource();
    if (!cubemapResource) return;

    CD3DX12_RESOURCE_DESC cubemapDesc(cubemapResource->GetDesc());

    auto stagingResource = cubemapResource;
    Texture stagingTexture(stagingResource);
    // If the passed-in resource does not allow for UAV access
    // then create a staging resource that is used to generate
    // the cubemap.
    if ((cubemapDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) == 0)
    {
        auto stagingDesc = cubemapDesc;
        stagingDesc.Format = Texture::GetUAVCompatableFormat(cubemapDesc.Format);
        stagingDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &stagingDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&stagingResource)

        ));

        ResourceStateTracker::AddGlobalResourceState(stagingResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST);

        stagingTexture.SetD3D12Resource(stagingResource);
        stagingTexture.CreateViews();
        stagingTexture.SetName(L"Pano to Cubemap Staging Texture");

        CopyResource(stagingTexture, cubemapTexture );
    }

    TransitionBarrier(stagingTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    m_d3d12CommandList->SetPipelineState(m_PanoToCubemapPSO->GetPipelineState().Get());
    SetComputeRootSignature(m_PanoToCubemapPSO->GetRootSignature());

    PanoToCubemapCB panoToCubemapCB;

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = Texture::GetUAVCompatableFormat(cubemapDesc.Format);
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
    uavDesc.Texture2DArray.FirstArraySlice = 0;
    uavDesc.Texture2DArray.ArraySize = 6;

    for (uint32_t mipSlice = 0; mipSlice < cubemapDesc.MipLevels; )
    {
        // Maximum number of mips to generate per pass is 5.
        uint32_t numMips = std::min<uint32_t>(5, cubemapDesc.MipLevels - mipSlice);

        panoToCubemapCB.FirstMip = mipSlice;
        panoToCubemapCB.CubemapSize = std::max<uint32_t>( static_cast<uint32_t>( cubemapDesc.Width ), cubemapDesc.Height) >> mipSlice;
        panoToCubemapCB.NumMips = numMips;

        SetCompute32BitConstants(PanoToCubemapRS::PanoToCubemapCB, panoToCubemapCB);

        SetShaderResourceView(PanoToCubemapRS::SrcTexture, 0, panoTexture, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

        for ( uint32_t mip = 0; mip < numMips; ++mip )
        {
            uavDesc.Texture2DArray.MipSlice = mipSlice + mip;
            SetUnorderedAccessView(PanoToCubemapRS::DstMips, mip, stagingTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 0, 0, &uavDesc);
        }

        if (numMips < 5)
        {
            // Pad unused mips. This keeps DX12 runtime happy.
            m_DynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageDescriptors(PanoToCubemapRS::DstMips, panoToCubemapCB.NumMips, 5 - numMips, m_PanoToCubemapPSO->GetDefaultUAV());
        }

        Dispatch(Math::DivideByMultiple(panoToCubemapCB.CubemapSize, 16), Math::DivideByMultiple(panoToCubemapCB.CubemapSize, 16), 6 );

        mipSlice += numMips;
    }

    if (stagingResource != cubemapResource)
    {
        CopyResource(cubemapTexture, stagingTexture);
    }
}
*/

void DX12CommandList::ClearTexture(const DX12Texture& texture, const float clearColor[4])
{
	TransitionBarrier(texture, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_d3d12CommandList->ClearRenderTargetView(texture.GetRenderTargetView(), clearColor, 0, nullptr);

	TrackResource(texture);
}

void DX12CommandList::ClearDepthStencilTexture( const DX12Texture& texture, D3D12_CLEAR_FLAGS clearFlags, float depth, uint8_t stencil)
{
    TransitionBarrier(texture, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    m_d3d12CommandList->ClearDepthStencilView(texture.GetDepthStencilView(), clearFlags, depth, stencil, 0, nullptr);

    TrackResource(texture);
}

// * LoadTextureFromFile函数中调用
void DX12CommandList::CopyTextureSubresource(DX12Texture& texture, uint32_t fisrtSubresource, uint32_t numSubresources, D3D12_SUBRESOURCE_DATA* subresourceData)
{
	auto device = DX12Device::GetInst()->GetD3D12Device();
	auto destinationResource = texture.GetD3D12Resource();

	if (destinationResource)
	{
		// * 1.先转换资源状态
		//TransitionBarrier(texture, D3D12_RESOURCE_STATE_COPY_DEST);
		//FlushResourceBarriers();
		// ! Copy Queue 上资源从 COMMON 到 COPY_DEST 是隐式提升的
		// ! 显式发出 Legacy Barrier 会产生 LEGACY_COPY_DEST，
		// ! 与新版 Enhanced Barriers 校验冲突（期望 COMMON）
		if (m_d3d12CommandListType != D3D12_COMMAND_LIST_TYPE_COPY)
		{
			TransitionBarrier(texture, D3D12_RESOURCE_STATE_COPY_DEST);
			FlushResourceBarriers();
		}

		// * 2.计算upload buffer需要空间大小
		UINT64 requiredSize = GetRequiredIntermediateSize(destinationResource.Get(), fisrtSubresource, numSubresources);

		// * 3.创建upload heap/buffer
		ComPtr<ID3D12Resource> intermediateResource;
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(requiredSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&intermediateResource)
		));

		// * 4.上传 uplaod
		UpdateSubresources(
			m_d3d12CommandList.Get(),
			destinationResource.Get(),
			intermediateResource.Get(),
			0,
			fisrtSubresource,
			numSubresources,
			subresourceData);

		TrackResource(intermediateResource);
		TrackResource(destinationResource);
	}
}

void DX12CommandList::SetGraphicsDynamicConstantBuffer(uint32_t rootParameterIndex, size_t sizeInBytes, const void* bufferData)
{
	// * 1.在upload heap中分配内存(CPU+GPU)
	// * 常量缓冲区必须256字节对齐
	auto heapAllocation = m_UploadBuffer->Allocate(sizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

	// * 2.bufferData拷贝到GPU
	memcpy(heapAllocation.CPU, bufferData, sizeInBytes);

	// * 3.将GPU地址绑定到Root Signature的CBV槽位
	m_d3d12CommandList->SetGraphicsRootConstantBufferView(rootParameterIndex, heapAllocation.GPU);
}

void DX12CommandList::SetComputeDynamicConstantBuffer(uint32_t rootParameterIndex, size_t sizeInBytes, const void* bufferData)
{
	auto heapAllocation = m_UploadBuffer->Allocate(sizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	memcpy(heapAllocation.CPU, bufferData, sizeInBytes);
	m_d3d12CommandList->SetComputeRootConstantBufferView(rootParameterIndex, heapAllocation.GPU);
}

void DX12CommandList::SetGraphics32BitConstants(uint32_t rootParameterIndex, uint32_t numConstants, const void* constants)
{
	// * 无需heap, 内嵌在root signature中
	// ! 整个Root Signature大小为256字节(64 DWORDS)
	m_d3d12CommandList->SetGraphicsRoot32BitConstants(rootParameterIndex, numConstants, constants, 0);
}

void DX12CommandList::SetCompute32BitConstants(uint32_t rootParameterIndex, uint32_t numConstants, const void* constants)
{
	m_d3d12CommandList->SetComputeRoot32BitConstants(rootParameterIndex, numConstants, constants, 0);
}

void DX12CommandList::SetVertexBuffer(uint32_t slot, const DX12VertexBuffer& vertexBuffer)
{
	// * 1. 资源状态管理(dx11没有)
	TransitionBarrier(vertexBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	// * 2. DX11直接传的是ComPtr<ID3D11Buffer> 
	// * DX12:: 传的是View(GPU虚拟地址 + 大小 + stride)
	// > D3D12_VERTEX_BUFFER_VIEW view = {
    // > gpuVirtualAddress,  // GPU 虚拟地址，非指针
    // > sizeInBytes,
    // > strideInBytes};
	auto vertexBufferView = vertexBuffer.GetVertexBufferView();
	m_d3d12CommandList->IASetVertexBuffers(slot, 1, &vertexBufferView);

	// * 3. DX11 runtime内部有引用计数,绑定到管线的资源不会被释放
    // * DX12没有这个保护——命令录制完到GPU真正执行中间有时间差,资源可能被CPU提前释放
	// * TrackResource把ComPtr存入m_TrackedObjects,直到Reset()被调用(即确认GPU执行完毕后)才释放。
	TrackResource(vertexBuffer);
}

void DX12CommandList::SetDynamicVertexBuffer(uint32_t slot, size_t numVertices, size_t vertexSize, const void* vertexBufferData)
{
	size_t bufferSize = numVertices * vertexSize;

	auto heapAllocation = m_UploadBuffer->Allocate(bufferSize, vertexSize);
	memcpy(heapAllocation.CPU, vertexBufferData, bufferSize);

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
	vertexBufferView.BufferLocation = heapAllocation.GPU;
	vertexBufferView.SizeInBytes = static_cast<UINT>(bufferSize);
	vertexBufferView.StrideInBytes = static_cast<UINT>(vertexSize);

	m_d3d12CommandList->IASetVertexBuffers(slot, 1, &vertexBufferView);
}

void DX12CommandList::SetIndexBuffer(const DX12IndexBuffer& indexBuffer)
{
	TransitionBarrier(indexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER);

	auto indexBufferView = indexBuffer.GetIndexBufferView();

	m_d3d12CommandList->IASetIndexBuffer(&indexBufferView);

	TrackResource(indexBuffer);
}

void DX12CommandList::SetDynamicIndexBuffer(size_t numIndices, DXGI_FORMAT indexFormat, const void* indexBufferData)
{
	size_t indexSizeInBytes = indexFormat == DXGI_FORMAT_R16_UINT ? 2 : 4;
	size_t bufferSize = numIndices * indexSizeInBytes;

	auto heapAllocation = m_UploadBuffer->Allocate(bufferSize, indexSizeInBytes);
	memcpy(heapAllocation.CPU, indexBufferData, bufferSize);

	D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
	indexBufferView.BufferLocation = heapAllocation.GPU;
	indexBufferView.SizeInBytes = static_cast<UINT>(bufferSize);
	indexBufferView.Format = indexFormat;

	m_d3d12CommandList->IASetIndexBuffer(&indexBufferView);
	
}

void DX12CommandList::SetGraphicsDynamicStructuredBuffer(uint32_t slot, size_t numElements, size_t elementSize, const void* bufferData)
{
	size_t bufferSize = numElements * elementSize;

	auto heapAllocation = m_UploadBuffer->Allocate(bufferSize, elementSize);
	memcpy(heapAllocation.CPU, bufferData, bufferSize);

	m_d3d12CommandList->SetGraphicsRootShaderResourceView(slot, heapAllocation.GPU);
}

void DX12CommandList::SetViewport(const D3D12_VIEWPORT& viewport)
{
    SetViewports( {viewport} );
}


void DX12CommandList::SetViewports(const std::vector<D3D12_VIEWPORT>& viewports)
{
	assert(viewports.size() < D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);
	m_d3d12CommandList->RSSetViewports(static_cast<UINT>(viewports.size()), viewports.data());
}

void DX12CommandList::SetScissorRect(const D3D12_RECT& scissorRect)
{
    SetScissorRects({scissorRect});
}

void DX12CommandList::SetScissorRects(const std::vector<D3D12_RECT>& scissorRects)
{
    assert( scissorRects.size() < D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);
    m_d3d12CommandList->RSSetScissorRects( static_cast<UINT>( scissorRects.size() ), 
        scissorRects.data());
}


void DX12CommandList::SetPipelineState(ComPtr<ID3D12PipelineState> pipelineState)
{
	m_d3d12CommandList->SetPipelineState(pipelineState.Get());

	TrackResource(pipelineState);
}

// ! 根签名(interface/link)
// ! Root Signature
// !	-> Root Paramters
// !		-> Root Constant
// !		-> Root Descriptors
// !        -> Root Tables(Descriptor Tables)
// !    -> Sampler Descriptors

void DX12CommandList::SetGraphicsRootSignature(const RootSignature& rootSignature)
{
	auto d3d12RootSignature = rootSignature.GetRootSignature().Get();
	if (m_RootSignature != d3d12RootSignature)
	{
		m_RootSignature = d3d12RootSignature;

		for(int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
		{
			m_DynamicDescriptorHeap[i]->ParseRootSignature(rootSignature);
		}

		m_d3d12CommandList->SetGraphicsRootSignature(m_RootSignature);

		TrackResource(m_RootSignature);
	}
}

void DX12CommandList::SetComputeRootSignature(const RootSignature& rootSignature)
{
	auto d3d12RootSignature = rootSignature.GetRootSignature().Get();
	if (m_RootSignature != d3d12RootSignature)
	{
		m_RootSignature = d3d12RootSignature;

		for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
		{
			m_DynamicDescriptorHeap[i]->ParseRootSignature(rootSignature);
		}

		m_d3d12CommandList->SetComputeRootSignature(m_RootSignature);

		TrackResource(m_RootSignature);
	}
}

void DX12CommandList::SetShaderResourceView(uint32_t rootParameterIndex,
	uint32_t descriptorOffset,
	const Resource& resource,
	D3D12_RESOURCE_STATES stateAfter,
	UINT firstSubresource,
	UINT numSubresources,
	const D3D12_SHADER_RESOURCE_VIEW_DESC* srv)
{
	if (numSubresources < D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
	{
		for (uint32_t i = 0; i < numSubresources; ++i)
		{
			TransitionBarrier(resource, stateAfter, firstSubresource + i);
		}
	}
	else
	{
		TransitionBarrier(resource, stateAfter);
	}

	m_DynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageDescriptors(rootParameterIndex, descriptorOffset, 1, resource.GetShaderResourceView(srv));

	TrackResource(resource);
}

void DX12CommandList::SetUnorderedAccessView(uint32_t rootParameterIndex,
	uint32_t descriptorOffset,
	const Resource& resource,
	D3D12_RESOURCE_STATES stateAfter,
	UINT firstSubresource,
	UINT numSubresources,
	const D3D12_UNORDERED_ACCESS_VIEW_DESC* uav)
{
	if (numSubresources < D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
	{
		for (uint32_t i = 0; i < numSubresources; ++i)
		{
			TransitionBarrier(resource, stateAfter, firstSubresource + i);
		}
	}
	else
	{
		TransitionBarrier(resource, stateAfter);
	}

	m_DynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageDescriptors(rootParameterIndex, descriptorOffset, 1, resource.GetUnorderedAccessView(uav));

	TrackResource(resource);
}

void DX12CommandList::SetRenderTarget(const RenderTarget& renderTarget)
{
	// * Heap类型	                   Shader可索引				GPU使用方式
	// * CBV/SRV/UAV（Shader-Visible）   	✅	Shader		通过Descriptor Table读取
	// * Sampler（Shader-Visible）	        ✅	Shader            采样贴图
	// * RTV（Non-Shader-Visible）	        ❌	             OM固定阶段写入颜色
	// * DSV（Non-Shader-Visible）	        ❌	             OM固定阶段写入深度/模板

	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargetDescriptors;
	renderTargetDescriptors.reserve(AttachmentPoint::NumAttachmentPoints);

	const auto& textures = renderTarget.GetTextures();

	for (int i = 0; i < 8; ++i)
	{
		auto& texture = textures[i];

		if (texture.IsValid())
		{
			TransitionBarrier(texture, D3D12_RESOURCE_STATE_RENDER_TARGET);
			renderTargetDescriptors.push_back(texture.GetRenderTargetView());

			TrackResource(texture);
		}
	}

	const auto& depthTexture = renderTarget.GetTexture(AttachmentPoint::DepthStencil);

	CD3DX12_CPU_DESCRIPTOR_HANDLE depthStencilDescriptor(D3D12_DEFAULT);
	if (depthTexture.GetD3D12Resource())
	{
		TransitionBarrier(depthTexture, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		depthStencilDescriptor = depthTexture.GetDepthStencilView();

		TrackResource(depthTexture);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE* pDSV = depthStencilDescriptor.ptr != 0 ? &depthStencilDescriptor : nullptr;

	m_d3d12CommandList->OMSetRenderTargets(static_cast<UINT>(renderTargetDescriptors.size()),
		renderTargetDescriptors.data(), FALSE, pDSV);

}

void DX12CommandList::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t startVertex, uint32_t startInstance)
{
	FlushResourceBarriers();

	for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
	{
		m_DynamicDescriptorHeap[i]->CommitStagedDescriptorsForDraw(*this);
	}

	m_d3d12CommandList->DrawInstanced(vertexCount, instanceCount, startVertex, startInstance);

}




void DX12CommandList::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex, int32_t baseVertex, uint32_t startInstance)
{
	FlushResourceBarriers();
	for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
	{
		m_DynamicDescriptorHeap[i]->CommitStagedDescriptorsForDraw(*this);
	}

	m_d3d12CommandList->DrawIndexedInstanced(indexCount, instanceCount, startIndex, baseVertex, startInstance);
}

void DX12CommandList::Dispatch(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ)
{
	FlushResourceBarriers();
	for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
	{
		m_DynamicDescriptorHeap[i]->CommitStagedDescriptorsForDispatch(*this);
	}

	m_d3d12CommandList->Dispatch(numGroupsX, numGroupsY, numGroupsZ);
}

bool DX12CommandList::Close( DX12CommandList& pendingCommandList )
{
    // Flush any remaining barriers.
    FlushResourceBarriers();

    m_d3d12CommandList->Close();

    // Flush pending resource barriers.
    uint32_t numPendingBarriers = m_ResourceStateTracker->FlushPendingResourceBarriers( pendingCommandList );
    // Commit the final resource state to the global state.
    m_ResourceStateTracker->CommitFinalResourceStates();

    return numPendingBarriers > 0;
}


void DX12CommandList::Close()
{
	FlushResourceBarriers();
	m_d3d12CommandList->Close();
}

void DX12CommandList::Reset()
{
	ThrowIfFailed(m_d3d12CommandAllocator->Reset());
	ThrowIfFailed(m_d3d12CommandList->Reset(m_d3d12CommandAllocator.Get(), nullptr));

	m_ResourceStateTracker->Reset();
	m_UploadBuffer->Reset();

	ReleaseTrackedObjects();

	for ( int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i )
    {
        m_DynamicDescriptorHeap[i]->Reset();
        m_DescriptorHeaps[i] = nullptr;
    }

    m_RootSignature = nullptr;
    m_ComputeCommandList = nullptr;

}

void DX12CommandList::TrackResource(Microsoft::WRL::ComPtr<ID3D12Object> object)
{
    m_TrackedObjects.push_back(object);
}

void DX12CommandList::TrackResource(const Resource& res)
{
    TrackResource(res.GetD3D12Resource());
}

void DX12CommandList::ReleaseTrackedObjects()
{
    m_TrackedObjects.clear();
}



void DX12CommandList::SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, ID3D12DescriptorHeap* heap)
{
	if (m_DescriptorHeaps[heapType] != heap)
	{
		m_DescriptorHeaps[heapType] = heap;
		BindDescriptorHeaps();
	}
}

void DX12CommandList::BindDescriptorHeaps()
{
	UINT numDescriptorHeaps = 0;
	ID3D12DescriptorHeap* descriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = {};

	for (uint32_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
	{
		ID3D12DescriptorHeap* descriptorHeap = m_DescriptorHeaps[i];
		if (descriptorHeap)
		{
			descriptorHeaps[numDescriptorHeaps++] = descriptorHeap;
		}
	}

	m_d3d12CommandList->SetDescriptorHeaps(numDescriptorHeaps, descriptorHeaps);
}




