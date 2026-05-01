#pragma once
#include "CMaterial.h" 

class CMaterialInstance :
    public CMaterial
{
private:
    Ptr<CMaterial> m_ParentMaterial;

	// --- Override tracking ---
	tMaterialConst m_OverrideConst;
	bool m_bOverrideScalar[SCALAR_PARAM::SCALAR_END] = {};

	Ptr<CTexture> m_arrOverrideTex[TEX_PARAM::TEX_END];
	bool m_bOverrideTex[TEX_PARAM::TEX_END] = {};

public:
    void SetParentMaterial(Ptr<CMaterial> _Parent) 
    {
		m_ParentMaterial = _Parent;
        // Sync base class shader so GetShader() works for CCamera::SortObjects()
        if (nullptr != _Parent.Get())
            m_Shader = _Parent->GetShader();
        else
            m_Shader = nullptr;
    }
    Ptr<CMaterial> GetParentMaterial() { return m_ParentMaterial; }

    // Override a texture for this instance only
    void SetTexOverride(TEX_PARAM _Param, Ptr<CTexture> _Tex)
    {
        m_arrOverrideTex[_Param] = _Tex;
        m_bOverrideTex[_Param] = true;
    }

    // Override a scalar for this instance only
    template<typename T>
    void SetScalarOverride(SCALAR_PARAM _Param, const T& _Value);

    virtual void Binding() override;

    CLONE(CMaterialInstance);
public:
    CMaterialInstance();
    ~CMaterialInstance();
};

template<typename T>
inline void CMaterialInstance::SetScalarOverride(SCALAR_PARAM _Param, const T& _Value)
{
    // Store override in local const buffer
    void* pDest = nullptr;
    if (_Param >= INT_0 && _Param <= INT_3)
        pDest = &m_OverrideConst.iArr[_Param - INT_0];
    else if (_Param >= FLOAT_0 && _Param <= FLOAT_3)
        pDest = &m_OverrideConst.fArr[_Param - FLOAT_0];
    else if (_Param >= VEC2_0 && _Param <= VEC2_3)
        pDest = &m_OverrideConst.v2Arr[_Param - VEC2_0];
    else if (_Param >= VEC4_0 && _Param <= VEC4_3)
        pDest = &m_OverrideConst.v4Arr[_Param - VEC4_0];
    else if (_Param >= MAT_0 && _Param <= MAT_3)
        pDest = &m_OverrideConst.matArr[_Param - MAT_0];

    if (pDest)
    {
        memcpy(pDest, &_Value, sizeof(T));
        m_bOverrideScalar[_Param] = true;
    }
}