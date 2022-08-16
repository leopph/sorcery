#pragma once

#include "Main.hpp"

// Entry point for LeopphEngine.
// This must be included exactly once in your application.
int main()
{
	return leopph::internal::main(leopph::init);
}
