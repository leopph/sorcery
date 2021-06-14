#pragma once


#include "../api/leopphapi.h"
#include "component.h"
#include <set>


namespace leopph
{
	/*------------------------------------------------------------------------------------------------------
	Behaviors are special components that provide can hook into and effect the status of objects at runtime.
	Subclass this to provide the logic for your application.
	See "object.h" and "component.h" for more information.
	------------------------------------------------------------------------------------------------------*/

	class Behavior : public Component
	{
	public:
		/* Internal tool to update all current Behaviors */
		// TODO this should not be accessible outside
		LEOPPHAPI static void UpdateAll();

		/* Contructor and destructor used internally */
		LEOPPHAPI Behavior();
		LEOPPHAPI ~Behavior() override = 0;

		/* This function gets called every frame. Override this to provide business logic */
		LEOPPHAPI virtual void OnFrameUpdate() {}

	private:
		static std::set<Behavior*> s_Behaviors;
	};
}