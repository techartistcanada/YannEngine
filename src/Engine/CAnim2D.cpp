#include "pch.h"
#include "CAnim2D.h"

#include "CTexture.h"

#ifdef USE_DX11
#include "DX11/DX11Device.h"
#else
#include "DX12/DX12Device.h"
#endif

#include "CConstantBuffer.h"
#include "CTimeMgr.h"
#include "CPathMgr.h"
#include "CAssetMgr.h"

CAnim2D::CAnim2D()
	: m_Animator(nullptr)
	, m_CurIdx(0)
	, m_Finish(false)
	, m_Time(0.f)
{
}

CAnim2D::CAnim2D(const CAnim2D& _Other)
	: CEntity(_Other)
	, m_Animator(nullptr)
	, m_vecFrames(_Other.m_vecFrames)
	, m_BackgroundSize(_Other.m_BackgroundSize)
	, m_CurIdx(_Other.m_CurIdx)
	, m_Finish(_Other.m_Finish)
	, m_Time(_Other.m_Time)
	, m_AtlasTex(_Other.m_AtlasTex)
{
}


CAnim2D::~CAnim2D()
{
}


void CAnim2D::Create(Ptr<CTexture> _AtlasTex, Vec2 _LeftTopPixelPos, Vec2 _SlicePixelSize, Vec2 _BackgroundSize, int _FrameCount, UINT _FPS)
{
	assert(_AtlasTex.Get());

	m_AtlasTex = _AtlasTex;
	m_BackgroundSize.x = _BackgroundSize.x / m_AtlasTex->GetWidth();
	m_BackgroundSize.y = _BackgroundSize.y / m_AtlasTex->GetHeight();
	for (int i = 0; i < _FrameCount; i++)
	{
		tAnim2DFrame frame = {};

		frame.LeftTop.x = (_LeftTopPixelPos.x + _SlicePixelSize.x * i) / m_AtlasTex->GetWidth();
		frame.LeftTop.y = _LeftTopPixelPos.y / m_AtlasTex->GetHeight();
		frame.Duration = 1.f / (float)_FPS;
		frame.SliceSize.x = _SlicePixelSize.x / m_AtlasTex->GetWidth();
		frame.SliceSize.y = _SlicePixelSize.y / m_AtlasTex->GetHeight();
		
		m_vecFrames.push_back(frame);
	}
}


void CAnim2D::finaltick()
{
	if (m_Finish)
		return;

	m_Time += DT;
	if (m_vecFrames[m_CurIdx].Duration < m_Time)
	{
		m_Time = m_Time - m_vecFrames[m_CurIdx].Duration;
		++m_CurIdx;
		if ((int)m_vecFrames.size() <= m_CurIdx)
		{
			m_CurIdx = (int)m_vecFrames.size() - 1;
			m_Finish = true;
		}
	}
}

void CAnim2D::Binding()
{
	if (nullptr != m_AtlasTex)
		m_AtlasTex->Binding(14);

	static CConstantBuffer* pCB = RHI_DEVICE->GetConstantBuffer(CB_TYPE::ANIMATION);
	tAnim2DInfo info = {};
	info.vLeftTop = m_vecFrames[m_CurIdx].LeftTop;
	info.vSliceSize = m_vecFrames[m_CurIdx].SliceSize;
	info.vOffset = m_vecFrames[m_CurIdx].Offset;
	info.vBackground = m_BackgroundSize;
	info.UseAnim2D = 1;

	pCB->SetData(&info);
	pCB->Binding();
}

void CAnim2D::Clear()
{
	static CConstantBuffer* pCB = RHI_DEVICE->GetConstantBuffer(CB_TYPE::ANIMATION);
	pCB->Clear();
}

void CAnim2D::Save(const wstring& _RelativeFolderPath)
{
	wstring strFilePath = CPathMgr::GetInst()->GetContentPath() + _RelativeFolderPath + GetName() + L".anim";

	FILE* pFile = nullptr;
	_wfopen_s(&pFile, strFilePath.c_str(), L"wb");

	if (nullptr == pFile)
	{
		MessageBox(nullptr, L"Failed to save file", L"Failed to save animation file", MB_OK);
		return;
	}
	// ==================
	// 保存动画信息 save anim
	// ==================
	// animation name
	SaveWString(GetName(), pFile);

	// 保存帧数
	size_t numFrames = m_vecFrames.size();
	fwrite(&numFrames, sizeof(size_t), 1, pFile);

	// 保存所有帧信息
	fwrite(m_vecFrames.data(), sizeof(tAnim2DFrame), numFrames, pFile);

	// backgroundsize
	fwrite(&m_BackgroundSize, sizeof(Vec2), 1, pFile);


	// atlas texture
	SaveAssetRef(m_AtlasTex, pFile);
	fclose(pFile);
}

void CAnim2D::Load(const wstring& _RelativePath)
{
	wstring strFilePath = CPathMgr::GetInst()->GetContentPath() + _RelativePath;

	FILE* pFile = nullptr;
	_wfopen_s(&pFile, strFilePath.c_str(), L"rb");

	if (nullptr == pFile)
	{
		MessageBox(nullptr, L"Failed to open file", L"Failed to open animation file", MB_OK);
		return;
	}
	// ==================
	// 读取动画信息 load anim
	// ==================
	// read anim name
	wstring strName;
	LoadWString(strName, pFile);
	SetName(strName);

	// num frames
	size_t numFrames = 0;
	fread(&numFrames, sizeof(size_t), 1, pFile);

	// frames
	m_vecFrames.resize(numFrames);
	fread(m_vecFrames.data(), sizeof(tAnim2DFrame), numFrames, pFile);

	// backgroundsize
	fread(&m_BackgroundSize, sizeof(Vec2), 1, pFile);

	LoadAssetRef(m_AtlasTex, pFile);

	fclose(pFile);
}

void CAnim2D::SaveToLevelFile(FILE* _File)
{
	// 1. save name
	SaveWString(GetName(), _File);

	// 2. save frames data
	size_t NumFrames = m_vecFrames.size();
	fwrite(&NumFrames, sizeof(size_t), 1, _File);
	fwrite(m_vecFrames.data(), sizeof(tAnim2DFrame), m_vecFrames.size(), _File);

	// 3 "background" size
	fwrite(&m_BackgroundSize, sizeof(Vec2), 1, _File);

	// 4. atlas texture
	SaveAssetRef(m_AtlasTex, _File);
}

void CAnim2D::LoadFromLevelFile(FILE* _File)
{
	// 1. load name
	wstring strName;
	LoadWString(strName, _File);
	SetName(strName);

	// 2. load frames data
	size_t NumFrames = 0;
	fread(&NumFrames, sizeof(size_t), 1, _File);
	m_vecFrames.resize(NumFrames);
	fread(m_vecFrames.data(), sizeof(tAnim2DFrame), NumFrames, _File);

	// 3 "background" size
	fread(&m_BackgroundSize, sizeof(Vec2), 1, _File);

	// 4. atlas texture
	LoadAssetRef(m_AtlasTex, _File);
}
