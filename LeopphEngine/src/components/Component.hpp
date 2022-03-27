#pragma once

#include "../api/LeopphApi.hpp"

#include <concepts>
#include <memory>


namespace leopph
{
	class Entity;
	class Component;

	// Reference counted pointer to a Component with ownership.
	template<std::derived_from<Component> T = Component>
	using ComponentPtr = std::shared_ptr<T>;

	template<std::derived_from<Component> T, typename... Args>
	constexpr auto CreateComponent(Args&&... args) -> ComponentPtr<T>;

	// Special objects used that add custom properties to Entities.
	// Components are attached to Entities to achieve this decoration.
	// The Entity they are attached to is said to be their owner.
	// Components can also be active or inactive.
	// Only attached and active Components take part in internal calculations.
	class Component : public std::enable_shared_from_this<Component>
	{
		public:
			// Get the owner of the Component or nullptr if none.
			[[nodiscard]] LEOPPHAPI
			auto Owner() const noexcept -> Entity*;

			// Set the owner of the Component.
			// Passing nullptr will leave the Component unattached.
			// If the Component already has an owner, it will detach itself first.
			LEOPPHAPI virtual
			auto Owner(Entity* entity) -> void;

			// Same as calling Owner with a non-null pointer.
			// Hence, nullptr must not be passed.
			LEOPPHAPI
			auto Attach(Entity* entity) -> void;

			// Same as calling Owner with nullptr.
			LEOPPHAPI
			auto Detach() -> void;

			// Returns whether the Component has an owner.
			// Same as Owner() != nullptr.
			[[nodiscard]] LEOPPHAPI
			auto Attached() const -> bool;

			// Get whether the Component is active.
			[[nodiscard]] LEOPPHAPI
			auto Active() const noexcept -> bool;

			// Set whether the Component is active.
			LEOPPHAPI virtual
			auto Active(bool active) -> void;

			// Same as calling Active(true).
			LEOPPHAPI
			auto Activate() -> void;

			// Same as calling Active(false);
			LEOPPHAPI
			auto Deactivate() -> void;

			// Returns whether the Component takes part in internal calculations.
			// Same as Owner() && Active().
			[[nodiscard]] LEOPPHAPI
			auto InUse() const noexcept -> bool;

			// Gets called after the Component changed owner.
			LEOPPHAPI inline virtual
			auto OnOwnerChange(Entity* oldOwner) -> void;

			// Gets called after the Component was attached.
			LEOPPHAPI inline virtual
			auto OnAttach() -> void;

			// Gets called after the Component was detached.
			LEOPPHAPI inline virtual
			auto OnDetach() -> void;

			// Gets called when the Component was activated.
			LEOPPHAPI inline virtual
			auto OnActivate() -> void;

			// Gets called when the Component was deactivated.
			LEOPPHAPI inline virtual
			auto OnDeactivate() -> void;

			Component(Component&&) = delete;
			auto operator=(Component&&) -> Component& = delete;

			LEOPPHAPI virtual ~Component() = 0;

		protected:
			Component() = default;

			// The new Component will have the same active state but will be unattached.
			LEOPPHAPI
			Component(Component const& other);

			// Attaches itself to the the Component's Entity and takes its active state.
			LEOPPHAPI
			auto operator=(Component const& other) -> Component&;

		private:
			Entity* m_Owner{nullptr};
			bool m_Active{true};
	};


	template<std::derived_from<Component> T, typename... Args>
	constexpr auto CreateComponent(Args&&... args) -> ComponentPtr<T>
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}


	inline auto Component::OnOwnerChange(Entity*) -> void
	{ }


	inline auto Component::OnAttach() -> void
	{ }


	inline auto Component::OnDetach() -> void
	{ }


	inline auto Component::OnActivate() -> void
	{ }


	inline auto Component::OnDeactivate() -> void
	{ }
}
