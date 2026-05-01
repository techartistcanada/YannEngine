#pragma once
#include "CRenderComponent.h"
class CMeshRenderer :
    public CRenderComponent
{
public:
    virtual void finaltick() override;
    virtual void render() override;

    CLONE(CMeshRenderer);
public:
    CMeshRenderer();
    ~CMeshRenderer();
};

