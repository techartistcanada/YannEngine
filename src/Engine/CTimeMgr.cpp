#include "pch.h"
#include "CTimeMgr.h"
#include "CEngine.h"
#include "CLevelMgr.h"
#include "CLevel.h"

CTimeMgr::CTimeMgr()
	: m_llFrequency{}
	, m_llCurCount{}
	, m_llPrevCount{}
	, m_DT(0.f)
	, m_EditorDT(0.f)
	, m_AccTime(0.f)
	, m_Time(0.f)
	, m_EditorTime(0.f)
	, m_FrameCount(0)
	, m_FPS(0)
{
}

CTimeMgr::~CTimeMgr()
{
}
void CTimeMgr::init()
{
	QueryPerformanceFrequency(&m_llFrequency);
	QueryPerformanceCounter(&m_llCurCount);
	m_llPrevCount = m_llCurCount;
}

void CTimeMgr::tick()
{
	QueryPerformanceCounter(&m_llCurCount);
	m_EditorDT = m_DT = (float)(m_llCurCount.QuadPart - m_llPrevCount.QuadPart) / (float)m_llFrequency.QuadPart;

	CLevel* pCurLevel = CLevelMgr::GetInst()->GetCurrentLevel();
	if (nullptr == pCurLevel || LEVEL_STATE::PLAY != pCurLevel->GetState())
	{
		m_DT = 0.f;
	}

	m_llPrevCount = m_llCurCount;
	
	m_Time += m_DT;
	m_EditorTime += m_EditorDT;

	// 每隔一秒钟更新一次窗口标题栏中的FPS和DeltaTime信息
	// TODO: m_DT and m_EditorDT
	m_AccTime += m_EditorDT;
	++m_FrameCount;
	if (1.f < m_AccTime)
	{
		m_FPS = m_FrameCount;

		HWND hMainWnd = CEngine::GetInst()->GetMainWnd();
		wchar_t szText[255] = {};
		swprintf_s(szText, L"FPS: %d, DeltaTime: %f",m_FrameCount, m_DT);
		SetWindowText(hMainWnd, szText);

		m_FrameCount = 0;
		m_AccTime -= 1.f;
	}

	g_GlobalData.DeltaTime = m_DT;
	g_GlobalData.Time = m_Time;

	g_GlobalData.EditorDeltaTime = m_EditorDT;
	g_GlobalData.EditorTime = m_EditorTime;
}
