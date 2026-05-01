#pragma once
class CTimeMgr : public CSingleton<CTimeMgr>
{
	SINGLE(CTimeMgr)
private:
	LARGE_INTEGER m_llFrequency;
	LARGE_INTEGER m_llCurCount; // 计算机已经运行的时间
	LARGE_INTEGER m_llPrevCount;

	float m_DT; // 每一帧的时间
	float m_EditorDT;
	float m_Time; // 游戏已经运行的时间
	float m_EditorTime;
	float m_AccTime;
	UINT  m_FrameCount;
	UINT  m_FPS;
public:
	void init();
	void tick();

public:
	float GetDeltaTime()
	{
		return m_DT;
	}
	float GetDeltaTimeEditor()
	{
		return m_EditorDT;
	}
	UINT GetFPS()
	{
		return m_FPS;
	}
};

