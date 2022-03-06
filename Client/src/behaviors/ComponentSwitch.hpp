#pragma once

#include "Leopph.hpp"

#include <vector>


class ComponentSwitch final : public leopph::Behavior
{
	public:
		ComponentSwitch(leopph::Entity* entity, std::vector<Component*> components);

		auto OnFrameUpdate() -> void override;

	private:
		std::vector<Component*> m_Components;
};