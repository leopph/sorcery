#pragma once

#include "leopphapi.h"

namespace leopph
{
	class Object;


	// Users subclass this and add them to Objects to create game logic
	class LEOPPHAPI Behavior
	{
	protected:
		Object& m_Object;

	public:
		Behavior(Object& object);
		virtual ~Behavior() = default;

		// THIS IS CALLED EVERY FRAME FOR ALL BEHAVIORS
		virtual void operator()() = 0;

		Object& OwningObject();
	};
}