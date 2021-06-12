#pragma once

#include "leopphapi.h"

namespace leopph::impl
{
	bool LEOPPHAPI InitGL();

	void LEOPPHAPI TerminateGL();

	void LEOPPHAPI GetErrorGL();
}