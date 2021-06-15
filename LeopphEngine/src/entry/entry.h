#pragma once

#include "launch.h"

/*-------------------------------------
Entry point for the engine.
INCLUDE THIS EXACTLY ONCE IN YOUR CODE!
-------------------------------------*/

int main()
{
	return leopph::impl::Launch(leopph::AppStart);
}