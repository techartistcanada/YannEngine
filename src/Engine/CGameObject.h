#pragma once
#include "CEntity.h"

class CComponent;
class CRenderComponent;
class CScript;

#define GET_COMPONENT(Type, TYPE) class C##Type* Type() { return (C##Type*)m_arrComponents[(UINT)COMPONENT_TYPE::TYPE];} 

class CGameObject :
    public CEntity
{
private:
    CComponent*             m_arrComponents[(UINT)COMPONENT_TYPE::END];
    CRenderComponent*       m_RenderComponent;

    vector<CScript*>        m_vecScripts;

	CGameObject*            m_Parent;
	vector<CGameObject*>    m_vecChildren;

    int                     m_LayerIdx;
    bool                    m_IsDead;


public:
    void begin();
    void tick();
    virtual void finaltick();
    void render();

public:
    void AddComponent(CComponent* _Component);
    CComponent* GetComponent(COMPONENT_TYPE _Type) { return m_arrComponents[(UINT)_Type]; }
	CRenderComponent* GetRenderComponent() { return m_RenderComponent; }
	int GetLayerIdx() { return m_LayerIdx; }

    GET_COMPONENT(Transform, TRANSFORM);
    GET_COMPONENT(MeshRenderer, MESHRENDERER);
    GET_COMPONENT(Camera, CAMERA);
    GET_COMPONENT(Collider2D, COLLIDER2D);
    GET_COMPONENT(Animator2D, ANIMATOR2D);
    GET_COMPONENT(Light2D, LIGHT2D);
    GET_COMPONENT(Light3D, LIGHT3D);
	GET_COMPONENT(ParticleSystem, PARTICLESYSTEM);
	GET_COMPONENT(SkyBox, SKYBOX);
	GET_COMPONENT(Decal, DECAL);
	GET_COMPONENT(BoundingBox, BOUNDINGBOX);


	void AddChild(CGameObject* _Child);
    void RegisterAsParentObjectInCurrentLayer();
    void DetachFromParent();
	const vector<CGameObject*>& GetChildren() { return m_vecChildren; }
	CGameObject* GetParent() { return m_Parent; }
	bool IsAncestorOf(CGameObject* _Object);
    bool IsDead() { return m_IsDead; }
    void Destroy();

	const vector<CScript*>& GetScripts() { return m_vecScripts; }

    template<typename T>
    T* GetScript();
private:
	void SetLayerIdx(int _Idx) { m_LayerIdx = _Idx; }

public:
    CLONE(CGameObject)
    CGameObject();
	CGameObject(const CGameObject& _Origin);
    ~CGameObject();

    friend class CLayer;
    friend class CTaskMgr;
};

template<typename T>
inline T* CGameObject::GetScript()
{
    for (size_t i = 0; i < m_vecScripts.size(); ++i)
    {
        if (dynamic_cast<T*>(m_vecScripts[i]))
            return  (T*)m_vecScripts[i];
    }
    return nullptr;
}
