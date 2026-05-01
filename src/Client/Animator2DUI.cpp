#include "pch.h"
#include "Animator2DUI.h"


Animator2DUI::Animator2DUI()
	: ComponentUI("Animator2D", "##Animator2DUI", COMPONENT_TYPE::ANIMATOR2D)
{
	SetSizeAsChild(ImVec2(0.0f, 200.0f));
}

Animator2DUI::~Animator2DUI()
{
}

void Animator2DUI::render_tick()
{
	render_title();
}
