#include "events/DirShadowResEvent.hpp"

namespace leopph::internal
{
	DirShadowResEvent::DirShadowResEvent(const std::span<const std::size_t> res) :
		Resolutions{res.begin(), res.end()}
	{}
}