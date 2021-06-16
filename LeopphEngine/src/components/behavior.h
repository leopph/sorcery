#pragma once


#include "../api/leopphapi.h"
#include "component.h"


namespace leopph
{
	/*------------------------------------------------------------------------------------------------------
	Behaviors are special components that provide can hook into and effect the status of objects at runtime.
	Subclass this to provide the logic for your application.
	See "object.h" and "component.h" for more information.
	------------------------------------------------------------------------------------------------------*/

	class LEOPPHAPI Behavior : public Component
	{
	public:
		/* Contructor and destructor used internally */
		Behavior();
		~Behavior() override = 0;

		/* This function gets called every frame. Override this to provide business logic */
		virtual void OnFrameUpdate() {}
	};
}