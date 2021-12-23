#pragma once

#include "../../components/lighting/Light.hpp"


namespace leopph::internal
{
	struct LightCloserToCamera
	{
		auto operator()(const Light* left, const Light* right) const -> bool;
	};
}
