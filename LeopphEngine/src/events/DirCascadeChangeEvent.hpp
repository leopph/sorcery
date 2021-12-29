#pragma once

#include "Event.hpp"
#include "../misc/ShadowCascade.hpp"

#include <span>
#include <vector>


namespace leopph::internal
{
	struct DirCascadeChangeEvent final : public Event
	{
		explicit DirCascadeChangeEvent(std::span<const ShadowCascade> cascades);

		std::vector<ShadowCascade> Cascades;
	};
}
