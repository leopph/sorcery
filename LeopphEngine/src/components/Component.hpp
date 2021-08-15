#pragma once

#include "../api/leopphapi.h"


namespace leopph
{
	class Object;


	/* --------------------------------------------------------------------------------------
	Components are entities that are attached to Objects to provide additional functionality.
	Subclass this to provide your implementation.
	See "object.h" for more information.
	---------------------------------------------------------------------------------------*/

	class Component
	{
	public:
		LEOPPHAPI explicit Component(Object& owner);
		LEOPPHAPI virtual ~Component() = 0;

		Object& object;

		Component(const Component&) = delete;
		Component(Component&&) = delete;
		void operator=(const Component&) = delete;
		void operator=(Component&&) = delete;
	};
}
