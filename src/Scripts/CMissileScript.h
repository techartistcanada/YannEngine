#pragma once
#include <CScript.h>
class CMissileScript :
    public CScript
{
private:
    float m_Speed;
public:
    virtual void tick() override;


	CLONE(CMissileScript);
public:
    CMissileScript();
    ~CMissileScript();
};

