#pragma once

#include "Leopph.hpp"


class Exiter final : public leopph::Behavior
{
	public:
		auto OnFrameUpdate() -> void override;
};
