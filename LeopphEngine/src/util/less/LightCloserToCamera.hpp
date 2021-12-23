#pragma once

#include "../../components/lighting/Light.hpp"


namespace leopph::internal
{
	struct LightCloserToCamera
	{
		bool operator()(const Light* left, const Light* right) const;
	};
}