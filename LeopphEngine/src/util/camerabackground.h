#pragma once

#include "../rendering/color.h"
#include "skybox.h"

#include <memory>

namespace leopph
{
	struct CameraBackground
	{
		Color color;
		std::unique_ptr<Skybox> skybox;
	};
}