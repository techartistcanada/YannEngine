#pragma once
class CEntity
{
private:
	static UINT g_NextID;

private:
	wstring m_Name;
	UINT m_ID;
public:
	const wstring& GetName() { return m_Name; }
	void SetName(const wstring& _Name) { m_Name = _Name; }
	UINT GetID() { return m_ID; }

	virtual CEntity* Clone() = 0;

public:
	CEntity();
	CEntity(const CEntity& _Orgin);
	// NOTE: 如果析构函数没有定义为“虚函数”，那么删除的时候不会调用子类的析构函数
	virtual ~CEntity();
};

