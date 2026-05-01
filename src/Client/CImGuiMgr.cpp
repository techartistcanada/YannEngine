#include "pch.h"
#include "CImGuiMgr.h"
#include "ImGui/IconsFontAwesome6.h"
#include <CPathMgr.h>


#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"

#ifdef USE_DX11
#include "ImGui/imgui_impl_dx11.h"
#include <DX11/DX11Device.h>
#else
#include "ImGui/imgui_impl_dx12.h"
#include <DX12/DX12Device.h>
#include <DX12/DX12CommandList.h>
#endif

#include "InspectorUI.h"
#include "ListUI.h"
#include "ContentUI.h"
#include "OutlinerUI.h"
#include "ParameterUI.h"
#include "MainMenuBarUI.h"


CImGuiMgr::CImGuiMgr()
	: m_hMainWnd(nullptr)
	, m_bShowDemo(false)
{
}

CImGuiMgr::~CImGuiMgr()
{
#ifdef USE_DX11
    ImGui_ImplDX11_Shutdown();
#else
	ImGui_ImplDX12_Shutdown();
#endif
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    // Delete EditorUI
    Safe_Del_Map(m_mapUI);
}

int CImGuiMgr::init(HWND _hwnd)
{
	m_hMainWnd = _hwnd;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
#ifdef USE_DX11
    // TODO: dx12 viewports
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
#endif

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
		ApplyEditorDarkTheme();
    }

    // Setup Platform/Renderer backends
    if (!ImGui_ImplWin32_Init(m_hMainWnd))
        return E_FAIL;

#ifdef USE_DX11
    if (!ImGui_ImplDX11_Init(DEVICE, CONTEXT))
        return E_FAIL;
#else
        // ==================================================
    // DX12 ImGui 初始化
    // ==================================================
    {
        // 创建 GPU-visible SRV 描述符堆（由 DX12Device 管理）
        // Slot 0 = ImGui font texture, 其余 slot 给用户纹理
        if (FAILED(DX12Device::GetInst()->InitImGuiSrvHeap()))
            return E_FAIL;
    }

    if (!ImGui_ImplDX12_Init(
            DX12Device::GetInst()->GetD3D12Device(),
            3,                              // BufferCount = 3 (triple buffering)
            DXGI_FORMAT_R8G8B8A8_UNORM,     // swap chain back buffer format
            DX12Device::GetInst()->GetImGuiSrvHeap(),
            DX12Device::GetInst()->GetImGuiSrvCpuHandle(0),
            DX12Device::GetInst()->GetImGuiSrvGpuHandle(0)))
    {
        return E_FAIL;
    }
#endif

    LoadCustomResources();
    CreateEditorUI();

	wstring strContentPath = CPathMgr::GetInst()->GetContentPath();
    m_hNotify = FindFirstChangeNotification(
        strContentPath.c_str(),
        TRUE,
        FILE_NOTIFY_CHANGE_FILE_NAME |
        FILE_NOTIFY_CHANGE_DIR_NAME |
		FILE_ACTION_ADDED |
		FILE_ACTION_REMOVED |
        FILE_NOTIFY_CHANGE_ATTRIBUTES);

	return S_OK;
}

void CImGuiMgr::tick()
{
	// Start the Dear ImGui frame
#ifdef USE_DX11
    ImGui_ImplDX11_NewFrame();
#else
	ImGui_ImplDX12_NewFrame();
#endif
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

    // reset parameterUI id
	ParameterUI::ResetID();

    if(m_bShowDemo)
		ImGui::ShowDemoWindow();

	for (const auto& pair : m_mapUI)
    {
		if (pair.second->IsActive())
        {
            pair.second->tick();
        }
    }

	ImGui::Render();
#ifdef USE_DX11
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#else
    {
        // 确保 back buffer 在 RENDER_TARGET 状态，并将 RTV 绑定到命令列表
        DX12Device::GetInst()->PrepareBackBufferForImGui();

        auto pCmdListWrapper = DX12Device::GetInst()->GetCurrentCommandList();
        if (pCmdListWrapper)
        {
		    ID3D12GraphicsCommandList* pCmdList =
			pCmdListWrapper->GetGraphicsCommandList().Get();

            // ImGui DX12 backend 在 RenderDrawData 时需要绑定自己的 SRV 堆
            ID3D12DescriptorHeap* heaps[] = { DX12Device::GetInst()->GetImGuiSrvHeap() };
            pCmdList->SetDescriptorHeaps(1, heaps);

            ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pCmdList);
        }
    }
#endif

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    ObserveContentFolderChanges();

}


void CImGuiMgr::CreateEditorUI()
{
    EditorUI* pUI = nullptr;

    // Inspector UI
	pUI = new InspectorUI;
	//pUI->SetActive(false);
	pUI->SetDisplayName("Inspector");
	m_mapUI.insert({ pUI->GetDisplayName(), pUI });

    pUI = new ListUI;
    pUI->SetDisplayName("List");
	pUI->SetActive(false);
    m_mapUI.insert({ pUI->GetDisplayName(), pUI });

    // Content UI
	pUI = new ContentUI;
	pUI->SetDisplayName("Content");
	pUI->SetActive(true);
    m_mapUI.insert({ pUI->GetDisplayName(), pUI });

	// Outliner UI
    pUI = new OutlinerUI;
    pUI->SetDisplayName("Outliner");
	pUI->SetActive(true);
    m_mapUI.insert({ pUI->GetDisplayName(), pUI });

	// Menu Bar UI
	pUI = new MainMenuBarUI;
	pUI->SetActive(true);
	m_mapUI.insert({ pUI->GetDisplayName(), pUI });


}

void CImGuiMgr::ObserveContentFolderChanges()
{
	DWORD dwWaitStatus = WaitForSingleObject(m_hNotify, 0);

	if (WAIT_OBJECT_0 == dwWaitStatus)
    {
        // Content folder changed, reload assets
        ContentUI* pContentUI = FindEditorUI<ContentUI>("Content");
        if (pContentUI)
        {
            pContentUI->ReloadContentToRAM();
            pContentUI->UpdateContent();
        }
        // 继续监视
        FindNextChangeNotification(m_hNotify);
    }
}



void CImGuiMgr::LoadCustomResources()
{
    // load icons
	CAssetMgr::GetInst()->Load<CTexture>(L"EditorIcon_Transform", L"EditorAsset\\editoricon_transform.png");
	CAssetMgr::GetInst()->Load<CTexture>(L"EditorIcon_Logo", L"EditorAsset\\editoricon_logo.png");


    // ---------------------------------------
	// Font and Icon Font For Editor UI
    // load editor ui fonts
    // ---------------------------------------
	ImGuiIO& io = ImGui::GetIO();
	wstring contentPath = CPathMgr::GetInst()->GetContentPath();
	wstring fontPath = contentPath + L"EditorAsset\\InterRegular.ttf";
	string fontPathStr(fontPath.begin(), fontPath.end());
	io.Fonts->AddFontFromFileTTF(fontPathStr.c_str(), 14.0f);

    // 合并加载 Font Awesome（Solid）
	wstring faFontPath = contentPath + L"EditorAsset\\fa-solid-900.otf";
	string faFontPathStr(faFontPath.begin(), faFontPath.end());

	// 告诉 ImGui 这是“合并到已有字体”
	ImFontConfig config;
	config.MergeMode = true;
	config.PixelSnapH = true;

	// 只加载图标所在的 Unicode 范围
	static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	io.Fonts->AddFontFromFileTTF(faFontPathStr.c_str(), 14.0f, &config, icon_ranges);
}

// *******************************************************
// 主题配色方案：适合编辑器的暗色风格 ImGui Theme
// *******************************************************
void CImGuiMgr::ApplyEditorDarkTheme()
{
	// 调一次：例如 ImGui 初始化完成后、NewFrame() 之前
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // --- layout / shape ---
    style.WindowPadding     = ImVec2(6, 4);
    style.FramePadding      = ImVec2(6, 3);
    style.ItemSpacing       = ImVec2(6, 3);
    style.ItemInnerSpacing  = ImVec2(4, 3);
    style.ScrollbarSize     = 12.0f;
    style.GrabMinSize       = 8.0f;

    style.WindowRounding    = 8.0f;
    style.ChildRounding     = 8.0f;
    style.FrameRounding     = 6.0f;
    style.PopupRounding     = 8.0f;
    style.ScrollbarRounding = 10.0f;
    style.GrabRounding      = 6.0f;
    style.TabRounding       = 6.0f;

    style.WindowBorderSize  = 1.0f;
    style.FrameBorderSize   = 1.0f;
    style.PopupBorderSize   = 1.0f;

    // --- palette ---
    // base dark
    const ImVec4 bg0   = ImVec4(0.07f, 0.08f, 0.10f, 1.00f); // window bg
    const ImVec4 bg1   = ImVec4(0.10f, 0.11f, 0.14f, 1.00f); // child / panels
    const ImVec4 bg2   = ImVec4(0.13f, 0.14f, 0.18f, 1.00f); // frames
    const ImVec4 bg3   = ImVec4(0.16f, 0.17f, 0.22f, 1.00f); // hover
    const ImVec4 bg4   = ImVec4(0.18f, 0.20f, 0.26f, 1.00f); // active

    // borders
    const ImVec4 border = ImVec4(0.20f, 0.22f, 0.28f, 1.00f);

    // text
    const ImVec4 text      = ImVec4(0.90f, 0.92f, 0.96f, 1.00f);
    const ImVec4 textDis   = ImVec4(0.55f, 0.58f, 0.64f, 1.00f);

    // accent (cool cyan)
    const ImVec4 accent    = ImVec4(0.20f, 0.78f, 0.78f, 1.00f);
    const ImVec4 accentH   = ImVec4(0.26f, 0.88f, 0.88f, 1.00f);
    const ImVec4 accentA   = ImVec4(0.16f, 0.62f, 0.62f, 1.00f);

    // --- global ---
    colors[ImGuiCol_Text]                  = text;
    colors[ImGuiCol_TextDisabled]          = textDis;

    colors[ImGuiCol_WindowBg]              = bg0;
    colors[ImGuiCol_ChildBg]               = bg1;
    colors[ImGuiCol_PopupBg]               = ImVec4(bg1.x, bg1.y, bg1.z, 0.98f);

    colors[ImGuiCol_Border]                = border;
    colors[ImGuiCol_BorderShadow]          = ImVec4(0,0,0,0);

    // --- frames / inputs ---
    colors[ImGuiCol_FrameBg]               = bg2;
    colors[ImGuiCol_FrameBgHovered]        = bg3;
    colors[ImGuiCol_FrameBgActive]         = bg4;

    // --- titles ---
    colors[ImGuiCol_TitleBg]               = bg1;
    colors[ImGuiCol_TitleBgActive]         = bg1;
    colors[ImGuiCol_TitleBgCollapsed]      = bg1;

    // --- scrollbars ---
    colors[ImGuiCol_ScrollbarBg]           = ImVec4(bg1.x, bg1.y, bg1.z, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.30f, 0.32f, 0.38f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.38f, 0.40f, 0.48f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.45f, 0.48f, 0.58f, 1.00f);

    // --- check / sliders ---
    colors[ImGuiCol_CheckMark]             = accent;
    colors[ImGuiCol_SliderGrab]            = accentA;
    colors[ImGuiCol_SliderGrabActive]      = accentH;

    // --- buttons ---
    colors[ImGuiCol_Button]                = bg2;
    colors[ImGuiCol_ButtonHovered]         = bg3;
    colors[ImGuiCol_ButtonActive]          = bg4;

    // --- headers (tree, selectable, collapsing) ---
    //colors[ImGuiCol_Header]                = ImVec4(bg2.x, bg2.y, bg2.z, 1.00f);
    //colors[ImGuiCol_HeaderHovered]         = ImVec4(bg3.x, bg3.y, bg3.z, 1.00f);
    //colors[ImGuiCol_HeaderActive]          = ImVec4(bg4.x, bg4.y, bg4.z, 1.00f);
    //colors[ImGuiCol_Header]        = ImVec4(accent.x,  accent.y,  accent.z,  0.22f);
    //colors[ImGuiCol_HeaderHovered] = ImVec4(accentH.x, accentH.y, accentH.z, 0.30f);
    //colors[ImGuiCol_HeaderActive]  = ImVec4(accentH.x, accentH.y, accentH.z, 0.38f);
    colors[ImGuiCol_Header]        = ImVec4(0.20f, 0.22f, 0.30f, 1.00f); // 比 bg2 明显
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.24f, 0.26f, 0.35f, 1.00f);
    colors[ImGuiCol_HeaderActive]  = ImVec4(0.28f, 0.30f, 0.42f, 1.00f);





    // --- separators / resize grips ---
    colors[ImGuiCol_Separator]             = border;
    colors[ImGuiCol_SeparatorHovered]      = ImVec4(accent.x, accent.y, accent.z, 0.60f);
    colors[ImGuiCol_SeparatorActive]       = ImVec4(accent.x, accent.y, accent.z, 0.90f);

    colors[ImGuiCol_ResizeGrip]            = ImVec4(0,0,0,0);
    colors[ImGuiCol_ResizeGripHovered]     = ImVec4(accent.x, accent.y, accent.z, 0.35f);
    colors[ImGuiCol_ResizeGripActive]      = ImVec4(accent.x, accent.y, accent.z, 0.60f);

    // --- tabs ---
    colors[ImGuiCol_Tab]                   = bg1;
    colors[ImGuiCol_TabHovered]            = ImVec4(bg3.x, bg3.y, bg3.z, 1.00f);
    colors[ImGuiCol_TabActive]             = bg2;
    colors[ImGuiCol_TabUnfocused]          = bg1;
    colors[ImGuiCol_TabUnfocusedActive]    = bg2;

    // --- docking (如果你用 Docking) ---
    colors[ImGuiCol_DockingPreview]        = ImVec4(accent.x, accent.y, accent.z, 0.25f);
    colors[ImGuiCol_DockingEmptyBg]        = bg0;

    // --- plots ---
    colors[ImGuiCol_PlotLines]             = ImVec4(0.80f, 0.82f, 0.88f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]      = accentH;
    colors[ImGuiCol_PlotHistogram]         = ImVec4(0.75f, 0.70f, 0.30f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(0.90f, 0.82f, 0.35f, 1.00f);

    // --- selection / dragdrop ---
    colors[ImGuiCol_TextSelectedBg]        = ImVec4(accent.x, accent.y, accent.z, 0.25f);
    colors[ImGuiCol_DragDropTarget]        = ImVec4(accentH.x, accentH.y, accentH.z, 0.90f);

    // --- navigation highlights ---
    colors[ImGuiCol_NavHighlight]          = ImVec4(accentH.x, accentH.y, accentH.z, 0.70f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1,1,1,0.60f);
    colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0,0,0,0.35f);
    colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0,0,0,0.45f);
}
