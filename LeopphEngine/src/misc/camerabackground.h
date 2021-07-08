#pragma once

#include "../misc/color.h"
#include "skybox.h"

#include <memory>

namespace leopph
{
	/*---------------------------------------------------------------------------------------
	The CameraBackground struct is used to paint the back of a Camera object.
	It contains a color and potentially a Skybox. If only a Color is given,
	the background will be of solid color. If a Skybox is present, LeopphEngine ignores the
	color property and uses the Skybox to draw the background.
	---------------------------------------------------------------------------------------*/
	struct CameraBackground
	{
		Color color;
		std::unique_ptr<Skybox> skybox;
	};
}