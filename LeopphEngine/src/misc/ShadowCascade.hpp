#pragma once

#include <cstddef>


namespace leopph
{
	// Describes a partition of space relative to the Camera where shadows are calculated.
	struct ShadowCascade
	{
		// The resolution of the square shadow map when using this cascade.
		std::size_t Resolution;
		// The distance from the Camera at which rendering switches to the next cascade.
		std::size_t Bound;
	};
}