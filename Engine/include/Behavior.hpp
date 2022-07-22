#pragma once

#include "Component.hpp"


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
			virtual void OnFrameUpdate() = 0;

			// Get the Update Index of the Behavior.
			[[nodiscard]] LEOPPHAPI int UpdateIndex() const noexcept;

			// Set the Update Index of the Behavior.
			LEOPPHAPI void UpdateIndex(int index);

			LEOPPHAPI void Owner(Entity* entity) final;
			using Component::Owner;

			LEOPPHAPI void Active(bool active) final;
			using Component::Active;

			Behavior(Behavior&& other) = delete;
			Behavior& operator=(Behavior&& other) = delete;

			LEOPPHAPI ~Behavior() override;

		protected:
			Behavior() = default;

			// Takes the other's update index.
			Behavior(Behavior const& other) = default;

			// Takes the other's update index.
			LEOPPHAPI Behavior& operator=(Behavior const& other);

		private:
			int m_UpdateIndex{0};
	};
}
