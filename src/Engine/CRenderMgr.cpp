#include "pch.h"
#include "CRenderMgr.h"

#ifdef USE_DX11
#include "DX11/DX11Device.h"
#else 
#include "DX12/DX12Device.h"
#endif

#include "RHI/IRHIDevice.h"
#include "RHI/IRHICommandList.h"

#include "CCamera.h"
#include "CTransform.h"
#include "CLight2D.h"
#include "CLight3D.h"
#include "CStructuredBuffer.h"
#include "CConstantBuffer.h"
#include "CRenderTargetSet.h"
#include "CIBLManager.h"

#include "CAssetMgr.h"
#include "CLevelMgr.h"
#include "CFontMgr.h"
#include "CTimeMgr.h"


CRenderMgr::CRenderMgr()
	: m_Light2DBuffer(nullptr)
	, m_Light3DBuffer(nullptr)
	, m_EditorCam(nullptr)
	, m_MRT{}
	, m_bDeferredDebugViewActive(false)
	, m_bShowBoundingBox(true)
	, m_NumDrawCalls(0)
{
	m_Light2DBuffer = new CStructuredBuffer;
	m_Light3DBuffer = new CStructuredBuffer;
	RenderFunc = &CRenderMgr::render_play;
}

CRenderMgr::~CRenderMgr()
{
	delete m_Light2DBuffer;
	delete m_Light3DBuffer;

	Safe_Del_Array(m_MRT);
}


void CRenderMgr::render()
{
	if (!CLevelMgr::GetInst()->GetCurrentLevel())
		return;

	m_NumDrawCalls = 0;


	ClearRenderTargetSet();

	DataBinding();


	(this->*RenderFunc)();

	DataClear();
}

void CRenderMgr::display_numdrawcalls()
{
	wchar_t szDrawCall[255] = {};

	swprintf_s(szDrawCall, L"Draw Calls: %d", m_NumDrawCalls);

	CFontMgr::GetInst()->DrawFont(szDrawCall, 10.f, 55.f, 20.f, FONT_RGBA(0, 255, 0, 255));
}

void CRenderMgr::display_fps()
{
	wchar_t szFPS[255] = {};

	swprintf_s(szFPS, L"FPS: %d", CTimeMgr::GetInst()->GetFPS());

	CFontMgr::GetInst()->DrawFont(szFPS, 10.f, 30.f, 20.f, FONT_RGBA(0, 255, 0, 255));
}

void CRenderMgr::display_camerapos()
{
	CCamera* pCam = m_EditorCam ? m_EditorCam : (m_vecCams.empty() ? nullptr : m_vecCams[0]);
	if (!pCam)
		return;

	Vec3 vPos = pCam->GetOwner()->Transform()->GetRelativePos();
	Vec3 vRot = pCam->GetOwner()->Transform()->GetRelativeRotation();

	wchar_t szBuf[255] = {};
	swprintf_s(szBuf, L"Cam Pos: (%.1f, %.1f, %.1f)", vPos.x, vPos.y, vPos.z);

	swprintf_s(szBuf, L"Cam Rot: (%.1f, %.1f, %.1f)",
		XMConvertToDegrees(vRot.x),
		XMConvertToDegrees(vRot.y),
		XMConvertToDegrees(vRot.z));

	CFontMgr::GetInst()->DrawFont(szBuf, 10.f, 75.f, 20.f, FONT_RGBA(0, 255, 0, 255));
}

void CRenderMgr::render_play()
{
	for (size_t i = 0; i < m_vecCams.size(); ++i)
	{
		if(m_vecCams[i] == nullptr)
			continue;

		m_vecCams[i]->render();
	}

	// ! NOTE:
	// Re-bind SWAPCHAIN MRT — the camera's render_postprocess() may leave
	// the back buffer in COPY_SOURCE state after CopyRenderTarget().
	m_MRT[(UINT)MRT_TYPE::SWAPCHAIN]->OMSet();
}

void CRenderMgr::render_editor()
{
	if (nullptr == m_EditorCam)
		return;

	// ==================================
	// 0. shadow map
	// ==================================
	render_shadowmap();

	// ==================================
	// 1. transform data & sort objects
	// ==================================
	g_Trans.matView = m_EditorCam->GetViewMat();
	g_Trans.matViewInv = m_EditorCam->GetViewMatInv();
	g_Trans.matProj = m_EditorCam->GetProjMat();
	g_Trans.matProjInv = m_EditorCam->GetProjMatInv();

	m_EditorCam->SortObjects();

	// ====================
	// 2. Bind IBL textures
	// ====================
	CIBLManager::GetInst()->Binding();

	// ====================
	// 3. Deferred G-Buffer Pass
	// ====================
	m_MRT[(UINT)MRT_TYPE::DEFERRED]->OMSet();
	m_EditorCam->render_deferred();
	
	// ====================
	// 4. Decal Pass(modify color and emissive)
	// ====================
	m_MRT[(UINT)MRT_TYPE::DEFERRED_DECAL]->OMSet();
	m_EditorCam->render_deferred_decal();

	// ====================
	// 5. SSAO Pass
	// ====================
	//render_ssao();

	// ====================
	// 6. Lighting Pass(read gbuffer and write into diffuse and specular)
	// ====================
	m_MRT[(UINT)MRT_TYPE::DEFERRED_LIGHT]->OMSet();


	for (size_t i = 0; i < m_vecLights3D.size(); ++i)
	{
		m_vecLights3D[i]->ApplyLighting();
		CRenderMgr::GetInst()->IncrDrawCallCount();
	}

	// ====================
	// 7. Merge Pass
	// ====================
	m_MRT[(UINT)MRT_TYPE::HDR_SCENE]->OMSet();

	Ptr<CMesh> pRectMesh = CAssetMgr::GetInst()->FindAsset<CMesh>(L"RectMesh");
	Ptr<CMaterial> pMergingMat = CAssetMgr::GetInst()->FindAsset<CMaterial>(L"DeferredMergingMaterial");

	if (m_bDeferredDebugViewActive)
	{
		// ========================
		// Debug View for G-Buffer
		// ========================
		pMergingMat->SetTexParam(TEX_PARAM::TEX_0, nullptr);
		pMergingMat->SetTexParam(TEX_PARAM::TEX_1, nullptr);
		pMergingMat->SetTexParam(TEX_PARAM::TEX_2, nullptr);
		pMergingMat->SetTexParam(TEX_PARAM::TEX_3, nullptr);
		pMergingMat->SetTexParam(TEX_PARAM::TEX_4, nullptr);
		pMergingMat->SetTexParam(TEX_PARAM::TEX_5, m_DebugViewRTTexture);

		pMergingMat->SetScalarParam(SCALAR_PARAM::INT_0, 1); // Debug View Flag
	}
	else
	{
		pMergingMat->SetTexParam(TEX_PARAM::TEX_0, CAssetMgr::GetInst()->FindAsset<CTexture>(L"GBuffer_Color"));
		pMergingMat->SetTexParam(TEX_PARAM::TEX_1, CAssetMgr::GetInst()->FindAsset<CTexture>(L"GBuffer_Diffuse"));
		pMergingMat->SetTexParam(TEX_PARAM::TEX_2, CAssetMgr::GetInst()->FindAsset<CTexture>(L"GBuffer_Specular"));
		pMergingMat->SetTexParam(TEX_PARAM::TEX_3, CAssetMgr::GetInst()->FindAsset<CTexture>(L"GBuffer_Emmisive"));
		//pMergingMat->SetTexParam(TEX_PARAM::TEX_4, CAssetMgr::GetInst()->FindAsset<CTexture>(L"GBuffer_Position"));
		pMergingMat->SetTexParam(TEX_PARAM::TEX_4, CAssetMgr::GetInst()->FindAsset<CTexture>(L"GBuffer_CustomData"));
		pMergingMat->SetTexParam(TEX_PARAM::TEX_5, m_DebugViewRTTexture);

		pMergingMat->SetScalarParam(SCALAR_PARAM::INT_0, 0); // Debug View Flag
	}

	pMergingMat->Binding();
	pRectMesh->render();
	CRenderMgr::GetInst()->IncrDrawCallCount();


	// ====================
	// 8. Forward rendering
	// ====================
	m_EditorCam->render_opaque();
	m_EditorCam->render_masked();
	m_EditorCam->render_transparent();
	m_EditorCam->render_particle();

	// ===================
	// 9. Tone Mapping — HDR_SCENE → SWAPCHAIN
	// ===================
	m_MRT[(UINT)MRT_TYPE::SWAPCHAIN]->OMSet();

	Ptr<CMaterial> pToneMapMat = CAssetMgr::GetInst()->FindAsset<CMaterial>(L"ToneMappingMaterial");
	pToneMapMat->SetTexParam(TEX_PARAM::TEX_0, CAssetMgr::GetInst()->FindAsset<CTexture>(L"HDRSceneTex"));
	pToneMapMat->SetScalarParam(SCALAR_PARAM::FLOAT_0, 0.6f); // Exposure
	pToneMapMat->SetScalarParam(SCALAR_PARAM::INT_0, 0);       // 0=ACES, 1=Reinhard, 2=Uncharted2
	pToneMapMat->Binding();
	pRectMesh->render();
	CRenderMgr::GetInst()->IncrDrawCallCount();

	// ===================
	// 10.Postprocess
	// ===================
	m_EditorCam->render_postprocess();

	// ===================
	// 11. Clear IBL
	// ===================
	CIBLManager::GetInst()->Clear();

	// ! NOTE:
	// TODO: FIXME: 
	// Re-bind SWAPCHAIN MRT — CopyRenderTarget() inside render_postprocess() transitions
	// the back buffer to COPY_SOURCE. We must transition it back to RENDER_TARGET
	// so that subsequent draws (CDbgRenderMgr, etc.) can use it.
	m_MRT[(UINT)MRT_TYPE::SWAPCHAIN]->OMSet();
}

void CRenderMgr::render_shadowmap()
{
	for (size_t i = 0; i < m_vecLights3D.size(); ++	i)
	{
		if (m_vecLights3D[i]->GetIsRenderShadow())
		{
			m_vecLights3D[i]->RenderShadowMap();
		}
	}
}

void CRenderMgr::render_ssao()
{
	Ptr<CMesh>     pRectMesh = CAssetMgr::GetInst()->FindAsset<CMesh>(L"RectMesh");
	Ptr<CMaterial> pSSAOMat  = CAssetMgr::GetInst()->FindAsset<CMaterial>(L"SSAOMaterial");

	pSSAOMat->SetTexParam(TEX_PARAM::TEX_0, CAssetMgr::GetInst()->FindAsset<CTexture>(L"GBuffer_Position"));
	pSSAOMat->SetTexParam(TEX_PARAM::TEX_1, CAssetMgr::GetInst()->FindAsset<CTexture>(L"GBuffer_Normal"));
	pSSAOMat->SetTexParam(TEX_PARAM::TEX_2, 
		CAssetMgr::GetInst()->Load<CTexture>(L"SSAONoise", L"texture\\ssao_noise.png"));

	// Pass 1: SSAO generation
	m_MRT[(UINT)MRT_TYPE::SSAO]->OMSet();
	pSSAOMat->Binding();
	pRectMesh->render();
	IncrDrawCallCount();

	// Pass 2: Bilateral blur
	m_MRT[(UINT)MRT_TYPE::SSAO_BLUR]->OMSet();
	Ptr<CMaterial> pBlurMat = CAssetMgr::GetInst()->FindAsset<CMaterial>(L"SSAOBlurMaterial");
	pBlurMat->SetTexParam(TEX_PARAM::TEX_0, CAssetMgr::GetInst()->FindAsset<CTexture>(L"SSAOTexture")); // 修复: "SSAOTex" → "SSAOTexture"
	pBlurMat->Binding();
	pRectMesh->render();
	IncrDrawCallCount();

	// Bind blurred SSAO result to t17 for lighting pass
	Ptr<CTexture> pSSAOBlurTex = CAssetMgr::GetInst()->FindAsset<CTexture>(L"SSAOBlurTex");
	pSSAOBlurTex->Binding(17);
}



void CRenderMgr::DataBinding()
{

	// ====================
	// Set global data to Constant Buffer 设置全局数据到常量缓冲区
	// ====================
	g_GlobalData.Light2DCount = (UINT)m_vecLights2D.size();
	g_GlobalData.Light3DCount = (UINT)m_vecLights3D.size();
	static CConstantBuffer* pGlobalCB = RHI_DEVICE->GetConstantBuffer(CB_TYPE::GLOBAL);
	pGlobalCB->SetData(&g_GlobalData);
	pGlobalCB->Binding();
	pGlobalCB->Binding_CS();

	// ====================
	// Set 2d light data to Structured Buffer 设置灯光数据到结构化缓冲区
	// ====================
	if (m_Light2DBuffer->GetElementCount() < m_vecLights2D.size())
	{
		m_Light2DBuffer->Create(sizeof(tLightInfo),  (UINT)m_vecLights2D.size(), SB_TYPE::SRV_ONLY, true);
	}

	// NOTE: 临时变量，避免频繁分配和释放内存
	static vector<tLightInfo> vecLightInfos;
	vecLightInfos.clear();

	for (size_t i = 0; i < m_vecLights2D.size(); ++i)
	{
		vecLightInfos.push_back(m_vecLights2D[i]->GetLightInfo());
	}

	// 只有当有灯光的时候才设置数据和绑定
	if (!vecLightInfos.empty())
	{
		m_Light2DBuffer->SetData(vecLightInfos.data(), (UINT)m_vecLights2D.size());
		m_Light2DBuffer->Binding(15); // 10
	}
	// ====================
	// Set 3d light data to Structured Buffer 设置灯光数据到结构化缓冲区
	// ====================
	if (m_Light3DBuffer->GetElementCount() < m_vecLights3D.size())
	{
		m_Light3DBuffer->Create(sizeof(tLightInfo),  (UINT)m_vecLights3D.size(), SB_TYPE::SRV_ONLY, true);
	}

	vecLightInfos.clear();

	for (size_t i = 0; i < m_vecLights3D.size(); ++i)
	{
		vecLightInfos.push_back(m_vecLights3D[i]->GetLightInfo());
	}

	// 只有当有灯光的时候才设置数据和绑定
	if (!vecLightInfos.empty())
	{
		m_Light3DBuffer->SetData(vecLightInfos.data(), (UINT)m_vecLights3D.size());
		m_Light3DBuffer->Binding(16); // 10
	}
}

void CRenderMgr::DataClear()
{
	m_vecLights2D.clear();
	m_vecLights3D.clear();

	// Unbind all PS texture SRV slots to prevent RT/SRV hazard on the next
	// frame's geometry pass. The merge pass leaves GBuffer textures bound as
	// SRVs (slots 0-5); if not cleared, OMSetRenderTargets will conflict when
	// those same textures are re-bound as render targets.
	IRHICommandList* pCmdList = g_pRHIDevice->GetCommandList();
	for (UINT i = 0; i < 16; ++i)
		pCmdList->ClearTextureSRV(i);
}


void CRenderMgr::Resize(UINT _Width, UINT _Height)
{
	if (!g_pRHIDevice || !g_pRHIDevice->GetCommandList())
		return;

	g_pRHIDevice->SetResizing(true);

	
	g_pRHIDevice->GetCommandList()->Flush(); // 确保GPU不再使用这些资源了
	// 1.释放原来的Render Target Set
#ifdef USE_DX11
	g_pRHIDevice->GetCommandList()->SetRenderTargets(nullptr, 0, nullptr);
#endif

	// 2. 删除原来的 Render Target Set（会释放 COM 引用）
	for (UINT i = 0; i < (UINT)MRT_TYPE::END; ++i)
	{
		if (m_MRT[i])
		{
			delete m_MRT[i];
			m_MRT[i] = nullptr;
		}
	}
	m_RenderTargetTexCopy = nullptr;
	// 3. 从 AssetMgr 删除旧纹理
	CAssetMgr::GetInst()->DeleteMyAsset<CTexture>(L"RenderTargetTex");
	CAssetMgr::GetInst()->DeleteMyAsset<CTexture>(L"DepthStencilTex");
	CAssetMgr::GetInst()->DeleteMyAsset<CTexture>(L"RenderTargetTexCopy");

	// gbuffer textures
	CAssetMgr::GetInst()->DeleteMyAsset<CTexture>(L"GBuffer_Color");
	CAssetMgr::GetInst()->DeleteMyAsset<CTexture>(L"GBuffer_Normal");
	CAssetMgr::GetInst()->DeleteMyAsset<CTexture>(L"GBuffer_Position");
	CAssetMgr::GetInst()->DeleteMyAsset<CTexture>(L"GBuffer_Emmisive");
	CAssetMgr::GetInst()->DeleteMyAsset<CTexture>(L"GBuffer_CustomData");
	CAssetMgr::GetInst()->DeleteMyAsset<CTexture>(L"GBuffer_Diffuse");
	CAssetMgr::GetInst()->DeleteMyAsset<CTexture>(L"GBuffer_Specular");

	CAssetMgr::GetInst()->DeleteMyAsset<CTexture>(L"HDRSceneTex");

	// 4. CDevice 负责 SwapChain 级别操作（ResizeBuffers + 新 RT + Viewport）
	g_pRHIDevice->ResizeSwapChain(_Width, _Height);

	// 5. 重建自身管理的资源
	CreateRenderTargetSet();


	g_pRHIDevice->SetResizing(false);
}

void CRenderMgr::CopyRenderTarget()
{
	Ptr<CTexture> pRTTex = CAssetMgr::GetInst()->FindAsset<CTexture>(L"RenderTargetTex");

	g_pRHIDevice->GetCommandList()->CopyTexture(
	m_RenderTargetTexCopy->GetRHITexture(),
	pRTTex->GetRHITexture());

}

void CRenderMgr::RegisterCamera(CCamera* _Cam, int _Priority)
{

	if (m_vecCams.size() <= _Priority)
	{
		m_vecCams.resize(_Priority + 1);
	}

	// NOTE: 如果优先级冲突了，说明有两个相机设置了同样的优先级，这时需要抛出一个断言
	if (m_vecCams[_Priority] != nullptr && m_vecCams[_Priority] != _Cam)
	{
		assert(nullptr, "Camera priority conflict!");
	}


	m_vecCams[_Priority] = _Cam;
}


void CRenderMgr::tick()
{
}

