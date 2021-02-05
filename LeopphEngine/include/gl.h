#pragma once

#include "leopphapi.h"

namespace leopph::implementation
{
	bool LEOPPHAPI InitGL();

	void LEOPPHAPI TerminateGL();

	void LEOPPHAPI GetErrorGL();
}