#pragma once
#include "CRenderComponent.h"

enum SKYBOX_TYPE
{
    SPHERE,
    CUBE,
};

class CSkyBox :
    public CRenderComponent
{
private:
	SKYBOX_TYPE		m_Type;
	Ptr<CTexture>	m_SkyBoxTexture;
	bool            m_bVisible;
	float           m_fRotationY;
	float           m_fExposure;
	// m_fRoughnessOverride < 0 means no override, use roughness from material
	float           m_fRoughnessOverride;

public:
	void SetVisible(bool _bVisible) { m_bVisible = _bVisible; }
	bool IsVisible() const { return m_bVisible; }

	void SetRotationY(float _fRotationY) { m_fRotationY = _fRotationY; }
	float GetRotationY() const { return m_fRotationY; }

	void SetExposure(float _fExposure) { m_fExposure = _fExposure; }
	float GetExposure() const { return m_fExposure; }

	void SetRoughnessOverride(float _fRoughnessOverride) { m_fRoughnessOverride = _fRoughnessOverride; }
	float GetRoughnessOverride() const { return m_fRoughnessOverride; }

	void SetSkyBoxType(SKYBOX_TYPE _Type);
	void SetSkyBoxTexture(Ptr<CTexture> _Texture);
	SKYBOX_TYPE GetSkyBoxType() { return m_Type; }
	Ptr<CTexture> GetSkyBoxTexture() { return m_SkyBoxTexture; }
public:
	virtual void finaltick() override;
	virtual void render() override;
public:
	CLONE(CSkyBox);
	CSkyBox();
	~CSkyBox();
};

