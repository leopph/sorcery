#include "DirShadowMapResChangedEvent.hpp"


namespace leopph::impl
{
	DirShadowMapResChangedEvent::DirShadowMapResChangedEvent(const std::vector<std::size_t>& resolutions) :
		Resolutions{resolutions}
	{
	}
}