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
			// This function is called on all attached active Behaviors every frame.
			virtual auto OnFrameUpdate() -> void;

			// Activate the Behavior.
			// OnFrameUpdate is only called on attached active Behaviors.
			LEOPPHAPI auto Activate() -> void override;

			// Deactive the Behavior.
			// OnFrameUpdate is only called on attached active Behaviors.
			LEOPPHAPI auto Deactivate() -> void override;

			LEOPPHAPI auto Attach(leopph::Entity* entity) -> void final;

			LEOPPHAPI auto Detach() -> void final;

			Behavior(const Behavior& other) = delete;
			auto operator=(const Behavior& other) -> Behavior& = delete;

			Behavior(Behavior&& other) noexcept = delete;
			auto operator=(Behavior&& other) noexcept -> Behavior& = delete;

			LEOPPHAPI ~Behavior() override;

		protected:
			Behavior() = default;
	};


	inline auto Behavior::OnFrameUpdate() -> void
	{}
}
