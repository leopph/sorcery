#pragma once

#include "Component.hpp"
#include "../api/LeopphApi.hpp"


namespace leopph
{
	// Behaviors are special components that provide actions to and modify the status of Entities at runtime.
	// Subclass this to provide logic for your application.
	class Behavior : public Component
	{
		public:
			// This function is called on all Behaviors every frame.
			virtual auto OnFrameUpdate() -> void
			{}

			LEOPPHAPI ~Behavior() override;

		protected:
			LEOPPHAPI explicit Behavior(leopph::Entity* entity);

			Behavior(const Behavior& other) = default;
			auto operator=(const Behavior& other) -> Behavior& = default;

			Behavior(Behavior&& other) = default;
			auto operator=(Behavior&& other) -> Behavior& = default;
	};
}
