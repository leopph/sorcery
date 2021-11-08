#pragma once

#include "Component.hpp"
#include "../api/LeopphApi.hpp"


namespace leopph
{
	/* Behaviors are special components that represent actions and can affect the status of Entities at runtime.
	 * Subclass this to provide the logic for your application. */
	class Behavior : public Component
	{
	public:
		// This function gets called every frame.
		virtual void OnFrameUpdate() {}


		LEOPPHAPI explicit Behavior(leopph::Entity& owner);
		Behavior(const Behavior&) = delete;
		Behavior(Behavior&&) = delete;

		LEOPPHAPI ~Behavior() override = 0;

		void operator=(const Behavior&) = delete;
		void operator=(Behavior&&) = delete;
	};
}