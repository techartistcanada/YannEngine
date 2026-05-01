// * 为什么需要一个ResourceStateTracker类
// *在DX12中,每个GPU资源(ID3D12Resource)(纹理,Buffer等)在任意时刻都处于某个特定状态,例如:
//  D3D12_RESOURCE_STATE_RENDER_TARGET	作为渲染目标写入
//  D3D12_RESOURCE_STATE_DEPTH_WRITE
//  D3D12_RESOURCE_STATE_DEPTH_READ
//  D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
//  D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
//  D3D12_RESOURCE_COPY_DEST
//  D3D12_RESOURCE_COPY_SOURCE
//  D3D12_RESOURCE_STATE_PRESENT
//  D3D12_RESOURCE_STATE_UNORDERED_ACCESS
//  D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
//  D3D12_RESOURCE_STATE_INDEX_BUFFER
// * 当你要切换用途时,必须提交一个ResourceBarrier,且必须正确填写StateBefore(当前状态)和StateAfter(目标状态)
// * 问题在哪?
// * 多个CommandList并行录制时,你根本不知道某个资源此刻在另一个线程里被用成什么状态了, StateBefore填什么?
// * 这就是ResourceStateTracker要解决的核心问题
#include "dx12pch.h"
#include "DX12ResourceStateTracker.h"

#include "DX12CommandList.h"
#include "DX12Resource.h"


// static definitions
std::mutex ResourceStateTracker::ms_GlobalMutex;
bool ResourceStateTracker::ms_IsLocked = false;
ResourceStateTracker::ResourceStateMap ResourceStateTracker::ms_GlobalResourceState;

ResourceStateTracker::ResourceStateTracker()
{
}

ResourceStateTracker::~ResourceStateTracker()
{
}

void ResourceStateTracker::ResourceBarrier(const D3D12_RESOURCE_BARRIER& barrier)
{
	if (barrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION)
	{
		// > struct D3D12_RESOURCE_BARRIER {
		// > D3D12_RESOURCE_BARRIER_TYPE  Type;
		// > D3D12_RESOURCE_BARRIER_FLAGS Flags;
		// > union {
		// >     D3D12_RESOURCE_TRANSITION_BARRIER Transition; // ← 这个
		// >     D3D12_RESOURCE_ALIASING_BARRIER   Aliasing;
		// >     D3D12_RESOURCE_UAV_BARRIER        UAV;
		// > };
		const D3D12_RESOURCE_TRANSITION_BARRIER& transitionBarrier = barrier.Transition;

		// * ========================================================================
		// * 这个resource被这个commandlist用过
		// * ========================================================================
		const auto iter = m_FinalResourceState.find(transitionBarrier.pResource);
		if (iter != m_FinalResourceState.end())
		{
			auto& resourceState = iter->second;
			// ! 分支1: SubresourceState map不为空(各子资源状态不统一)
			// > #define D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES 0xffffffff
			// > "我不指定某一个,我要一次性转换所有子资源"
			if (transitionBarrier.Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES &&
				!resourceState.SubresourceState.empty())
			{
				for (auto subresourceState : resourceState.SubresourceState)
				{
					if (transitionBarrier.StateAfter != subresourceState.second)
					{
						D3D12_RESOURCE_BARRIER newBarrier = barrier;
						newBarrier.Transition.Subresource = subresourceState.first;
						newBarrier.Transition.StateBefore = subresourceState.second;
						m_ResourceBarriers.push_back(newBarrier);
					}
				}
			}
			// ! 分支2: 
			else
			{
				auto finalState = resourceState.GetSubresourceState(transitionBarrier.Subresource);
				if (transitionBarrier.StateAfter != finalState)
				{
					D3D12_RESOURCE_BARRIER newBarrier = barrier;
					newBarrier.Transition.StateBefore = finalState;
					m_ResourceBarriers.push_back(newBarrier);
				}
			}
		}
		// * ========================================================================
		// * 这个commandlist第一次使用这个resource
		// ! 所以我们无法知道这个resource的stateBefore
		// * ========================================================================
		else
		{
			m_PendingResourceBarriers.push_back(barrier);
		}

		m_FinalResourceState[transitionBarrier.pResource].SetSubresourceState(transitionBarrier.Subresource, transitionBarrier.StateAfter);

	}
	// 非状态转换的barrier
	else
	{
		// Just push non-transition barriers to the resource barriers array.
		m_ResourceBarriers.push_back(barrier);
	}

}

void ResourceStateTracker::TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource)
{
	if (resource)
	{
		ResourceBarrier(CD3DX12_RESOURCE_BARRIER::Transition(resource, D3D12_RESOURCE_STATE_COMMON, stateAfter, subResource));
	}
}

void ResourceStateTracker::TransitionResource(const Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource)
{
	TransitionResource(resource.GetD3D12Resource().Get(), stateAfter, subResource);
}

void ResourceStateTracker::UAVBarrier(const Resource* resource)
{
	ID3D12Resource* pResource = resource != nullptr ? resource->GetD3D12Resource().Get() : nullptr;

	ResourceBarrier(CD3DX12_RESOURCE_BARRIER::UAV(pResource));
}

void ResourceStateTracker::AliasBarrier(const Resource* resourceBefore, const Resource* resourceAfter)
{
	ID3D12Resource* pResourceBefore = resourceBefore != nullptr ? resourceBefore->GetD3D12Resource().Get() : nullptr;
	ID3D12Resource* pResourceAfter = resourceAfter != nullptr ? resourceAfter->GetD3D12Resource().Get() : nullptr;

	ResourceBarrier(CD3DX12_RESOURCE_BARRIER::Aliasing(pResourceBefore, pResourceAfter));
}

void ResourceStateTracker::FlushResourceBarriers(DX12CommandList& commandList)
{
	UINT numBarriers = static_cast<UINT>(m_ResourceBarriers.size());
	if (numBarriers > 0)
	{
		auto d3d12CommandList = commandList.GetGraphicsCommandList();
		d3d12CommandList->ResourceBarrier(numBarriers, m_ResourceBarriers.data());
		m_ResourceBarriers.clear();
	}
}

uint32_t ResourceStateTracker::FlushPendingResourceBarriers(DX12CommandList& commandList)
{
	assert(ms_IsLocked);

	ResourceBarriers resourceBarriers;
	resourceBarriers.reserve(m_PendingResourceBarriers.size());

	for (auto pendingBarrier : m_PendingResourceBarriers)
	{
		if (pendingBarrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION)
		{
			auto pendingTransition = pendingBarrier.Transition;

			const auto& iter = ms_GlobalResourceState.find(pendingTransition.pResource);
			if (iter != ms_GlobalResourceState.end())
			{
				auto& resourceState = iter->second;
				// * ===================================
				// * Case 1: 各子资源状态不一致 
				// * ===================================
				if (pendingTransition.Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES &&
					!resourceState.SubresourceState.empty())
				{
					for (auto subresourceState : resourceState.SubresourceState)
					{
						if (pendingTransition.StateAfter != subresourceState.second)
						{
							D3D12_RESOURCE_BARRIER newBarrier = pendingBarrier;
							newBarrier.Transition.Subresource = subresourceState.first;
							newBarrier.Transition.StateBefore = subresourceState.second;
							resourceBarriers.push_back(newBarrier);
						}
					}
				}
				// * ===================================
				// * Case 2: 一致
				// * ===================================
				else
				{
					auto globalState = (iter->second).GetSubresourceState(pendingTransition.Subresource);
					if (pendingTransition.StateAfter != globalState)
					{
						pendingBarrier.Transition.StateBefore = globalState;
						resourceBarriers.push_back(pendingBarrier);
					}

				}
			}
		}
	}

	UINT numBarriers = static_cast<UINT>(resourceBarriers.size());
	if (numBarriers > 0)
	{
		auto d3d12CommandList = commandList.GetGraphicsCommandList();
		d3d12CommandList->ResourceBarrier(numBarriers, resourceBarriers.data());
	}

	m_PendingResourceBarriers.clear();

	return numBarriers;
}

void ResourceStateTracker::CommitFinalResourceStates()
{
	assert(ms_IsLocked);

	for (const auto& resourceState : m_FinalResourceState)
	{
		ms_GlobalResourceState[resourceState.first] = resourceState.second;
	}

	m_FinalResourceState.clear();
}

void ResourceStateTracker::Reset()
{
    // Reset the pending, current, and final resource states.
    m_PendingResourceBarriers.clear();
    m_ResourceBarriers.clear();
    m_FinalResourceState.clear();
}

void ResourceStateTracker::Lock()
{
    ms_GlobalMutex.lock();
    ms_IsLocked = true;
}

void ResourceStateTracker::Unlock()
{
    ms_GlobalMutex.unlock();
    ms_IsLocked = false;
}

void ResourceStateTracker::AddGlobalResourceState(ID3D12Resource* resource, D3D12_RESOURCE_STATES state)
{
    if ( resource != nullptr )
    {
        std::lock_guard<std::mutex> lock(ms_GlobalMutex);
        ms_GlobalResourceState[resource].SetSubresourceState(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, state);
    }
}

void ResourceStateTracker::RemoveGlobalResourceState(ID3D12Resource* resource)
{
    if ( resource != nullptr )
    {
        std::lock_guard<std::mutex> lock(ms_GlobalMutex);
        ms_GlobalResourceState.erase(resource);
    }
}


