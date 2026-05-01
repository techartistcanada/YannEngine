#ifndef _PARTICLE_TICK
#define _PARTICLE_TICK

#include "values.fx"
#include "func.fx"

RWStructuredBuffer<tParticle>     g_ParticleBuffer : register(u0);
RWStructuredBuffer<tSpawnCount>   g_SpawnCountBuffer : register(u1);

Texture2D                         NoiseTex : register(t20);
StructuredBuffer<tParticleModule> ModuleBuffer : register(t21);


#define ParticleBufferSize g_int_0
#define ParticleObjectPos  g_vec4_0
#define Particle g_ParticleBuffer[_ID.x]
#define SpawnCount g_SpawnCountBuffer[0].spawnCount

// modules check
#define SpawnModule       ModuleBuffer[0].ModuleOnOff[0]
#define SpawnBurstModule  ModuleBuffer[0].ModuleOnOff[1]
#define AddVelocityModule ModuleBuffer[0].ModuleOnOff[2]


[numthreads(32, 1, 1)]
void CS_ParticleTick(int3 _ID : SV_DispatchThreadID)
{
    if(ParticleBufferSize <= _ID.x)
    {
        return;
    }

    if(0 == Particle.Active)
    {
        // ------------------
        // Spawn Module Logic
        // ------------------
        if (SpawnModule || SpawnBurstModule)
        {
            int CurSpawnCount = SpawnCount; // CurSpawnCount为此线程以为的当前的SpawnCount值
            while (0 < CurSpawnCount)
            {
                // 只要此线程以为还有粒子需要被激活，就尝试将SpawnCount减1
                int OriginalValue = 0; // originalValue的意思是在执行写入操作前, SpawnCount的值是多少, 比如它以为SpawnCount是5(CurSpawnCount=5)，
                // 但是如果在它执行写入操作前，另一个线程已经将SpawnCount改成了4(OriginalValue == 4)，
                // 那么将不会进行写入操作(并返回spawncount的真实值到originalValue)
                // 只有当CurSpawnCount == SpawnCount时，才会进行写入操作
                InterlockedCompareExchange(SpawnCount, CurSpawnCount, CurSpawnCount - 1, OriginalValue);
                if (CurSpawnCount == OriginalValue)
                {
                    // 只有当InterlockedCompareExchange成功时，才会执行到这里(也就是说线程以为的SpawnCount值（CurSpawnCount）和实际的SpawnCount值（OriginalValue）相等)
                    float3 vRandom = GetRandom(NoiseTex, 2.f * ((float) _ID.x / (float)(ParticleBufferSize - 1)));
                    float3 vRandom1 = GetRandom(NoiseTex, (float) (_ID.x + 1) / (float) (ParticleBufferSize - 1));
                    float3 vRandom2 = GetRandom(NoiseTex, (float) (_ID.x + 2) / (float) (ParticleBufferSize - 1));

                    float3 vSpawnPos = (float3) 0.f;

                    // 0: 立方体 1: 球体
                    if (ModuleBuffer[0].SpawnShape == 0) // Cube
                    {
                        vSpawnPos.x = vRandom.x * ModuleBuffer[0].SpawnShapeScale.x - (ModuleBuffer[0].SpawnShapeScale.x / 2.f);
                        vSpawnPos.y = vRandom.y * ModuleBuffer[0].SpawnShapeScale.y - (ModuleBuffer[0].SpawnShapeScale.y / 2.f);
                        vSpawnPos.z = vRandom.z * ModuleBuffer[0].SpawnShapeScale.z - (ModuleBuffer[0].SpawnShapeScale.z / 2.f);
                    }
                    else if (ModuleBuffer[0].SpawnShape == 1) // Sphere
                    {
                        float fRadius = ModuleBuffer[0].SpawnShapeScale.x;
                        float fBlockingRadius = ModuleBuffer[0].BlockingSpawnShapeScale.x;
                        float fDiffRadius = fRadius - fBlockingRadius;

                        vSpawnPos = normalize(vRandom1 - 0.5f) * fDiffRadius * vRandom2.x + normalize(vRandom1 - 0.5f) * fBlockingRadius;

                        
                    }
                    // --------------------------
                    // Add Velocity Module Logic
                    // --------------------------
                    Particle.vVelocity = (float3) 0.f;
                    if(AddVelocityModule)
                    {
                        float fSpeed = ModuleBuffer[0].AddVelocityMinSpeed + (ModuleBuffer[0].AddVelocityMaxSpeed - ModuleBuffer[0].AddVelocityMinSpeed) * vRandom.x;
                        // Random
                        if(0 == ModuleBuffer[0].AddVelocityType) 
                        {
                            Particle.vVelocity = normalize(vRandom2 - 0.5f) * fSpeed;
                        }
                        // From Center
                        else if(1 == ModuleBuffer[0].AddVelocityType) 
                        {
                            Particle.vVelocity = normalize(vSpawnPos) * fSpeed;
                        }
                        // To Center
                        else if(2 == ModuleBuffer[0].AddVelocityType) 
                        {
                            Particle.vVelocity = normalize(-vSpawnPos) * fSpeed;
                        }
                        // Fixed
                        else
                        {
                            Particle.vVelocity = normalize(ModuleBuffer[0].AddVelocityFixedDir) * fSpeed;
                        }
                    }

                    Particle.vLocalPos = vSpawnPos;
                    Particle.vWorldPos = Particle.vLocalPos + ParticleObjectPos.xyz;
                    Particle.vWorldScale = lerp(ModuleBuffer[0].vSpawnMinScale.xyz, ModuleBuffer[0].vSpawnMaxScale.xyz, vRandom.x);
                    Particle.vColor = ModuleBuffer[0].vSpawnColor;

                    Particle.Age = 0.f;
                    Particle.Life = lerp(ModuleBuffer[0].minLife, ModuleBuffer[0].maxLife, vRandom.y);
                    Particle.Active = 1;

                    break;
                }
                // 线程会重新以当前的SpawnCount值(4)为CurSpawnCount继续尝试
                CurSpawnCount = SpawnCount;
            }

        }
    }
    else
    {
        Particle.Age += DeltaTime;

        Particle.vWorldPos += Particle.vVelocity * DeltaTime;

        if(Particle.Life <= Particle.Age)
        {
            Particle.Active = 0;
        }
        //Particle.vWorldPos.y += DeltaTime * 100.f;
    }
}

#endif