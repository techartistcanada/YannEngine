#pragma once

#include "CComponent.h"

#include "ptr.h"
#include "CMesh.h"
#include "CMaterial.h"

class CRenderTargetSet;

class CLight3D :
    public CComponent
{
private:
    tLightInfo        m_Info;
	int               m_LightIdx; // 在finaltick中由渲染管理器设置
	bool              m_bDrawGizmo;
	bool              m_bRenderShadow;

    Ptr<CMesh>        m_VolumeMesh;
	Ptr<CMaterial>    m_DeferredLightingMaterial; // 用于延迟渲染的材质

    // TODO: think of a better way to store the camera
    CGameObject*      m_LightCamObj;
    CRenderTargetSet* m_ShadowMapMRT;
    Matrix            m_matLightProjSnapped;



public:
    virtual void finaltick();
    
    void RenderShadowMap();
    void ApplyLighting();
private:
    void Binding();
public:
    void SetLightType(LIGHT_TYPE _Type);
    void SetDiffuse(Vec3 _Diffuse) { m_Info.Light.vDiffuse = _Diffuse; }
    void SetAmbient(Vec3 _Ambient) { m_Info.Light.vAmbient = _Ambient; }
    void SetSpecular(Vec3 _Specular) { m_Info.Light.vMaxSpecular = _Specular; }

    
    void SetRange(float _Range);
    void SetAngle(float _Angle) { m_Info.Angle = _Angle; }
    
	void SetDrawGizmo(bool _bDraw) { m_bDrawGizmo = _bDraw; }
	bool GetDrawGizmo() const { return m_bDrawGizmo; }
    
    void SetIsRenderShadow(bool _bRender) { m_bRenderShadow = _bRender; }
    bool GetIsRenderShadow() const { return m_bRenderShadow; }

	const tLightInfo& GetLightInfo() const { return m_Info; }

	virtual void SaveToLevelFile(FILE* _File);
	virtual void LoadFromLevelFile(FILE* _File);

    CLONE(CLight3D);
public:
    CLight3D();
	CLight3D(const CLight3D& _Origin);
    ~CLight3D();
};

