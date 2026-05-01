#include "pch.h"
#include "CAnimator2D.h"
#include "CAnim2D.h"


CAnimator2D::CAnimator2D()
	: CComponent(COMPONENT_TYPE::ANIMATOR2D)
	, m_CurAnim(nullptr)
	, m_Loop(false)
{
}

CAnimator2D::CAnimator2D(const CAnimator2D& _Other)
	: CComponent(_Other)
	, m_CurAnim(nullptr)
	, m_Loop(_Other.m_Loop)
{
	for (const auto& pair : _Other.m_mapAnim)
	{
		CAnim2D* pCloneAnim = pair.second->Clone();
		pCloneAnim->m_Animator = this;
		m_mapAnim.insert(make_pair(pair.first, pCloneAnim));
	}
	if (nullptr != _Other.m_CurAnim)
	{
		m_CurAnim = FindAnimation(_Other.m_CurAnim->GetName());
	}
}

CAnimator2D::~CAnimator2D()
{
	Safe_Del_Map(m_mapAnim);
}

void CAnimator2D::CreateAnimation(const wstring& _AnimName, Ptr<CTexture> _AtlasTex, Vec2 _LeftTopPixelPos, Vec2 _SlicePixelSize, Vec2 _BackgroundSize, int _FrameCount, UINT _FPS)
{
	assert(!FindAnimation(_AnimName));

	CAnim2D* pAnim = new CAnim2D;
	pAnim->SetName(_AnimName);

	pAnim->Create(_AtlasTex, _LeftTopPixelPos, _SlicePixelSize, _BackgroundSize, _FrameCount, _FPS);

	pAnim->m_Animator = this;
	m_mapAnim.insert(make_pair(_AnimName, pAnim));
}

CAnim2D* CAnimator2D::FindAnimation(const wstring& _AnimName)
{
	map<wstring, CAnim2D*>::iterator iter = m_mapAnim.find(_AnimName);
	if (iter == m_mapAnim.end())
		return nullptr;

	return iter->second;
}

void CAnimator2D::LoadAnimation(const wstring& _strRelativePath)
{
	CAnim2D* pAnim = new CAnim2D;
	pAnim->Load(_strRelativePath);

	pAnim->m_Animator = this;
	m_mapAnim.insert(make_pair(pAnim->GetName(), pAnim));
}

void CAnimator2D::Play(const wstring& _strAnimName, bool _Loop)
{
	m_CurAnim = FindAnimation(_strAnimName);
	m_Loop = _Loop;
}

void CAnimator2D::finaltick()
{
	if (nullptr == m_CurAnim)
		return;
	if (m_Loop && m_CurAnim->IsFinish())
	{
		m_CurAnim->Reset();
	}
	m_CurAnim->finaltick();
}

void CAnimator2D::Binding()
{
	if(nullptr != m_CurAnim)
		m_CurAnim->Binding();
}

void CAnimator2D::SaveToLevelFile(FILE* _File)
{
	// 1. save number of animations
	size_t NumAnims = m_mapAnim.size();
	fwrite(&NumAnims, sizeof(size_t), 1, _File);

	// 2. write current animation name
	wstring strCurAnimName = (nullptr == m_CurAnim) ? L"" : m_CurAnim->GetName();
	SaveWString(strCurAnimName, _File);

	// 3. loop?
	fwrite(&m_Loop, sizeof(bool), 1, _File);

	// 4. save anims
	for (const auto& pair : m_mapAnim)
	{
		pair.second->SaveToLevelFile(_File);
	}
}

void CAnimator2D::LoadFromLevelFile(FILE* _File)
{
	// 1. read number of animations
	size_t NumAnims = 0;
	fread(&NumAnims, sizeof(size_t), 1, _File);

	// 2. read current animation name
	wstring strCurAnimName;
	LoadWString(strCurAnimName, _File);
	// 3. loop?
	fread(&m_Loop, sizeof(bool), 1, _File);

	// 4. load anims
	for (size_t i = 0; i < NumAnims; ++i)
	{
		CAnim2D* pAnim = new CAnim2D;
		pAnim->LoadFromLevelFile(_File);
		pAnim->m_Animator = this;
		m_mapAnim.insert(make_pair(pAnim->GetName(), pAnim));
	}

	// 5. set current animation
	m_CurAnim = FindAnimation(strCurAnimName);

}
