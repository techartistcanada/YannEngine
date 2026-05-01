#pragma once


#include "CImGuiMgr.h"
#include "EditorUI.h"

class ParameterUI
{
private:
	static UINT g_ParameterUI_ID;

	static EditorUI*	 g_CallerUI;
	static UI_DELEGATE_1 g_Delegate_1;
public:
	static void ResetID() { g_ParameterUI_ID = 0; }
	static void RegisterSelTexDelegate(EditorUI* _pCallerUI, UI_DELEGATE_1 _Delegate) { g_CallerUI = _pCallerUI; g_Delegate_1 = _Delegate; }
public:
	static int Param_DragInt(const string& _strName, int* _pInOut, int _Speed = 1);
	static int Param_DragFloat(const string& _strName, float* _pInOut, float _Speed = 0.1f);
	static int Param_DragVec2(const string& _strName, Vec2* _pInOut, float _Speed = 0.1f);
	static int Param_DragVec4(const string& _strName, Vec4* _pInOut, float _Speed = 0.1f);
	static int Param_DragMatrix(const string& _strName, Matrix* _pInOut, float _Speed = 0.1f);
	static int Param_Checkbox(const string& _strName, int* _pInOut);
	static int Param_Texture(const string& _strName, Ptr<CTexture>& _pTex);
};

