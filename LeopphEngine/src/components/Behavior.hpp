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
			virtual
			auto OnFrameUpdate() -> void = 0;

			// Get the Update Index of the Behavior.
			// The update index defines the order of OnFrameUpdate calls on Behaviors.
			// Behaviors with the a lower Update Index are updated before the ones with a higher index.
			// Behaviors with the same index have an undefined order between them.
			[[nodiscard]] LEOPPHAPI
			auto UpdateIndex() const noexcept -> int;

			// Set the Update Index of the Behavior.
			// The update index defines the order of OnFrameUpdate calls on Behaviors.
			// Behaviors with the a lower Update Index are updated before the ones with a higher index.
			// Behaviors with the same index have an undefined order between them.
			LEOPPHAPI
			auto UpdateIndex(int index) -> void;

			// Activate the Behavior.
			// OnFrameUpdate is only called on attached, active Behaviors.
			LEOPPHAPI
			auto Activate() -> void override;

			// Deactive the Behavior.
			// OnFrameUpdate is only called on attached, active Behaviors.
			LEOPPHAPI
			auto Deactivate() -> void override;

			// Attach the Behavior to the Entity.
			// OnFrameUpdate is only called on attached, active Behaviors.
			LEOPPHAPI
			auto Attach(leopph::Entity* entity) -> void override;

			LEOPPHAPI
			// Detach the Behavior from its Entity.
			// OnFrameUpdate is only called on attached, active Behaviors.
			auto Detach() -> void override;

			Behavior(Behavior const& other) = delete;
			auto operator=(Behavior const& other) -> Behavior& = delete;

			Behavior(Behavior&& other) noexcept = delete;
			auto operator=(Behavior&& other) noexcept -> Behavior& = delete;

			LEOPPHAPI ~Behavior() override;

		protected:
			Behavior() = default;

		private:
			int m_UpdateIndex{0};
	};
}
