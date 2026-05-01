#pragma once
#include "CAsset.h"
#include "RHI/IRHIBuffer.h"

class CMesh :
    public CAsset
{
public:
    struct tSubMesh
    {
        UINT IndexStart;
		UINT IndexCount;
        UINT MaterialIndex;
    };

private:
	IRHIBuffer*          m_pVB;
    UINT                 m_VtxCount;
    Vertex*              m_VtxSysMem;

    IRHIBuffer*          m_pIB;
    UINT                 m_IdxCount;
    UINT*                m_IdxSysMem;

	vector<tSubMesh>    m_vecSubMeshes;

    // Bounding sphere
	Vec3                 m_BoundCenter;
	float                m_BoundRadius;
    // AABB half-extents(local space)
	Vec3                 m_BoundHalfExtents;

public:
	UINT GetVertexCount() const { return m_VtxCount; }
	UINT GetIndexCount() const { return m_IdxCount; }
	UINT GetSubMeshCount() const { return (UINT)(m_vecSubMeshes.size()); }
	const tSubMesh& GetSubMesh(UINT _Idx) const { return m_vecSubMeshes[_Idx]; }

	Vec3 GetBoundCenter() const { return m_BoundCenter; }
	float GetBoundRadius() const { return m_BoundRadius; }
	Vec3 GetBoundHalfExtents() const { return m_BoundHalfExtents; }
public:
    int Create(Vertex* _VtxSysMem, size_t _VtxCount, UINT* _IdxSysMem, size_t _IdxCount);
    void render();
	void render_submesh(UINT _SubMeshIdx);
	void render_particle(UINT _InstanceCount);
private:
    virtual int Load(const wstring& _FilePath) override;
    virtual int Save(const wstring& _FilePath) override { return S_OK; }
    void Binding();
public:
    CLONE_DISABLED(CMesh);

public:
    CMesh(bool _bEngineAsset = false);
    ~CMesh();
};

