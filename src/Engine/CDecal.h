#pragma once

#include "CRenderComponent.h"

class CDecal :
    public CRenderComponent
{
private:
	Ptr<CTexture> m_DecalTexture;
    bool          m_bAsEmissive;
	float         m_EmissiveIntensity;

public:
	void SetDecalTexture(Ptr<CTexture> _DecalTexture) { m_DecalTexture = _DecalTexture; }
	void SetAsEmissive(bool _bAsEmissive) { m_bAsEmissive = _bAsEmissive; }
	void SetEmissiveIntensity(float _EmissiveIntensity)
	{
		m_EmissiveIntensity = _EmissiveIntensity;

		if (m_EmissiveIntensity < 0.f)
			m_EmissiveIntensity = 0.f;

		if (1.f < m_EmissiveIntensity)
			m_EmissiveIntensity = 1.f;
	}

public:
    virtual void render() override;
	virtual void finaltick() override;

    CLONE(CDecal);
public:
    CDecal();
    ~CDecal();
};

