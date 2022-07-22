#pragma once

#include "LeopphApi.hpp"


namespace leopph
{
	class Window;
	class Settings;

	LEOPPHAPI Window* GetWindow();
	LEOPPHAPI Settings* GetSettings();
}
