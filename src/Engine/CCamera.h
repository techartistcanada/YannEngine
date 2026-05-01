#pragma once

#include "CComponent.h"
#include "CFrustum.h"




class CCamera :
    public CComponent
{
private:
	CFrustum  m_Frustum;
    PROJ_TYPE m_ProjType;
    int       m_CamPriority;
    Matrix    m_matView;
    Matrix    m_matViewInv;
    Matrix    m_matProj;
    Matrix    m_matProjInv;

    float     m_FOV;
    float     m_Far;
    float     m_AspectRatio;
    float     m_Width;

    float     m_Scale;  // for orthographic projection

    UINT      m_LayerCheck;  // 32 bits (0 - 31 layer)

	vector<CGameObject*> m_vecDeferredObjects;
	vector<CGameObject*> m_vecDeferredDecalObjects;
	vector<CGameObject*> m_vecOpaqueObjects;
	vector<CGameObject*> m_vecMaskedObjects;
	vector<CGameObject*> m_vecTransparentObjects;
	vector<CGameObject*> m_vecParticleObjects;
	vector<CGameObject*> m_vecPostProcessObjects;
    // for shadowmap
	vector<CGameObject*> m_vecShadowMapObjects;
public:
    virtual void begin() override;
    virtual void finaltick() override;
    void render();
public:
    void SetCameraPriority(int _Priority);

    void SetProjType(PROJ_TYPE _Type) { m_ProjType = _Type; }
    void SetFar(float _Far) { m_Far = _Far; }
    void SetFOV(float _FOV) { m_FOV = _FOV; }
    void SetWidth(float _Width) { m_Width = _Width; }
    void SetAspectRatio(float _AR) { m_AspectRatio = _AR; }
    void SetScale(float _Scale) { m_Scale = _Scale; }

    int GetCameraPriority() { return m_CamPriority; }

	const Matrix& GetViewMat() { return m_matView; }
	const Matrix& GetViewMatInv() { return m_matViewInv; }
	const Matrix& GetProjMat() { return m_matProj; }
	const Matrix& GetProjMatInv() { return m_matProjInv; }

    PROJ_TYPE GetProjType() { return m_ProjType; }
    float GetFar() { return m_Far; }
    float GetFOV() { return m_FOV; }
    float GetWidth() { return m_Width; }
    float GetAspectRatio() { return m_AspectRatio; }
    float GetScale() { return m_Scale; }

	void LayerCheckOn(int _LayerIdx);
	void LayerCheckAll() { m_LayerCheck = 0xffffffff; }

	void SortObjects(); 
    void SortObjects_ShadowMap();
    //void ClearSortedObjects();

	virtual void SaveToLevelFile(FILE* _File) override;
	virtual void LoadFromLevelFile(FILE* _File) override;

public:
    // TODO: public?
    void SortClear();
	void render_deferred();
	void render_deferred_decal();
	void render_opaque();
    void render_masked();
	void render_transparent();
	void render_particle();
	void render_postprocess();

	void render_shadowmap();


    CLONE(CCamera);
public:
    CCamera();
	CCamera(const CCamera& _Other);
    ~CCamera();

};

