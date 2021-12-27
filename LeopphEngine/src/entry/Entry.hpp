#pragma once

#include "Main.hpp"

// Entry point for LeopphEngine.
// This must be included exactly once in your application.
auto main() -> int
{
	return leopph::internal::Main(leopph::Init);
}
