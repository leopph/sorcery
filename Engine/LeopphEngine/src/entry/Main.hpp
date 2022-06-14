#pragma once

#include "Init.hpp"
#include "LeopphApi.hpp"


namespace leopph::internal
{
	// LeopphEngine internal main function.
	// Must not be called explicitly.
	LEOPPHAPI auto Main(decltype(Init) initFunc) -> int;
}
