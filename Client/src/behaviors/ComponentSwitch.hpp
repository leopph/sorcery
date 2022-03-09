#pragma once

#include "Leopph.hpp"

#include <vector>


class ComponentSwitch final : public leopph::Behavior
{
	public:
		explicit ComponentSwitch(std::vector<Component*> components);

		auto OnFrameUpdate() -> void override;

	private:
		std::vector<Component*> m_Components;
};