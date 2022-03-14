#pragma once

#include "Event.hpp"

#include <span>
#include <vector>


namespace leopph::internal
{
	// An Event representing a change in DirectionalLight shadow map resolution.
	struct DirShadowEvent final : Event
	{
		explicit DirShadowEvent(std::span<const std::size_t> res);
		std::vector<std::size_t> Resolutions;
	};
}
