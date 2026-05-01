#pragma once
#include "Singleton.h"

class CEquirectToCubeCS;
class CIBLIrradianceCS;
class CIBLPrefilterCS;
class CBRDFLutCS;
class CGenMipsCubemapCS;

class CIBLManager :
	public CSingleton<CIBLManager>
{
	SINGLE(CIBLManager)
private:
	// Precomputed IBL textures
	Ptr<CTexture> m_EnvCubemap;
	Ptr<CTexture> m_IrradianceMap;
	Ptr<CTexture> m_PrefilterMap;
	Ptr<CTexture> m_BRDFLutTex;
	Ptr<CTexture> m_EnvCubemapScratch;

	// Compute shaders for generating IBL textures
	CEquirectToCubeCS*	  m_pEquirectToCubeCS;
	CIBLIrradianceCS*     m_pIrradianceCS;
	CIBLPrefilterCS*      m_pPrefilterCS;
	CBRDFLutCS*			  m_pBRDFLutCS;
	CGenMipsCubemapCS*    m_pGenMipsCubemapCS;

	bool			          m_bReady;

public:
	void init();
	void GenerateFromEquirect(Ptr<CTexture> _EqirectHDR);
	void GenerateFromCubemap(Ptr<CTexture> _Cubemap);
	void GenerateBRDFLut();
	void GenerateEnvCubemapMips();

	void Binding();
	void Clear();

	bool IsReady() const { return m_bReady; }

	Ptr<CTexture> GetEnvCubemap() const { return m_EnvCubemap; }
	Ptr<CTexture> GetIrradianceMap() const { return m_IrradianceMap; }
	Ptr<CTexture> GetPrefilteredMap() const { return m_PrefilterMap; }
	Ptr<CTexture> GetBRDFLut() const { return m_BRDFLutTex; }

};

