#pragma once

#include "Init.hpp"
#include "../api/LeopphApi.hpp"

/*-----------------------------------
Internal engine initializer function.
DO NOT CALL THIS EXPLICITLY!
-----------------------------------*/

namespace leopph::internal
{
	LEOPPHAPI auto Main(decltype(Init) initFunc) -> int;
}
