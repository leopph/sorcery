#pragma once

#include "../../components/lighting/Light.hpp"


namespace leopph::impl
{
	struct LightCloserToCamera
	{
		bool operator()(const Light* left, const Light* right) const;
	};
}