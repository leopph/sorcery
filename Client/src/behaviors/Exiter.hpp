#pragma once

#include "Leopph.hpp"


class Exiter final : public leopph::Behavior
{
	public:
		Exiter(leopph::Entity* entity);

		auto OnFrameUpdate() -> void override;
};
