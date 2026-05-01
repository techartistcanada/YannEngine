#include "pch.h"
#include "CRenderMgr.h"

#include "CAssetMgr.h"
#include "assets.h"
#include "CRenderTargetSet.h"
#include "RHI/IRHIDevice.h"
#include "RHI/RHIPrereqs.h"
#include "CIBLManager.h"

void CRenderMgr::init()
{
	CreateRenderTargetSet();
	CIBLManager::GetInst()->init();
}

void CRenderMgr::CreateRenderTargetSet()
{
	Vec2 vResolution = g_pRHIDevice->GetRenderResolution();

	// 0. Post-process用的Render Target纹理,大小和格式与主RenderTarget纹理相同
	m_RenderTargetTexCopy = CAssetMgr::GetInst()->CreateTexture(L"RenderTargetTexCopy"
		, (UINT)vResolution.x
		, (UINT)vResolution.y
		, DXGI_FORMAT_R8G8B8A8_UNORM
		, RHI_BIND_FLAG::SHADER_RESOURCE);

	// ===============================
	// 1. For Final Output 创建最终输出阶段的渲染目标集合,包含一个RenderTarget纹理和一个DepthStencil纹理
	// ===============================
	CRenderTargetSet* pRenderTargetSet = nullptr;
	pRenderTargetSet = m_MRT[(UINT)MRT_TYPE::SWAPCHAIN] = new CRenderTargetSet;

	Ptr<CTexture> pRTTex = CAssetMgr::GetInst()->FindAsset<CTexture>(L"RenderTargetTex");
	// create depth stencil texture 
	Ptr<CTexture> pDSTex = CAssetMgr::GetInst()->CreateTexture(L"DepthStencilTex", (UINT)vResolution.x, (UINT)vResolution.y, DXGI_FORMAT_D24_UNORM_S8_UINT, RHI_BIND_FLAG::DEPTH_STENCIL);

	pRenderTargetSet->Init(&pRTTex, 1, pDSTex);

	// ===============================
	// 2. GBuffer Geometry Pass阶段的渲染目标集合,包含多个RenderTarget纹理和一个Depth Stencil纹理
	// ===============================
	pRenderTargetSet = m_MRT[(UINT)MRT_TYPE::DEFERRED] = new CRenderTargetSet;
	Ptr<CTexture> arrRTTextures[8] =
	{
		// 0. color
		CAssetMgr::GetInst()->CreateTexture(L"GBuffer_Color", (UINT)vResolution.x, (UINT)vResolution.y, 
											DXGI_FORMAT_R16G16B16A16_FLOAT, 
											RHI_BIND_FLAG::RENDER_TARGET | RHI_BIND_FLAG::SHADER_RESOURCE),
		// 1. normal
		CAssetMgr::GetInst()->CreateTexture(L"GBuffer_Normal", (UINT)vResolution.x, (UINT)vResolution.y,
											DXGI_FORMAT_R32G32B32A32_FLOAT,
											RHI_BIND_FLAG::RENDER_TARGET | RHI_BIND_FLAG::SHADER_RESOURCE),
		// 2. position
		// 存的是ViewSpace坐标(xyz) and 几何体标记(a=1.0),需要高精度浮点
		// TODO: 改用深度重建位置(Reconstruct Position from Depth),不再直接存储世界空间位置以节省内存带宽
		CAssetMgr::GetInst()->CreateTexture(L"GBuffer_Position", (UINT)vResolution.x, (UINT)vResolution.y,
											DXGI_FORMAT_R32G32B32A32_FLOAT,
											RHI_BIND_FLAG::RENDER_TARGET | RHI_BIND_FLAG::SHADER_RESOURCE),
		// 3. emmisive
		CAssetMgr::GetInst()->CreateTexture(L"GBuffer_Emmisive", (UINT)vResolution.x, (UINT)vResolution.y,
											DXGI_FORMAT_R32G32B32A32_FLOAT,
											RHI_BIND_FLAG::RENDER_TARGET | RHI_BIND_FLAG::SHADER_RESOURCE),
		// 4. custom data
		CAssetMgr::GetInst()->CreateTexture(L"GBuffer_CustomData", (UINT)vResolution.x, (UINT)vResolution.y,
											DXGI_FORMAT_R32G32B32A32_FLOAT,
											RHI_BIND_FLAG::RENDER_TARGET | RHI_BIND_FLAG::SHADER_RESOURCE),
	};
	pRenderTargetSet->Init(arrRTTextures, 5, pDSTex);

	// ===============================
	// 3.SSAO
	// ===============================
	pRenderTargetSet = m_MRT[(UINT)MRT_TYPE::SSAO] = new CRenderTargetSet;
	Ptr<CTexture> pSSAOTex = CAssetMgr::GetInst()->CreateTexture(
		L"SSAOTexture",
		(UINT)vResolution.x, (UINT)vResolution.y,
		DXGI_FORMAT_R8_UNORM, 
		RHI_BIND_FLAG::RENDER_TARGET | RHI_BIND_FLAG::SHADER_RESOURCE);
	pRenderTargetSet->Init(&pSSAOTex, 1, nullptr);
	// 必须为1，否则ambient=0
	pRenderTargetSet->SetClearColor(0, Vec4(1.f, 1.f, 1.f, 1.f)); 


	// ===============================
	// 4.SSAO Blur
	// ===============================
	pRenderTargetSet = m_MRT[(UINT)MRT_TYPE::SSAO_BLUR] = new CRenderTargetSet;
	Ptr<CTexture> pSSAOBlurTex = CAssetMgr::GetInst()->CreateTexture(
		L"SSAOBlurTex",
		(UINT)vResolution.x,
		(UINT)vResolution.y,
		DXGI_FORMAT_R8_UNORM,
		RHI_BIND_FLAG::RENDER_TARGET | RHI_BIND_FLAG::SHADER_RESOURCE);
	pRenderTargetSet->Init(&pSSAOBlurTex, 1, nullptr);
	// 必须为1，否则ambient=0
	pRenderTargetSet->SetClearColor(0, Vec4(1.f, 1.f, 1.f, 1.f)); 

	// ===============================
	// 5. For Deferred Lighting Pass 创建DeferredLighting阶段的渲染目标集合,包含一个RenderTarget纹理和一个DepthStencil纹理
	// ===============================
	pRenderTargetSet = m_MRT[(UINT)MRT_TYPE::DEFERRED_LIGHT] = new CRenderTargetSet;
	Ptr<CTexture> arrRTTexturesLighting[8] =
	{
		// 0. diffuse color
		CAssetMgr::GetInst()->CreateTexture(L"GBuffer_Diffuse", (UINT)vResolution.x, (UINT)vResolution.y, 
											DXGI_FORMAT_R32G32B32A32_FLOAT,
											RHI_BIND_FLAG::RENDER_TARGET | RHI_BIND_FLAG::SHADER_RESOURCE),
		// 1. specular color
		CAssetMgr::GetInst()->CreateTexture(L"GBuffer_Specular", (UINT)vResolution.x, (UINT)vResolution.y,
											DXGI_FORMAT_R32G32B32A32_FLOAT,
											RHI_BIND_FLAG::RENDER_TARGET | RHI_BIND_FLAG::SHADER_RESOURCE),
	};
	pRenderTargetSet->Init(arrRTTexturesLighting, 2, nullptr);

	// ===============================
	// 6. For Deferred Decal 创建 Deferred Decal 阶段的渲染目标集合
	// ===============================
	pRenderTargetSet = m_MRT[(UINT)MRT_TYPE::DEFERRED_DECAL] = new CRenderTargetSet;
	Ptr<CTexture> arrRTTexturesDecal[8] =
	{
		// NOTE: decal pass 直接在G-Buffer的基础上进行修改
		CAssetMgr::GetInst()->FindAsset<CTexture>(L"GBuffer_Color"),

		CAssetMgr::GetInst()->FindAsset<CTexture>(L"GBuffer_Emmisive"),
	};
	pRenderTargetSet->Init(arrRTTexturesDecal, 2, nullptr);

	// ===============================
	// 7. For HDR Scene 创建HDR Scene阶段的渲染目标集合
	// ===============================
	pRenderTargetSet = m_MRT[(UINT)MRT_TYPE::HDR_SCENE] = new CRenderTargetSet;
	Ptr<CTexture> pHDRSceneTex = CAssetMgr::GetInst()->CreateTexture(
		L"HDRSceneTex",
		(UINT)vResolution.x,
		(UINT)vResolution.y,
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		RHI_BIND_FLAG::RENDER_TARGET | RHI_BIND_FLAG::SHADER_RESOURCE
	);
	pRenderTargetSet->Init(&pHDRSceneTex, 1, pDSTex);

}

void CRenderMgr::ClearRenderTargetSet()
{
	m_MRT[(UINT)MRT_TYPE::SWAPCHAIN]->ClearTargets();
	m_MRT[(UINT)MRT_TYPE::SWAPCHAIN]->ClearDepthStencil();

	m_MRT[(UINT)MRT_TYPE::DEFERRED]->ClearTargets();
	m_MRT[(UINT)MRT_TYPE::DEFERRED_LIGHT]->ClearTargets();
	m_MRT[(UINT)MRT_TYPE::SSAO]->ClearTargets();
	m_MRT[(UINT)MRT_TYPE::SSAO_BLUR]->ClearTargets();
}
