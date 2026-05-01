#include "dx12pch.h"
#include "DX12CommandQueue.h"
#include "DX12Device.h"
#include "DX12CommandList.h"
#include "DX12ResourceStateTracker.h"

DX12CommandQueue::DX12CommandQueue(D3D12_COMMAND_LIST_TYPE type)
	: m_FenceValue(0)
	, m_CommandListType(type)
	, m_bProcessInFlightCommandLists(true)
{
	//auto device = Application::Get().GetDevice();
	// NOTE: DX12
	auto device = DX12Device::GetInst()->GetD3D12Device();

	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = type;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;

	ThrowIfFailed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_d3d12CommandQueue)));
	ThrowIfFailed(device->CreateFence(m_FenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_d3d12Fence)));

	switch (type)
	{
		case D3D12_COMMAND_LIST_TYPE_COPY:
			m_d3d12CommandQueue->SetName(L"Copy Command Queue");
			break;
		case D3D12_COMMAND_LIST_TYPE_COMPUTE:
			m_d3d12CommandQueue->SetName(L"Compute Command Queue");
			break;
		case D3D12_COMMAND_LIST_TYPE_DIRECT:
			m_d3d12CommandQueue->SetName(L"Direct Command Queue");
			break;
	}

	m_ProcessInFlightCommandListsThread = std::thread(&DX12CommandQueue::ProccessInFlightCommandLists, this);

}


DX12CommandQueue::~DX12CommandQueue()
{
    m_bProcessInFlightCommandLists = false;
    m_ProcessInFlightCommandListsThread.join();
}


uint64_t DX12CommandQueue::Signal()
{
	uint64_t fenceValue = ++m_FenceValue;
	m_d3d12CommandQueue->Signal(m_d3d12Fence.Get(), fenceValue);
	return fenceValue;
}

bool DX12CommandQueue::IsFenceComplete(uint64_t fenceValue)
{
	return m_d3d12Fence->GetCompletedValue() >= fenceValue;
}

void DX12CommandQueue::WaitForFenceValue(uint64_t fenceValue)
{
    if (!IsFenceComplete(fenceValue))
    {
        auto event = ::CreateEvent( NULL, FALSE, FALSE, NULL );
        assert( event && "Failed to create fence event handle." );

        // Is this function thread safe?
        m_d3d12Fence->SetEventOnCompletion(fenceValue, event );
        ::WaitForSingleObject( event, DWORD_MAX);

        ::CloseHandle( event );
    }
}


void DX12CommandQueue::Flush()
{
    std::unique_lock<std::mutex> lock(m_ProcessInFlightCommandListsThreadMutex);
    m_ProcessInFlightCommandListsThreadCV.wait(lock, [this] { return m_InFlightCommandLists.Empty(); });

    // In case the command queue was signaled directly 
    // using the CommandQueue::Signal method then the 
    // fence value of the command queue might be higher than the fence
    // value of any of the executed command lists.
    WaitForFenceValue( m_FenceValue );
}


// Execute a command list.
// Returns the fence value to wait for for this command list.
uint64_t DX12CommandQueue::ExecuteCommandList(std::shared_ptr<DX12CommandList> commandList)
{
    return ExecuteCommandLists( std::vector<std::shared_ptr<DX12CommandList> >( { commandList } ) );
}


uint64_t DX12CommandQueue::ExecuteCommandLists(const std::vector<std::shared_ptr<DX12CommandList>>& commandLists)
{
	ResourceStateTracker::Lock();

	std::vector<std::shared_ptr<DX12CommandList>> toBeQueued;
	toBeQueued.reserve(commandLists.size() * 2);

	std::vector<std::shared_ptr<DX12CommandList>> generateMipsCommandLists;
	generateMipsCommandLists.reserve(commandLists.size());

	std::vector<ID3D12CommandList*>  d3d12CommandLists;
	d3d12CommandLists.reserve(commandLists.size() * 2);

	for (auto commandList : commandLists)
	{
		// * 拿到一个空的commandList作为pendingCommandList
		// ! pendingCommandList只负责记录transition barriers
		// ! 实际的命令如draw都放在传入的参数(已经录制好的)commandList上面
		auto pendingCommandList = GetCommandList(); 
		// ! 这里向pendingCommandList写入transition barriers
		bool hasPendingBarriers = commandList->Close(*pendingCommandList);
		pendingCommandList->Close();

		if (hasPendingBarriers)
		{
			d3d12CommandLists.push_back(pendingCommandList->GetGraphicsCommandList().Get());
		}
		d3d12CommandLists.push_back(commandList->GetGraphicsCommandList().Get());

		toBeQueued.push_back(pendingCommandList);
		toBeQueued.push_back(commandList);

		auto generateMipsCommandList = commandList->GetGenerateMipsCommandList();
        if ( generateMipsCommandList )
        {
            generateMipsCommandLists.push_back( generateMipsCommandList );
        }
	}

	// ! 此处为真正执行commandLists的地方
	UINT numCommandLists = static_cast<UINT>(d3d12CommandLists.size());
	m_d3d12CommandQueue->ExecuteCommandLists(numCommandLists, d3d12CommandLists.data());
	uint64_t fenceValue = Signal();

	ResourceStateTracker::Unlock();

	// Queue command lists for reuse.
    for (auto commandList : toBeQueued)
    {
        m_InFlightCommandLists.Push({ fenceValue, commandList });
    }

    // If there are any command lists that generate mips then execute those
    // after the initial resource command lists have finished.
    if ( generateMipsCommandLists.size() > 0 )
    {
	   auto computeQueue = DX12Device::GetInst()->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE);
       computeQueue->Wait( *this );
       computeQueue->ExecuteCommandLists( generateMipsCommandLists );
    }

    return fenceValue;
}

std::shared_ptr<DX12CommandList> DX12CommandQueue::GetCommandList()
{
	std::shared_ptr<DX12CommandList> commandList;
	if (!m_AvailableCommandLists.Empty())
	{
		m_AvailableCommandLists.TryPop(commandList);
	}
	else
	{
		commandList = std::make_shared<DX12CommandList>(m_CommandListType);
	}

	return commandList;
}


void DX12CommandQueue::Wait(const DX12CommandQueue& other)
{
	m_d3d12CommandQueue->Wait(other.m_d3d12Fence.Get(), other.m_FenceValue);
}

ComPtr<ID3D12CommandQueue> DX12CommandQueue::GetD3D12CommandQueue() const
{
	return m_d3d12CommandQueue;
}

void DX12CommandQueue::ProccessInFlightCommandLists()
{
	std::unique_lock<std::mutex> lock(m_ProcessInFlightCommandListsThreadMutex, std::defer_lock);

	while (m_bProcessInFlightCommandLists)
	{
		CommandListEntry commandListEntry;

		lock.lock();
		while (m_InFlightCommandLists.TryPop(commandListEntry))
		{
			auto fenceValue = std::get<0>(commandListEntry);
			auto commandList = std::get<1>(commandListEntry);

			WaitForFenceValue(fenceValue);

			commandList->Reset();

			m_AvailableCommandLists.Push(commandList);
		}
		lock.unlock();
		m_ProcessInFlightCommandListsThreadCV.notify_one();

		std::this_thread::yield();
	}
}
