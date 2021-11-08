#pragma once

#include "../api/LeopphApi.hpp"
#include "appstart.h"

/*-----------------------------------
Internal engine initializer function.
DO NOT CALL THIS EXPLICITLY!
-----------------------------------*/

namespace leopph::impl
{
	LEOPPHAPI int Launch(decltype(AppStart) appStart);
}