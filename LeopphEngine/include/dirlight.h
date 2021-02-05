#pragma once

#include "leopphapi.h"
#include "light.h"

namespace leopph
{
	class LEOPPHAPI DirectionalLight : public implementation::Light
	{
	public:
		using implementation::Light::Light;
	};
}