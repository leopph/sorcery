#pragma once

#include "LeopphApi.hpp"


namespace leopph
{
	// This function is called right before the first frame update.
	// Implement it to set the initial state of your application.
	void Init();


	namespace internal
	{
		// LeopphEngine internal main function.
		// Must not be called explicitly.
		LEOPPHAPI int Main(decltype(Init) initFunc);
	}
}
