#ifndef _COMPUTE
#define _COMPUTE

#include "values.fx"

RWTexture2D<float4> TargetTex : register(u0);

[numthreads(32,32,1)]
void CS_Test(int3 _ThreadID : SV_DispatchThreadID)
{
    TargetTex[_ThreadID.xy] = g_vec4_0;
}
#endif
