#include "pch.h"
#include "CEngine.h"

#ifdef USE_DX12
#include "DX12/DX12Device.h"
#include "DX12/DX12Helpers.h"
#include "DX12/DX12CommandList.h"
#include "DX12/DX12CommandQueue.h"
#include "DX12/DX12VertexBuffer.h"
#include "DX12/DX12IndexBuffer.h"
#include "DX12/DX12RootSignature.h"
#include "DX12/DX12ResourceStateTracker.h"
#include "DX12/DX12Texture.h"
#include "DX12/DX12RenderTarget.h"
#endif


#ifdef USE_DX11
#include "DX11/DX11Device.h"
#endif

#include "CTimeMgr.h"
#include "CPathMgr.h"
#include "CKeyMgr.h"
#include "CAssetMgr.h"
#include "CLevelMgr.h"
#include "CRenderMgr.h"
#include "CDbgRenderMgr.h"
#include "CCollisionManager.h"
#include "CTaskMgr.h"
#include "CFontMgr.h"

#include "RHI/IRHIDevice.h"
IRHIDevice* g_pRHIDevice = nullptr;


CEngine::CEngine()
	: m_hMainHwnd(nullptr)
	, m_Resolution{}
{
}

CEngine::~CEngine()
{
}

int CEngine::Init(HWND _hWnd, Vec2 _Resolution)
{
	m_hMainHwnd = _hWnd;
	m_Resolution = _Resolution;

	// 0.设置窗口大小和位置
	// 0. Set window size and position
	RECT rt = { 0, 0, (int)m_Resolution.x, (int)m_Resolution.y };
	AdjustWindowRect(&rt, WS_OVERLAPPEDWINDOW, false);
	SetWindowPos(m_hMainHwnd, nullptr, 0, 0, rt.right - rt.left, rt.bottom - rt.top, 0);

	// 1. 初始化RHI设备
	// 1. Initialize RHI device
#ifdef USE_DX12
	g_pRHIDevice = DX12Device::GetInst();
	if (FAILED(DX12Device::GetInst()->init(m_hMainHwnd, m_Resolution)))
	{
		MessageBox(m_hMainHwnd, L"Failed to initialize DirectX 12 Device", L"Error", MB_OK);
		return E_FAIL;
	}
#else
	g_pRHIDevice = DX11Device::GetInst();
	if(FAILED(DX11Device::GetInst()->init(m_hMainHwnd, m_Resolution)))
	{
		MessageBox(m_hMainHwnd, L"Failed to initialize DirectX 11 Device", L"Error", MB_OK);
		return E_FAIL;
	}
#endif

	// 2.Initialize Managers 初始化Managers
	CPathMgr::GetInst()->init();
	CTimeMgr::GetInst()->init();
	CKeyMgr::GetInst()->init();
	CAssetMgr::GetInst()->init();
	CRenderMgr::GetInst()->init();
	CLevelMgr::GetInst()->init();
	// TODO: dx12 font rendering
#ifdef USE_DX11
	CFontMgr::GetInst()->init();
#endif

	return S_OK;
}

 void CEngine::Progress()
{
	if(RHI_DEVICE->IsResizing())
        return;

	// ====================
	// Tick 
	// ====================
	CTimeMgr::GetInst()->tick();
	CKeyMgr::GetInst()->tick();
	CAssetMgr::GetInst()->tick();
	CLevelMgr::GetInst()->tick();
	CCollisionManager::GetInst()->tick();

	// ====================
	// Render 渲染
	// ====================
#ifdef USE_DX12
    DX12Device::GetInst()->BeginFrame();
#endif
	CRenderMgr::GetInst()->render();
	//CDbgRenderMgr::GetInst()->render();
#ifdef USE_DX11
	CRenderMgr::GetInst()->display_numdrawcalls();
	CRenderMgr::GetInst()->display_fps();
	CRenderMgr::GetInst()->display_camerapos();
#endif

	// ====================
	// Task Manager
	// ====================
	CTaskMgr::GetInst()->tick();

}
