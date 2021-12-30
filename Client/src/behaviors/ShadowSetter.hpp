#pragma once

#include "Leopph.hpp"

#include <span>
#include <vector>


class ShadowSetter final : public leopph::Behavior
{
	public:
		ShadowSetter(leopph::Entity* entity, std::span<leopph::Model* const> models);

		auto OnFrameUpdate() -> void override;

	private:
		std::vector<leopph::Model*> m_Models;
		bool m_Shadow{true};
};
