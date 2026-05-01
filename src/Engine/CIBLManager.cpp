#include "pch.h"
#include "CIBLManager.h"

#include "CEquirectToCubeCS.h"
#include "CIBLIrradianceCS.h"
#include "CIBLPrefilterCS.h"
#include "CBRDFLutCS.h"
#include "CGenMipsCubemapCS.h"

#include "CAssetMgr.h"
#include "CTexture.h"

#include "RHI/IRHIDevice.h"
#include "RHI/IRHICommandList.h"

constexpr UINT ENV_CUBEMAP_SIZE     = 1024;
constexpr UINT IRRADIANCE_SIZE      = 32;
constexpr UINT PREFILTER_SIZE       = 256;
constexpr UINT PREFILTER_MIP_LEVELS = 5;
constexpr UINT BRDF_LUT_SIZE       = 512;

#ifdef USE_DX12
#include "DX12/DX12Device.h"
#endif

CIBLManager::CIBLManager()
	: m_pEquirectToCubeCS(nullptr)
	, m_pIrradianceCS(nullptr)
	, m_pPrefilterCS(nullptr)
	, m_pBRDFLutCS(nullptr)
	, m_bReady(false)
{
}

CIBLManager::~CIBLManager()
{
	m_EnvCubemap    = nullptr;
	m_IrradianceMap = nullptr;
	m_PrefilterMap  = nullptr;
	m_BRDFLutTex    = nullptr;
}

void CIBLManager::init()
{
    m_pEquirectToCubeCS = (CEquirectToCubeCS*)CAssetMgr::GetInst()->FindAsset<CComputeShader>(L"EquirectToCubeCS").Get();
    m_pIrradianceCS     = (CIBLIrradianceCS*)CAssetMgr::GetInst()->FindAsset<CComputeShader>(L"IBLIrradianceCS").Get();
    m_pPrefilterCS      = (CIBLPrefilterCS*)CAssetMgr::GetInst()->FindAsset<CComputeShader>(L"IBLPrefilterCS").Get();
    m_pBRDFLutCS        = (CBRDFLutCS*)CAssetMgr::GetInst()->FindAsset<CComputeShader>(L"BRDFLutCS").Get();
    m_pGenMipsCubemapCS = (CGenMipsCubemapCS*)CAssetMgr::GetInst()->FindAsset<CComputeShader>(L"GenMipsCubemapCS").Get();

    // Create resource first — no commands needed
    m_BRDFLutTex = CAssetMgr::GetInst()->CreateTexture(
        L"IBL_BRDFLut",
        BRDF_LUT_SIZE, BRDF_LUT_SIZE,
        DXGI_FORMAT_R16G16_FLOAT,
        RHI_BIND_FLAG::SHADER_RESOURCE | RHI_BIND_FLAG::UNORDERED_ACCESS);

#ifdef USE_DX12
    DX12Device::GetInst()->BeginFrame();
#endif
    m_pBRDFLutCS->SetOutputTex(m_BRDFLutTex, BRDF_LUT_SIZE);
    m_pBRDFLutCS->Execute();
#ifdef USE_DX12
    DX12Device::GetInst()->EndFrame();
#endif
}

void CIBLManager::GenerateBRDFLut()
{
	// Delete old if regenerating
	if (m_BRDFLutTex.Get())
		CAssetMgr::GetInst()->DeleteMyAsset<CTexture>(L"IBL_BRDFLut");

	m_BRDFLutTex = CAssetMgr::GetInst()->CreateTexture(
		L"IBL_BRDFLut",
		BRDF_LUT_SIZE, BRDF_LUT_SIZE,
		DXGI_FORMAT_R16G16_FLOAT,
		RHI_BIND_FLAG::SHADER_RESOURCE | RHI_BIND_FLAG::UNORDERED_ACCESS);


	m_pBRDFLutCS->SetOutputTex(m_BRDFLutTex, BRDF_LUT_SIZE);
	m_pBRDFLutCS->Execute();
}
void CIBLManager::GenerateEnvCubemapMips()
{
    if (!m_EnvCubemap.Get() || !m_pGenMipsCubemapCS)
        return;

    const UINT mipLevels = m_EnvCubemap->GetMipLevels();
    if (mipLevels <= 1)
        return;

    DXGI_FORMAT fmt        = m_EnvCubemap->GetRHITexture()->GetDXGIFormat();
    IRHICommandList* pCmdList = g_pRHIDevice->GetCommandList();

    for (UINT mip = 1; mip < mipLevels; ++mip)
    {
        // src mip (N-1) face size — scratch must EXACTLY match this
        UINT srcSize = max(1u, (UINT)m_EnvCubemap->GetWidth() >> (mip - 1));

        // Recreate scratch only when size changes (changes every mip level)
        if (!m_EnvCubemapScratch.Get() || (UINT)m_EnvCubemapScratch->GetWidth() != srcSize)
        {
            if (m_EnvCubemapScratch.Get())
                CAssetMgr::GetInst()->DeleteMyAsset<CTexture>(L"IBL_EnvCubemapScratch");

            m_EnvCubemapScratch = CAssetMgr::GetInst()->CreateCubemapTexture(
                L"IBL_EnvCubemapScratch",
                srcSize,
                fmt,
                RHI_BIND_FLAG::SHADER_RESOURCE | RHI_BIND_FLAG::UNORDERED_ACCESS,
                1);  // 1 mip only
        }

        // Copy src mip (N-1) → scratch mip 0 (different resource → no SRV/UAV conflict)
        pCmdList->CopySubresourceMip(
            m_EnvCubemapScratch->GetRHITexture(), 0,
            m_EnvCubemap->GetRHITexture(),        mip - 1,
            6);

        m_pGenMipsCubemapCS->SetSrcMipTex(m_EnvCubemapScratch.Get());
        m_pGenMipsCubemapCS->SetDstCubemap(m_EnvCubemap.Get());
        m_pGenMipsCubemapCS->SetDstMip(mip);
        m_pGenMipsCubemapCS->Execute();
    }

    // Cleanup scratch after all mips are done
    if (m_EnvCubemapScratch.Get())
    {
        CAssetMgr::GetInst()->DeleteMyAsset<CTexture>(L"IBL_EnvCubemapScratch");
        m_EnvCubemapScratch = nullptr;
    }
}


void CIBLManager::GenerateFromEquirect(Ptr<CTexture> _EquirectHDR)
{
    if (!_EquirectHDR.Get()) return;

    // ── 1. Create all GPU resources BEFORE BeginFrame ──────────────────────
    if (m_EnvCubemap.Get())
        CAssetMgr::GetInst()->DeleteMyAsset<CTexture>(L"IBL_EnvCubemap");
    m_EnvCubemap = CAssetMgr::GetInst()->CreateCubemapTexture(
        L"IBL_EnvCubemap", ENV_CUBEMAP_SIZE,
        DXGI_FORMAT_R16G16B16A16_FLOAT,
        RHI_BIND_FLAG::SHADER_RESOURCE | RHI_BIND_FLAG::UNORDERED_ACCESS,
        0); // full mip chain

    // Scratch shares the same mip layout as env cubemap — allocated ONCE
    const UINT mipLevels = m_EnvCubemap->GetMipLevels();
    Ptr<CTexture> scratch;
    if (mipLevels > 1)
    {
        scratch = CAssetMgr::GetInst()->CreateCubemapTexture(
            L"IBL_EnvCubemapScratch", ENV_CUBEMAP_SIZE,
            m_EnvCubemap->GetRHITexture()->GetDXGIFormat(),
            RHI_BIND_FLAG::SHADER_RESOURCE | RHI_BIND_FLAG::UNORDERED_ACCESS,
            mipLevels); // same mip count → one scratch covers all mips
    }

    if (m_IrradianceMap.Get())
        CAssetMgr::GetInst()->DeleteMyAsset<CTexture>(L"IBL_Irradiance");
    m_IrradianceMap = CAssetMgr::GetInst()->CreateCubemapTexture(
        L"IBL_Irradiance", IRRADIANCE_SIZE,
        DXGI_FORMAT_R16G16B16A16_FLOAT,
        RHI_BIND_FLAG::SHADER_RESOURCE | RHI_BIND_FLAG::UNORDERED_ACCESS);

    if (m_PrefilterMap.Get())
        CAssetMgr::GetInst()->DeleteMyAsset<CTexture>(L"IBL_Prefilter");
    m_PrefilterMap = CAssetMgr::GetInst()->CreateCubemapTexture(
        L"IBL_Prefilter", PREFILTER_SIZE,
        DXGI_FORMAT_R16G16B16A16_FLOAT,
        RHI_BIND_FLAG::SHADER_RESOURCE | RHI_BIND_FLAG::UNORDERED_ACCESS,
        PREFILTER_MIP_LEVELS);

    // ── 2. Record ALL commands in one frame ────────────────────────────────
#ifdef USE_DX12
    DX12Device::GetInst()->BeginFrame();
#endif

    IRHICommandList* pCmdList = g_pRHIDevice->GetCommandList();

    // Equirect → Cubemap mip 0
    m_pEquirectToCubeCS->SetEquirectTexture(_EquirectHDR);
    m_pEquirectToCubeCS->SetOutputCubeMapTex(m_EnvCubemap, ENV_CUBEMAP_SIZE);
    m_pEquirectToCubeCS->Execute();

    // Cubemap mip chain: copy mip N-1 → scratch mip N-1, CS writes env mip N
    for (UINT mip = 1; mip < mipLevels; ++mip)
    {
        pCmdList->CopySubresourceMip(
            scratch->GetRHITexture(), mip - 1,
            m_EnvCubemap->GetRHITexture(), mip - 1,
            6);

        m_pGenMipsCubemapCS->SetSrcMipTex(scratch.Get());
        m_pGenMipsCubemapCS->SetSrcMip(mip - 1);   // read scratch mip N-1
        m_pGenMipsCubemapCS->SetDstCubemap(m_EnvCubemap.Get());
        m_pGenMipsCubemapCS->SetDstMip(mip);
        m_pGenMipsCubemapCS->Execute();
    }

    // Irradiance convolution
    m_pIrradianceCS->SetSourceCubemap(m_EnvCubemap);
    m_pIrradianceCS->SetOutputIrradiance(m_IrradianceMap, IRRADIANCE_SIZE);
    m_pIrradianceCS->Execute();

    // Prefiltered specular — all mips in the same frame
    m_pPrefilterCS->SetEnvMapSize(ENV_CUBEMAP_SIZE);
    for (UINT mip = 0; mip < PREFILTER_MIP_LEVELS; ++mip)
    {
        UINT  mipSize   = PREFILTER_SIZE >> mip;
        float roughness = (float)mip / (float)(PREFILTER_MIP_LEVELS - 1);
        m_pPrefilterCS->SetSourceCubemap(m_EnvCubemap);
        m_pPrefilterCS->SetOutputPrefilter(m_PrefilterMap);
        m_pPrefilterCS->SetMipLevel(mipSize, mip, roughness);
        m_pPrefilterCS->Execute();
    }

#ifdef USE_DX12
    DX12Device::GetInst()->EndFrame(); // GPU done — safe to delete scratch
#endif

    // ── 3. Release scratch after GPU has finished ──────────────────────────
    if (scratch.Get())
        CAssetMgr::GetInst()->DeleteMyAsset<CTexture>(L"IBL_EnvCubemapScratch");

    m_bReady = true;
}

void CIBLManager::GenerateFromCubemap(Ptr<CTexture> _Cubemap)
{
	if (!_Cubemap.Get())
		return;

	m_EnvCubemap = _Cubemap;

	// 2. Irradiance convolution (32x32 cubemap)
	if (m_IrradianceMap.Get())
		CAssetMgr::GetInst()->DeleteMyAsset<CTexture>(L"IBL_Irradiance");

	m_IrradianceMap = CAssetMgr::GetInst()->CreateCubemapTexture(
		L"IBL_Irradiance", IRRADIANCE_SIZE,
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		RHI_BIND_FLAG::SHADER_RESOURCE | RHI_BIND_FLAG::UNORDERED_ACCESS);

	m_pIrradianceCS->SetSourceCubemap(m_EnvCubemap);
	m_pIrradianceCS->SetOutputIrradiance(m_IrradianceMap, IRRADIANCE_SIZE);
	m_pIrradianceCS->Execute();

	// 3. Prefiltered specular (256x256, 5 mip levels)
	if (m_PrefilterMap.Get())
		CAssetMgr::GetInst()->DeleteMyAsset<CTexture>(L"IBL_Prefilter");

	m_PrefilterMap = CAssetMgr::GetInst()->CreateCubemapTexture(
		L"IBL_Prefilter", PREFILTER_SIZE,
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		RHI_BIND_FLAG::SHADER_RESOURCE | RHI_BIND_FLAG::UNORDERED_ACCESS,
		PREFILTER_MIP_LEVELS);

	// Dispatch per mip level — Binding() handles per-mip UAV via m_MipSlice
	m_pPrefilterCS->SetEnvMapSize(ENV_CUBEMAP_SIZE);
	for (UINT mip = 0; mip < PREFILTER_MIP_LEVELS; ++mip)
	{
		UINT mipSize = PREFILTER_SIZE >> mip;
		float roughness = (float)mip / (float)(PREFILTER_MIP_LEVELS - 1);

		m_pPrefilterCS->SetSourceCubemap(m_EnvCubemap);
		m_pPrefilterCS->SetOutputPrefilter(m_PrefilterMap);
		m_pPrefilterCS->SetMipLevel(mipSize, mip, roughness);
		m_pPrefilterCS->Execute();
	}




	m_bReady = true;
}

void CIBLManager::Binding()
{
	if (!m_bReady)
		return;

	// t5 = BRDF LUT
	if (m_BRDFLutTex.Get())
		m_BRDFLutTex->Binding(5);

	// t7 = Irradiance cubemap
	if (m_IrradianceMap.Get())
		m_IrradianceMap->Binding(7);

	// t8 = Prefiltered specular cubemap
	if (m_PrefilterMap.Get())
		m_PrefilterMap->Binding(8);

	// Note: t6 (env cubemap for skybox) is bound by CSkyBox
}

void CIBLManager::Clear()
{
	CTexture::Clear(5);
	CTexture::Clear(7);
	CTexture::Clear(8);
}