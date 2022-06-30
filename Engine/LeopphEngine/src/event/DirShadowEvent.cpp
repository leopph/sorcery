#include "events/DirShadowEvent.hpp"

namespace leopph::internal
{
	DirShadowEvent::DirShadowEvent(const std::span<const std::size_t> res) :
		Resolutions{res.begin(), res.end()}
	{}
}