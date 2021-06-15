#pragma once

#include "../api/leopphapi.h"
#include "appstart.h"

/*-----------------------------------
Internal engine initializer function.
DO NOT CALL THIS EXPLICITLY!
-----------------------------------*/

namespace leopph::impl
{
	LEOPPHAPI int Launch(decltype(AppStart) appStart);
}