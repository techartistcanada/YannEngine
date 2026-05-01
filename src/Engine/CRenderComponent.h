#pragma once
#include "CComponent.h"

#include "assets.h"

class CRenderComponent :
    public CComponent
{
private:
    Ptr<CMesh>                   m_Mesh;

	vector<Ptr<CMaterial>>       m_vecCurMaterials;
	vector<Ptr<CMaterial>>       m_vecSharedMaterials; // 资源管理器里的共享材质
	vector<Ptr<CMaterial>>       m_vecDynamicMaterials; //  动态材质,为本对象复制出来的“可改”材质

    bool                         m_bFrustumCheck;
	bool                         m_bCastDynamicShadow; // for 3D light shadowmap rendering

public:
    virtual void render() = 0;
    virtual void render_shadowmap();

public:
    void SetMesh(Ptr<CMesh> _Mesh);
    Ptr<CMesh> GetMesh() { return m_Mesh; }

	// Single Material(backward compatibility)
    void SetMaterial(Ptr<CMaterial> _Material) { SetMaterial(_Material, 0); }
    Ptr<CMaterial> GetMaterial() { return GetMaterial(0); }
    Ptr<CMaterial> GetDynamicMaterial() { return GetDynamicMaterial(0); }


    // multi materials
    void SetMaterial(Ptr<CMaterial> _Material, UINT _Slot);
    Ptr<CMaterial> GetMaterial(UINT _Slot);
    Ptr<CMaterial> GetDynamicMaterial(UINT _Slot);
    UINT GetMaterialCount() const { return (UINT)m_vecCurMaterials.size(); }

    void SetFrustumCheck(bool _Check) { m_bFrustumCheck = _Check; }
	bool IsFrustumCheck() { return m_bFrustumCheck; }
    void RestoreMaterial();
	void SetCastDynamicShadow(bool _bCast) { m_bCastDynamicShadow = _bCast; }
	bool IsCastDynamicShadow() { return m_bCastDynamicShadow; }

	virtual void SaveToLevelFile(FILE* _File);
	virtual void LoadFromLevelFile(FILE* _File);
public:
    CRenderComponent(COMPONENT_TYPE _Type);
    CRenderComponent(const CRenderComponent& _Other);
    ~CRenderComponent();
};

