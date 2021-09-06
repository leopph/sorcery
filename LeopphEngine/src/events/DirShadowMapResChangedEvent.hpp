#pragma once

#include "Event.hpp"

#include <cstddef>
#include <vector>


namespace leopph::impl
{
	struct DirShadowMapResChangedEvent : public Event
	{
		DirShadowMapResChangedEvent(const std::vector<std::size_t>& resolutions);

		const std::vector<std::size_t>& Resolutions;
	};
}