#pragma once

#include "Component.hpp"
#include "../api/LeopphApi.hpp"


namespace leopph
{
	// Special Components that are updated every frame.
	// Their Update Index defines their priority over other Behaviors.
	// Lower indices are called first.
	// Only active and attached Behaviors are updated.
	class Behavior : public Component
	{
		public:
			// This function is called on all attached active Behaviors every frame.
			virtual
			auto OnFrameUpdate() -> void = 0;

			// Get the Update Index of the Behavior.
			[[nodiscard]] LEOPPHAPI
			auto UpdateIndex() const noexcept -> int;

			// Set the Update Index of the Behavior.
			LEOPPHAPI
			auto UpdateIndex(int index) -> void;

			LEOPPHAPI
			auto Owner(Entity* entity) -> void final;
			using Component::Owner;

			LEOPPHAPI
			auto Active(bool active) -> void final;
			using Component::Active;

			Behavior(Behavior&& other) = delete;
			auto operator=(Behavior&& other) -> Behavior& = delete;

			LEOPPHAPI ~Behavior() override;

		protected:
			Behavior() = default;

			// Takes the other's update index.
			Behavior(Behavior const& other) = default;

			// Takes the other's update index.
			LEOPPHAPI
			auto operator=(Behavior const& other) -> Behavior&;

		private:
			int m_UpdateIndex{0};
	};
}
