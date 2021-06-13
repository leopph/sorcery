#pragma once

#include "../../api/leopphapi.h"

namespace leopph::impl
{
	bool LEOPPHAPI InitGL();

	void LEOPPHAPI TerminateGL();

	void LEOPPHAPI GetErrorGL();
}