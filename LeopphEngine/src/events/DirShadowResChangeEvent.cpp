#include "DirShadowResChangeEvent.hpp"


namespace leopph::internal
{
	DirShadowResChangeEvent::DirShadowResChangeEvent(const std::span<const std::size_t> ress) :
		Resolutions{ress.begin(), ress.end()}
	{ }
}
