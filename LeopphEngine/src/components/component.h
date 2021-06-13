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
		/* Constructors and destructors used internally */
		LEOPPHAPI Component(Object& object);
		LEOPPHAPI virtual ~Component() = 0;


		/* These provide access to the owning Object */
		LEOPPHAPI leopph::Object& Object();
		LEOPPHAPI const leopph::Object& Object() const;

	private:
		leopph::Object& m_Object;
	};
}
