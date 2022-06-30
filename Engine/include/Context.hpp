#pragma once

#include "LeopphApi.hpp"

namespace leopph
{
	class Window;
	class Settings;

	auto LEOPPHAPI GetWindow() -> Window*;
	auto LEOPPHAPI GetSettings() -> Settings*;
}