#ifndef _YANN_ENGINE_DEFINE_H_
#define _YANN_ENGINE_DEFINE_H_

#include "RHIConfig.h"

#define SINGLE(Type)	private:\
							Type();\
							Type(const Type& _origin) = delete;\
						public:\
							~Type();\
						friend class CSingleton<Type>;

#ifdef USE_DX12
	#define RHI_DEVICE DX12Device::GetInst()
#else
	#define RHI_DEVICE DX11Device::GetInst()
	#define DEVICE DX11Device::GetInst()->GetDevice()
	#define CONTEXT DX11Device::GetInst()->GetContext()
#endif


#define KEY_CHECK(Key, State) (CKeyMgr::GetInst()->GetKeyState(Key) == State)
#define KEY_TAP(Key)		KEY_CHECK(Key, KEY_STATE::TAP)
#define KEY_PRESSED(Key)	KEY_CHECK(Key, KEY_STATE::PRESSED)
#define KEY_RELEASED(Key)	KEY_CHECK(Key, KEY_STATE::RELEASED)
#define KEY_NONE(Key)		KEY_CHECK(Key, KEY_STATE::NONE)


#define CLONE(Type) virtual Type* Clone(){ return new Type(*this); }
#define CLONE_DISABLED(Type) virtual Type* Clone() { return nullptr; }\
							 private:\
							     Type(const Type& _Origin) = delete;

#define DT CTimeMgr::GetInst()->GetDeltaTime()
#define DT_EDITOR CTimeMgr::GetInst()->GetDeltaTimeEditor()

#define MAX_LAYER 32

inline constexpr float ToRadians(float fDegrees) { return fDegrees * (XM_PI / 180.0f); }
inline constexpr float ToDegrees(float fRadians) { return fRadians * (180.0f / XM_PI); }

typedef Vector2 Vec2;
typedef Vector3 Vec3;
typedef Vector4 Vec4;

#define SHADOWMAP_RESOLUTION_LOW 1024
#define SHADOWMAP_RESOLUTION_MEDIUM 2048
#define SHADOWMAP_RESOLUTION_HIGH 4096


#endif
