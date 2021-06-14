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
		friend class Object;

	public:
		/* Constructors and destructors used internally */
		LEOPPHAPI Component();
		LEOPPHAPI virtual ~Component() = 0;


		/* This function is called immediately after construction.
		If you need to refer to your component's owning object during initialization,
		use this. DO NOT REFER TO IT IN CONSTRUCTORS! */
		LEOPPHAPI virtual void Init() {}


		/* These provide access to the owning Object */
		LEOPPHAPI leopph::Object& Object();
		LEOPPHAPI const leopph::Object& Object() const;

	private:
		leopph::Object* m_Object;
	};
}
