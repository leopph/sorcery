#pragma once

#include "../api/leopphapi.h"


namespace leopph
{
	class Entity;


	/* --------------------------------------------------------------------------------------
	Components are objects that are attached to Entities to provide additional functionality.
	Subclass this to provide your implementation.
	See "Entity.hpp" for more information.
	---------------------------------------------------------------------------------------*/

	class Component
	{
	public:
		LEOPPHAPI explicit Component(Entity& owner);
		LEOPPHAPI virtual ~Component() = 0;

		Entity& entity;

		Component(const Component&) = delete;
		Component(Component&&) = delete;
		void operator=(const Component&) = delete;
		void operator=(Component&&) = delete;
	};
}
