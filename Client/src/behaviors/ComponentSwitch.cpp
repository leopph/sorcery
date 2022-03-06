#include "ComponentSwitch.hpp"

#include <utility>


ComponentSwitch::ComponentSwitch(leopph::Entity* entity, std::vector<Component*> components) :
	Behavior{entity},
	m_Components{std::move(components)}
{}


auto ComponentSwitch::OnFrameUpdate() -> void
{
	if (leopph::Input::GetKeyDown(leopph::KeyCode::K))
	{
		for (const auto component : m_Components)
		{
			component->Activate();
		}
	}

	if (leopph::Input::GetKeyDown(leopph::KeyCode::L))
	{
		for (const auto component : m_Components)
		{
			component->Deactivate();
		}
	}
}
