#pragma once

#include "leopphapi.h"

namespace leopph
{
	// Users subclass this and add them to Objects to create game logic
	class LEOPPHAPI Behavior
	{
	public:
		// THIS IS CALLED EVERY FRAME FOR ALL BEHAVIORS
		virtual void operator()() = 0;

		virtual ~Behavior() = default;
	};
}