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
			Behavior(const Behavior& other) = default;
			auto operator=(const Behavior& other) -> Behavior& = default;

			Behavior(Behavior&& other) = default;
			auto operator=(Behavior&& other) -> Behavior& = default;

			LEOPPHAPI ~Behavior() override;

			// This function gets called every frame.
			virtual auto OnFrameUpdate() -> void
			{}

		protected:
			LEOPPHAPI explicit Behavior(leopph::Entity* entity);
	};
}
