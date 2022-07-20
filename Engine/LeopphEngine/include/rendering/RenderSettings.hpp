#pragma once

#include "Types.hpp"

#include <array>

namespace leopph::rendersettings
{
	u8 constexpr numMaxShadowCascades{3};
	std::array<u16, numMaxShadowCascades> constexpr dirShadowResolutions{4096, 2048, 1024};
	f32 constexpr dirShadowCorrection{.75f};

	u16 constexpr numMaxSpot{8};
	u16 constexpr numMaxSpotShadow{8};
	u16 constexpr spotShadowResolution{2048};

	u16 constexpr numMaxPoint{8};
	u16 constexpr numMaxPointShadow{8};
	u16 constexpr pointShadowResolution{1024};
}
