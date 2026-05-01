#pragma once
#include "Singleton.h"
#include "CTexture.h"

enum class RENDER_MODE
{
    PLAY,
    EDITOR,
};

class CCamera;
class CLight2D;
class CLight3D;
class CStructuredBuffer;
class CRenderTargetSet;

class CRenderMgr :
    public CSingleton<CRenderMgr>
{
    SINGLE(CRenderMgr)
private:
    vector<CCamera*>   m_vecCams;
	CCamera*           m_EditorCam;
	void(CRenderMgr::* RenderFunc)(void);

    vector<CLight2D*>  m_vecLights2D;
    CStructuredBuffer* m_Light2DBuffer;

	vector<CLight3D*>  m_vecLights3D;
    CStructuredBuffer* m_Light3DBuffer;

	Ptr<CTexture>      m_RenderTargetTexCopy; // for post-process
	CRenderTargetSet*  m_MRT[(UINT)MRT_TYPE::END];

	bool               m_bDeferredDebugViewActive;
	Ptr<CTexture>      m_DebugViewRTTexture;
    
    // TODO: move this to dgbredermgr
	bool 		       m_bShowBoundingBox;
	UINT               m_NumDrawCalls;

	// TODO: this is for submesh with different material, i'm wondering if there's a better way to do this
	SHADER_DOMAIN	   m_CurRenderDomain = SHADER_DOMAIN::DOMAIN_DEFFERED;

public:
	void Resize(UINT _Width, UINT _Height);
    void CopyRenderTarget();
    void RegisterCamera(CCamera* _Cam, int _Priority);
    void RegisterEditorCamera(CCamera* _Cam){ m_EditorCam = _Cam; }

	void SetCurRenderDomain(SHADER_DOMAIN _Domain) { m_CurRenderDomain = _Domain; }
	SHADER_DOMAIN GetCurRenderDomain() { return m_CurRenderDomain; }

	void IncrDrawCallCount() { m_NumDrawCalls++; }
	void SetShowBoundingBox(bool _bShow) { m_bShowBoundingBox = _bShow; }
	bool IsShowBoundingBox() { return m_bShowBoundingBox; }
	void SetDeferredDebugView(bool _bOutput, Ptr<CTexture> _OutputTargetTexture) { m_bDeferredDebugViewActive = _bOutput; m_DebugViewRTTexture = _OutputTargetTexture; }
	bool IsDeferredDebugView() { return m_bDeferredDebugViewActive; }
	Ptr<CTexture> GetDebugViewRTTexture() { return m_DebugViewRTTexture; }

    int RegisterLight2D(CLight2D* _Light2D)
    {
        m_vecLights2D.push_back(_Light2D);
        return (int)m_vecLights2D.size() - 1;
    }
    int RegisterLight3D(CLight3D* _Light3D)
    {
        m_vecLights3D.push_back(_Light3D);
        return (int)m_vecLights3D.size() - 1;
	}
public:
	vector<CCamera*>& GetRegisteredCameras() { return m_vecCams; }
public:
    void init();
    void tick();
    void render();
	// TODO: move this to dgbredermgr
	void display_numdrawcalls();
    void display_fps();
    void display_camerapos();

    void ChangeRenderMode(RENDER_MODE _Mode)
    { 
		RENDER_MODE::PLAY == _Mode ? RenderFunc = &CRenderMgr::render_play : RenderFunc = &CRenderMgr::render_editor;
    }

private:
    void render_play();
	void render_editor();
	void render_shadowmap();
    void render_ssao();

private:
    void DataBinding();
    void DataClear();
	void CreateRenderTargetSet();
	void ClearRenderTargetSet();
};

