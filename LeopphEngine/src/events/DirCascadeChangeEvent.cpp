#include "DirCascadeChangeEvent.hpp"


namespace leopph::internal
{
	DirCascadeChangeEvent::DirCascadeChangeEvent(const std::span<const ShadowCascade> cascades) :
		Cascades{cascades.begin(), cascades.end()}
	{ }
}
