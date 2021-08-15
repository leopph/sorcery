#pragma once


#include "../api/leopphapi.h"
#include "Component.hpp"


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
		LEOPPHAPI explicit Behavior(Object& owner);
		LEOPPHAPI ~Behavior() override = 0;

		Behavior(const Behavior&) = delete;
		Behavior(Behavior&&) = delete;
		void operator=(const Behavior&) = delete;
		void operator=(Behavior&&) = delete;

		/* This function gets called every frame. Override this to provide business logic */
		virtual void OnFrameUpdate() {}
	};
}