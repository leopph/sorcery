#pragma once

#include "LeopphApi.hpp"


namespace leopph::time
{
	// The time it took in seconds to render the previous frame.
	LEOPPHAPI auto DeltaTime() -> float;

	// The time that has passed since the initialization of your application in seconds.
	LEOPPHAPI auto FullTime() -> float;
}
