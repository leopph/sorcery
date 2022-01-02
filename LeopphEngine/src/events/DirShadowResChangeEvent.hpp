#pragma once

#include "Event.hpp"

#include <span>
#include <vector>


namespace leopph::internal
{
	struct DirShadowResChangeEvent final : public Event
	{
		explicit DirShadowResChangeEvent(std::span<const std::size_t> ress);

		std::vector<std::size_t> Resolutions;
	};
}
