#pragma once
#include "CEntity.h"

struct tAnim2DFrame
{
    Vec2 LeftTop; 
    Vec2 SliceSize;
    Vec2 Offset; // per frame, not for all frames
    float Duration;

    int padding[3]; // 4*2 + 4*2 + 4 + 4*3 = 32 bytes
};

class CTexture;
class CAnimator2D;

class CAnim2D :
    public CEntity
{
private:
    CAnimator2D*         m_Animator;
    vector<tAnim2DFrame> m_vecFrames;
    Vec2                 m_BackgroundSize; // 尺寸最大的一帧动画有多大(比如俯冲，挥动武器)
    int                  m_CurIdx;
    bool                 m_Finish;

    float                m_Time;

    Ptr<CTexture>        m_AtlasTex;
public:
    bool IsFinish() { return m_Finish; }
    void Reset() { m_CurIdx = 0; m_Time = 0.f; m_Finish = false; }
public:
	void Create(Ptr<CTexture> _AtlasTex, Vec2 _LeftTopPixelPos, Vec2 _SlicePixelSize, Vec2 _BackgroundSize, int _FrameCount, UINT _FPS);

    void Save(const wstring& _RelativeFolderPath);
    void Load(const wstring& _RelativePath);

	void SaveToLevelFile(FILE* _File);
	void LoadFromLevelFile(FILE* _File);
public:
    void finaltick();
    void Binding();
    static void Clear();

    CLONE(CAnim2D);
public:
    CAnim2D();
	CAnim2D(const CAnim2D& _Other);
    ~CAnim2D();

    friend class CAnimator2D;
};

