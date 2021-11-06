#include "DirShadowResolutionEvent.hpp"


namespace leopph::impl
{
	DirShadowResolutionEvent::DirShadowResolutionEvent(const std::vector<std::size_t>& resolutions) :
		Resolutions{resolutions}
	{
	}
}