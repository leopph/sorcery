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

	class LEOPPHAPI Component
	{
		friend class Object;

	public:
		/* Constructors and destructors used internally */
		Component() = default;
		virtual ~Component() = 0;


		/* This function is called immediately after construction.
		If you need to refer to your component's owning object during initialization,
		use this. DO NOT REFER TO IT IN CONSTRUCTORS! */
		virtual void Init() {}


		/* These provide access to the owning Object */
		leopph::Object& Object();
		const leopph::Object& Object() const;

	private:
		void SetOwnership(leopph::Object* object);
		
		leopph::Object* m_Object{};
	};
}
