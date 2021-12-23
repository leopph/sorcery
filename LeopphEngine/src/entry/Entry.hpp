#pragma once

#include "Main.hpp"

/*-------------------------------------
Entry point for the engine.
INCLUDE THIS EXACTLY ONCE IN YOUR CODE!
-------------------------------------*/

auto main() -> int
{
	return leopph::internal::Main(leopph::Init);
}
