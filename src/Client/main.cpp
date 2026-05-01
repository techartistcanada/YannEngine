#include "pch.h"
#include <Windows.h>
#include "Client.h"

#include <CEngine.h>
#include <global.h>

#ifdef USE_DX11
#include <DX11/DX11Device.h>
#else
#include <DX12/DX12Device.h>
#endif

#include <CRenderMgr.h>


/*
#ifdef _DEBUG
#pragma comment(lib, "Engine/Engine_d.lib")
#else
#pragma comment(lib, "Engine/Engine.lib")
#endif

#ifdef _DEBUG
#pragma comment(lib, "Scripts/Scripts_d.lib")
#else
#pragma comment(lib, "Scripts/Scripts.lib")
#endif
*/

//#define GAME_RELEASE

#include "CTestLevel2D.h"
#include "CTestLevel3D.h"
#include "CEditorMgr.h"
#include "CImGuiMgr.h"
#include "ImGui/imgui.h"

// Global Variables:
HINSTANCE hInst;                                // current instance
static bool g_ResizingWindow = false;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance); // ATOM = unsigned short
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{

    // register window class
    MyRegisterClass(hInstance);

    // Perform application initialization:
    hInst = hInstance; // Store instance handle in our global variable

    // 1.创建windows窗口 create a Windows window
    HWND hWnd = CreateWindowW(L"Test",
        L"MyGame",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        0,
        CW_USEDEFAULT,
        0,
        nullptr,
        nullptr,
        hInstance,
        nullptr);

    if (!hWnd)
    {
       return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);


    // 2.创建引擎实例
	// Create cengine instance
    if (FAILED(CEngine::GetInst()->Init(hWnd, Vec2(1280, 720))))
    {
        MessageBox(nullptr, L"Engine failed to init!", L"Fatal", MB_OK);
        return 0;
    }

#ifndef GAME_RELEASE
	// 3.初始化编辑器管理器 EditorManager
    // int EditorManager
	CEditorMgr::GetInst()->init();

	// 4.IMGUI 初始化
    if (FAILED(CImGuiMgr::GetInst()->init(hWnd)))
    {
        MessageBox(nullptr, L"ImGui failed to init!", L"Fatal", MB_OK);
		return 0;
    }

	// 5.创建关卡
    CTestLevel3D::CreateTestLevel();
#endif

    // load shortcut key table
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLIENT));

    MSG msg;

	// 主消息循环 Main message loop
    while (true)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
			// 处理退出消息
            if (WM_QUIT == msg.message)
                break;

			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
        }
		// 消息队列为空时,执行游戏逻辑和渲染,因为计算机非常快,所以大部分时间是没有消息的。
        else
        {
            if (!RHI_DEVICE->IsResizing())
            {
                CEngine::GetInst()->Progress();
#ifndef GAME_RELEASE
                CEditorMgr::GetInst()->tick();
                CImGuiMgr::GetInst()->tick();
                CAssetMgr::GetInst()->ClearAssetsChangedFlag();
#endif
                RHI_DEVICE->Present();
            }

        }
    }
    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLIENT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = nullptr; // MAKEINTRESOURCEW(IDC_CLIENT);
    wcex.lpszClassName  = L"Test";
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return true;

    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
        // 窗口开始调整大小
    case WM_ENTERSIZEMOVE:
        g_ResizingWindow = true;
        break;
    case WM_EXITSIZEMOVE:
        g_ResizingWindow = false;
        {
            RECT rect;
            GetClientRect(hWnd, &rect);
            UINT width = rect.right - rect.left;
            UINT height = rect.bottom - rect.top;
            if (width > 0 && height > 0)
            {
                // windows resize
                Vec2 curRes = RHI_DEVICE->GetRenderResolution();
                if ((UINT)curRes.x != width || (UINT)curRes.y != height)
                {
					CRenderMgr::GetInst()->Resize(width, height);
                }
            }
        }
        break;
    case WM_SIZE:
	{
		if (wParam == SIZE_MINIMIZED)
		{
			return 0;
		}
		// 如果不是用户拖拽调整大小（那种情况由 WM_EXITSIZEMOVE 处理）
		if (!g_ResizingWindow)
		{
			UINT width = LOWORD(lParam);
			UINT height = HIWORD(lParam);
            Vec2 curRes = RHI_DEVICE->GetRenderResolution();
			if ((UINT)curRes.x != width || (UINT)curRes.y != height)
			{
				CRenderMgr::GetInst()->Resize(width, height);
			}
		}
	}
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_DPICHANGED:
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
        {
            const RECT* suggested_rect = (RECT*)lParam;
            SetWindowPos(hWnd, NULL, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
        }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
