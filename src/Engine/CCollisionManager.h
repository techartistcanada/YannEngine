#pragma once
#include "Singleton.h"

union COL_ID
{
    struct
    {
        UINT LeftID;
        UINT RightID;
    };

    LONGLONG ID;
};

class CCollider2D;

class CCollisionManager :
    public CSingleton<CCollisionManager>
{
    SINGLE(CCollisionManager)
private:
	UINT m_Matrix[(UINT)MAX_LAYER]; // 碰撞矩阵 32x32 collision matrix 32x32
    map<LONGLONG, bool> m_ColInfo; // 上一帧这一对collider是否重叠
public:
    void tick();
public:
    void LayerCheck(UINT _LayerIdx, UINT _LayerRightIdx);

	void SaveCollisionInfosToLevelFile(FILE* _File);
	void LoadCollisionInfosFromLevelFile(FILE* _File);
private:
	void CollisionBtwLayers(UINT _Left, UINT _Right);
	void CollisionBtwCollider2D(CCollider2D* _LeftCol, CCollider2D* _RightCol);
	bool IsCollision(CCollider2D* _LeftCol, CCollider2D* _RightCol);

};

