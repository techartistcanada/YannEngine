#include "pch.h"
#include "ParticleSystemUI.h"

void ParticleSystemUI::render_tick()
{
}

ParticleSystemUI::ParticleSystemUI()
	: ComponentUI("ParticleSystem", "##ParticleSystemUI", COMPONENT_TYPE::PARTICLESYSTEM)
{
	SetSizeAsChild(ImVec2(0.0f, 200.0f));
}

ParticleSystemUI::~ParticleSystemUI()
{
	render_title();
}
