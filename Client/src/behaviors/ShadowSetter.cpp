#include "ShadowSetter.hpp"

#include <algorithm>

using leopph::Input;
using leopph::KeyCode;


ShadowSetter::ShadowSetter(leopph::Entity* const entity, const std::span<leopph::Model* const> models) :
	Behavior{entity},
	m_Models{models.begin(), models.end()}
{}


auto ShadowSetter::OnFrameUpdate() -> void
{
	if (Input::GetKeyDown(KeyCode::T))
	{
		m_Shadow = !m_Shadow;
		std::ranges::for_each(m_Models, [=](const auto model)
		{
			model->CastsShadow(m_Shadow);
		});
	}
}
