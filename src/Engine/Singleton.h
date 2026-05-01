#pragma once

template<typename T>
class CSingleton
{
private:
	// 定义一个函数指针
	// 格式为 返回类型 (*类型名)(参数列表)
	typedef void(*DESTROYFunc)(void);
private:
	static T* m_This;

public:
	static T* GetInst()
	{
		if (nullptr == m_This)
		{
			m_This = new T;
		}

		return m_This;
	}

	static void Destroy()
	{
		if (nullptr != m_This)
		{
			delete m_This;
			m_This = nullptr;
		}
	}

protected:
	CSingleton()
	{
		atexit((DESTROYFunc)&CSingleton<T>::Destroy);
	}
	CSingleton(const CSingleton& _other) = delete;
	~CSingleton()
	{
	}
};

template<typename T>
T* CSingleton<T>::m_This = nullptr;
