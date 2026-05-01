#include "pch.h"
#include "CKeyMgr.h"

#include "CEngine.h"

UINT g_KeyValues[(UINT)KEY::KEY_END]
=
{
    'W',
    'S',
    'A',
    'D',
    'Z',
    'X',
    'C',
    'V',
    'R',
    'T',
    'Y',
    'U',
    'I',
    'O',
    'P',
    'J',
    'K',
    'L',

    '_0',
    '_1',
    '_2',
    '_3',
    '_4',
    '_5',
    '_6',
    '_7',
    '_8',
    '_9',

    VK_NUMPAD0,
    VK_NUMPAD1,
    VK_NUMPAD2,
    VK_NUMPAD3,
    VK_NUMPAD4,
    VK_NUMPAD5,
    VK_NUMPAD6,
    VK_NUMPAD7,
    VK_NUMPAD8,
    VK_NUMPAD9,

    VK_UP,
    VK_DOWN,
    VK_LEFT,
    VK_RIGHT,

    VK_LBUTTON,
    VK_RBUTTON,

    VK_RETURN,
    VK_ESCAPE,
    VK_SPACE,
    VK_LSHIFT,
    VK_MENU,
    VK_CONTROL,
};

CKeyMgr::CKeyMgr()
{
}

CKeyMgr::~CKeyMgr()
{
}

void CKeyMgr::init()
{
	for (UINT i = 0; i < (UINT)KEY::KEY_END; ++i)
	{
		tKeyInfo info = {};
		info.State = KEY_STATE::NONE;
		info.PrevPressed = false;
		m_vecKeys.push_back(info);
	}
}

void CKeyMgr::tick()
{
	// ==========================================
	// 这里是处理按键状态的更新
	// ==========================================
    for (size_t i = 0; i < m_vecKeys.size(); ++i)
    {
        if (GetAsyncKeyState(g_KeyValues[i]) & 0x8001)
        {
            if (!m_vecKeys[i].PrevPressed)
            {
                m_vecKeys[i].State = KEY_STATE::TAP;
            }
            else
            {
                m_vecKeys[i].State = KEY_STATE::PRESSED;
            }

            m_vecKeys[i].PrevPressed = true;
        }
        else
        {
            if (m_vecKeys[i].PrevPressed) // 上一帧被按过，这一帧没有按住
            {
                m_vecKeys[i].State = KEY_STATE::RELEASED;
            }
            else
            {
                m_vecKeys[i].State = KEY_STATE::NONE;
            }

            m_vecKeys[i].PrevPressed = false;
        }
    }
	// ==========================================
	// 这里可以添加鼠标位置的更新逻辑
	// ==========================================
	m_vPrevMousePos = m_vCurMousePos;

    POINT ptMouse = {};
    GetCursorPos(&ptMouse);
	ScreenToClient(CEngine::GetInst()->GetMainWnd(), &ptMouse);
	m_vCurMousePos = Vec2((float)ptMouse.x, (float)ptMouse.y);
	m_DragDir = m_vCurMousePos - m_vPrevMousePos;
    m_DragDir *= -1.f;
    //m_DragDir.Normalize();
}
