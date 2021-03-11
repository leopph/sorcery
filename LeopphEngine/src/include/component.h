#pragma once

#include "leopphapi.h"


namespace leopph
{
	class Object;


	// Attach this to Objects
	class LEOPPHAPI Component
	{
	private:
		Object& m_Object;

	public:
		Component(Object& object);
		virtual ~Component() = 0;


		// Access object the component is attached to
		leopph::Object& Object();
		const leopph::Object& Object() const;
	};
}
